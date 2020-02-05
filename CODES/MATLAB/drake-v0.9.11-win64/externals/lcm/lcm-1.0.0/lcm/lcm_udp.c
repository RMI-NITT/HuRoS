#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#ifndef WIN32
#include <sys/uio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/select.h>

typedef int SOCKET;
#endif

#ifdef SO_TIMESTAMP
#define MSG_EXT_HDR
#endif

#ifdef WIN32
#include "windows/WinPorting.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#define MSG_EXT_HDR
#endif

#include <glib.h>

#include "lcm.h"
#include "lcm_internal.h"
#include "dbg.h"
#include "ringbuffer.h"


#define LCM_RINGBUF_SIZE (200*1024)

#define LCM_DEFAULT_RECV_BUFS 2000

#define LCM2_MAGIC_SHORT 0x4c433032   // hex repr of ascii "LC02" 
#define LCM2_MAGIC_LONG  0x4c433033   // hex repr of ascii "LC03" 

#define LCM_SHORT_MESSAGE_MAX_SIZE 1400

#define SELF_TEST_CHANNEL "LCM_SELF_TEST"

// HUGE is not defined on cygwin as of 2008-03-05
#ifndef HUGE
#define HUGE 3.40282347e+38F
#endif

#ifdef __APPLE__
#define USE_REUSEPORT
#else
#ifdef __FreeBSD__
#define USE_REUSEPORT
#endif
#endif


typedef struct _lcm2_header_short lcm2_header_short_t;
struct _lcm2_header_short {
    uint32_t magic;
    uint32_t msg_seqno;
};

typedef struct _lcm2_header_long lcm2_header_long_t;
struct _lcm2_header_long {
    uint32_t magic;
    uint32_t msg_seqno;
    uint32_t msg_size;
    uint32_t fragment_offset;
    uint16_t fragment_no;
    uint16_t fragments_in_msg;
};
// if fragment_no == 0, then header is immediately followed by NULL-terminated
// ASCII-encoded channel name, followed by the payload data
// if fragment_no > 0, then header is immediately followed by the payload data

typedef struct _lcm_frag_buf {
    char      channel[LCM_MAX_CHANNEL_NAME_LENGTH+1];
    struct    sockaddr_in from;
    char      *data;
    uint32_t  data_size;
    uint16_t  fragments_remaining;
    uint32_t  msg_seqno;
    GTimeVal last_packet_time;
    int64_t   first_packet_utime;
} lcm_frag_buf_t;

typedef struct _lcm_buf {
    char  channel_name[LCM_MAX_CHANNEL_NAME_LENGTH+1];
    int   channel_size;      // length of channel name

    int64_t recv_utime;      // timestamp of first datagram receipt
    char *buf;               // pointer to beginning of message.  This includes 
                             // the header for unfragmented messages, and does
                             // not include the header for fragmented messages.

    int   data_offset;       // offset to payload
    int   data_size;         // size of payload
    lcm_ringbuf_t *ringbuf;  // the ringbuffer used to allocate buf.  NULL if
                             // not allocated from ringbuf

    int   packet_size;       // total bytes received
    int   buf_size;          // bytes allocated 

    struct sockaddr from;    // sender
    socklen_t fromlen;
    struct _lcm_buf *next;
} lcm_buf_t;

typedef struct _lcm_buf_queue {
    lcm_buf_t * head;
    lcm_buf_t ** tail;
    int count;
} lcm_buf_queue_t;

/**
 * udpm_params_t:
 * @mc_addr:        multicast address
 * @mc_port:        multicast port
 * @mc_ttl:         if 0, then packets never leave local host.
 *                  if 1, then packets stay on the local network 
 *                        and never traverse a router
 *                  don't use > 1.  that's just rude. 
 * @recv_buf_size:  requested size of the kernel receive buffer, set with
 *                  SO_RCVBUF.  0 indicates to use the default settings.
 *
 */
typedef struct _udpm_params_t udpm_params_t;
struct _udpm_params_t {
    struct in_addr mc_addr;
    uint16_t mc_port;
    uint8_t mc_ttl; 
    int recv_buf_size;
};

typedef struct _lcm_provider_t lcm_udpm_t;
struct _lcm_provider_t {
    SOCKET recvfd;
    SOCKET sendfd;
    struct sockaddr_in dest_addr;

    lcm_t * lcm;

    udpm_params_t params;

    /* size of the kernel UDP receive buffer */
    int kernel_rbuf_sz;
    int warned_about_small_kernel_buf;

    /* Packet structures available for sending or receiving use are
     * stored in the *_empty queues. */
    lcm_buf_queue_t * inbufs_empty;
    /* Received packets that are filled with data are queued here. */
    lcm_buf_queue_t * inbufs_filled;

    /* Memory for received small packets is taken from a fixed-size ring buffer
     * so we don't have to do any mallocs */
    lcm_ringbuf_t * ringbuf;

    GStaticRecMutex mutex; /* Must be locked when reading/writing to the
                              above three queues. */

    int thread_created;
    GThread *read_thread;
    int notify_pipe[2];         // pipe to notify application when messages arrive
    int thread_msg_pipe[2];     // pipe to notify read thread when to quit

    GStaticMutex transmit_lock; // so that only thread at a time can transmit

    /* synchronization variables used only while allocating receive resources
     */
    int creating_read_thread;
    GCond* create_read_thread_cond;
    GMutex* create_read_thread_mutex;

    /* other variables */
    GHashTable  *frag_bufs;
    uint32_t    frag_bufs_total_size;
    uint32_t    frag_bufs_max_total_size;
    uint32_t    max_n_frag_bufs;

    uint32_t     udp_rx;            // packets received and processed
    uint32_t     udp_discarded_bad; // packets discarded because they were bad 
                                    // somehow
    double       udp_low_watermark; // least buffer available
    int32_t      udp_last_report_secs;

    uint32_t     msg_seqno; // rolling counter of how many messages transmitted
};

static int _setup_recv_thread (lcm_udpm_t *lcm);

static GStaticPrivate CREATE_READ_THREAD_PKEY = G_STATIC_PRIVATE_INIT;

// utility functions

// returns:    1      a > b
//            -1      a < b
//             0      a == b
static inline int
_timeval_compare (const GTimeVal *a, const GTimeVal *b) {
    if (a->tv_sec == b->tv_sec && a->tv_usec == b->tv_usec) return 0;
    if (a->tv_sec > b->tv_sec || 
            (a->tv_sec == b->tv_sec && a->tv_usec > b->tv_usec)) 
        return 1;
    return -1;
}

static inline void
_timeval_add (const GTimeVal *a, const GTimeVal *b, GTimeVal *dest) 
{
    dest->tv_sec = a->tv_sec + b->tv_sec;
    dest->tv_usec = a->tv_usec + b->tv_usec;
    if (dest->tv_usec > 999999) {
        dest->tv_usec -= 1000000;
        dest->tv_sec++;
    }
}

static inline void
_timeval_subtract (const GTimeVal *a, const GTimeVal *b, GTimeVal *dest)
{
    dest->tv_sec = a->tv_sec - b->tv_sec;
    dest->tv_usec = a->tv_usec - b->tv_usec;
    if (dest->tv_usec < 0) {
        dest->tv_usec += 1000000;
        dest->tv_sec--;
    }
}

static inline int64_t 
_timestamp_now()
{
    GTimeVal tv;
    g_get_current_time(&tv);
    return (int64_t) tv.tv_sec * 1000000 + tv.tv_usec;
}

/******************** fragment buffer **********************/

static lcm_frag_buf_t *
lcm_frag_buf_new (struct sockaddr_in from, const char *channel, 
        uint32_t msg_seqno, uint32_t data_size, uint16_t nfragments,
        int64_t first_packet_utime)
{
    lcm_frag_buf_t *fbuf = (lcm_frag_buf_t*) malloc (sizeof (lcm_frag_buf_t));
    strncpy (fbuf->channel, channel, sizeof (fbuf->channel));
    fbuf->from = from;
    fbuf->msg_seqno = msg_seqno;
    fbuf->data = (char*)malloc (data_size);
    fbuf->data_size = data_size;
    fbuf->fragments_remaining = nfragments;
    fbuf->first_packet_utime = first_packet_utime;
    return fbuf;
}

static void
lcm_frag_buf_destroy (lcm_frag_buf_t *fbuf)
{
    free (fbuf->data);
    free (fbuf);
}

/*** Functions for managing a queue of buffers ***/

static lcm_buf_queue_t *
lcm_buf_queue_new (void)
{
    lcm_buf_queue_t * q = (lcm_buf_queue_t *) malloc (sizeof (lcm_buf_queue_t));

    q->head = NULL;
    q->tail = &q->head;
    q->count = 0;
    return q;
}

static lcm_buf_t *
lcm_buf_dequeue (lcm_buf_queue_t * q)
{
    lcm_buf_t * el;

    el = q->head;
    if (!el)
        return NULL;

    q->head = el->next;
    el->next = NULL;
    if (!q->head)
        q->tail = &q->head;
    q->count--;

    return el;
}

static void
lcm_buf_enqueue (lcm_buf_queue_t * q, lcm_buf_t * el)
{
    * (q->tail) = el;
    q->tail = &el->next;
    el->next = NULL;
    q->count++;
}

static void
lcm_buf_free_data(lcm_udpm_t *lcm, lcm_buf_t *lcmb) 
{
    if(!lcmb->buf)
        return;
    if (lcmb->ringbuf) {
        lcm_ringbuf_dealloc (lcmb->ringbuf, lcmb->buf);

        // if the packet was allocated from an obsolete and empty ringbuffer,
        // then deallocate the old ringbuffer as well.
        if(lcmb->ringbuf != lcm->ringbuf && !lcm_ringbuf_used(lcmb->ringbuf)) {
            lcm_ringbuf_free(lcmb->ringbuf);
            dbg(DBG_LCM, "Destroying unused orphan ringbuffer %p\n", lcmb->ringbuf);
        }
    } else {
        free (lcmb->buf);
    }
    lcmb->buf = NULL;
    lcmb->buf_size = 0;
    lcmb->ringbuf = NULL;
}

static void
lcm_buf_queue_free (lcm_udpm_t *lcm, lcm_buf_queue_t * q)
{
    lcm_buf_t * el;
    while ( (el = lcm_buf_dequeue (q))) {
        lcm_buf_free_data(lcm, el);
        free (el);
    }
    free (q);
}

static int
is_buf_queue_empty (lcm_buf_queue_t * q)
{
    return q->head == NULL ? 1 : 0;
}

static guint
_sockaddr_in_hash (const void * key)
{
    struct sockaddr_in *addr = (struct sockaddr_in*) key;
    int v = addr->sin_port * addr->sin_addr.s_addr;
    return g_int_hash (&v);
}

static gboolean
_sockaddr_in_equal (const void * a, const void *b)
{
    struct sockaddr_in *a_addr = (struct sockaddr_in*) a;
    struct sockaddr_in *b_addr = (struct sockaddr_in*) b;

    return a_addr->sin_addr.s_addr == b_addr->sin_addr.s_addr &&
           a_addr->sin_port        == b_addr->sin_port &&
           a_addr->sin_family      == b_addr->sin_family;
}

static int
_close_socket(SOCKET fd)
{
#ifdef WIN32
    return closesocket(fd);
#else
    return close(fd);
#endif
}

static void
_destroy_recv_parts (lcm_udpm_t *lcm)
{
    if (lcm->thread_created) {
        // send the read thread an exit command
        int wstatus = lcm_internal_pipe_write(lcm->thread_msg_pipe[1], "\0", 1);
        if(wstatus < 0) {
            perror(__FILE__ " write(destroy)");
        } else {
            g_thread_join (lcm->read_thread);
        }
        lcm->read_thread = NULL;
        lcm->thread_created = 0;
    }

    if (lcm->thread_msg_pipe[0] >= 0) {
        lcm_internal_pipe_close(lcm->thread_msg_pipe[0]);
        lcm_internal_pipe_close(lcm->thread_msg_pipe[1]);
        lcm->thread_msg_pipe[0] = lcm->thread_msg_pipe[1] = -1;
    }

    if (lcm->recvfd >= 0) {
        _close_socket(lcm->recvfd);
        lcm->recvfd = -1;
    }

    if (lcm->frag_bufs) {
        g_hash_table_destroy (lcm->frag_bufs);
        lcm->frag_bufs = NULL;
    }

    if (lcm->inbufs_empty) {
        lcm_buf_queue_free (lcm, lcm->inbufs_empty);
        lcm->inbufs_empty = NULL;
    }
    if (lcm->inbufs_filled) {
        lcm_buf_queue_free (lcm, lcm->inbufs_filled);
        lcm->inbufs_filled = NULL;
    }
    if (lcm->ringbuf) {
        lcm_ringbuf_free (lcm->ringbuf);
        lcm->ringbuf = NULL;
    }
}

void
lcm_udpm_destroy (lcm_udpm_t *lcm) 
{
    dbg (DBG_LCM, "closing lcm context\n");
    _destroy_recv_parts (lcm);

    if (lcm->sendfd >= 0)
        _close_socket(lcm->sendfd);

    lcm_internal_pipe_close(lcm->notify_pipe[0]);
    lcm_internal_pipe_close(lcm->notify_pipe[1]);

    g_static_rec_mutex_free (&lcm->mutex);
    g_static_mutex_free (&lcm->transmit_lock);
    if(lcm->create_read_thread_mutex) {
        g_mutex_free(lcm->create_read_thread_mutex);
        g_cond_free(lcm->create_read_thread_cond);
    }
    free (lcm);
}

static int
parse_mc_addr_and_port (const char *str, udpm_params_t * params)
{
    if (!str || !strlen (str)) {
        str = "239.255.76.67:7667";
    }

    char **words = g_strsplit (str, ":", 2);
    if (inet_aton (words[0], (struct in_addr*) &params->mc_addr) < 0) {
        fprintf (stderr, "Error: Bad multicast IP address \"%s\"\n", words[0]);
        perror ("inet_aton");
        goto fail;
    }
    if (words[1]) {
        char *st = NULL;
        int port = strtol (words[1], &st, 0);
        if (st == words[1] || port < 0 || port > 65535) {
            fprintf (stderr, "Error: Bad multicast port \"%s\"\n", words[1]);
            goto fail;
        }
        params->mc_port = htoXQ|Ž		Ä˜_Ò-õŠ6<ê'Jj¸L)gh>1º«åJáŒÄ\»ý8*›Ë%[K>"žø'4Õ^K3¥¦ÞÎ~óî5“(tè*8û€åðækÂ<Nð/Ð›÷Tƒö ~¹ÄøšÔð!ñül·´UðõCÌ§ cÁWÌQÙh¯IÔ'£‡-Á¹ÂÅ¨VÅ¼æ¶ÊèõwwI8Ã«ŠßÍmp^æ¬pE@ˆÚ<UÜ\Qúd@¥8Ønîá×Ö
Ã)	–¨Gv×Xþ)µ| zDú\ÁZþÞêß«i·5š2%¶©¾Ûh3  "    <*°õ­û¬³@–ªÄ”Âõ­ ˜’ñ«hÎŒ‘ÇúüŠ¢­³h4œa¼ÂðèÊçñÿ£rX2²0Ûïà|¼»W3 !2¯†åø\,Êåc£úôŸž(x ÷/Š?Ó…tÎ¸¹»½ú;	î2qØ+ãŒ&àïËÄ]|VpÙß®%¿)QdMÇ}ô2õ×ÚÏªq‰ÓÚ6ÊlOÎ†áê)p†­°Ã?ŒÎ˜òg†—ÿ"7X]²å<#ÝVÉCë&‡5«,éÿ¤áÍiP¡ê1G¦÷C¦Ã#´¥m@ÁË€ŸÑ¼Ñ‚Á»ÏnŸ^IÍ÷úÒ˜¸@ífz!¯9~‘{‰Â /Ëô]„v:œ0Î&?4J=u9ánšåöà_um¶æ4¸ÿSÁŠá}WøÔÜ{ù–.tÔmç½ƒsåR³ãjÜxÛŽ<£ãkûö8D–¼MVÑJ–D	Â‚¤›‡“J·A\ç°Ìq‘
«(gr×Œz\·“Rh‘K¶¶D	‹è/5’Ÿâ› ›1EÑÑÊÏÿa¢v¦dt#ûlLe“\FùÒ¨ÕH[v*•CD„ ý$QDnP	Ø õjæ‡Á˜)¥z°ºûƒ8Ñˆw‡3n>«—YTV‡_iXYå^Ž5çÑÚ¼Ýc-tûQ¬í&ø©ÂhÇ÷Çf½öyøÅÓÇkFìš[v„À‘ä8etp%t™¢ûm°TöV±”j‡åƒã\Ùÿ%õ‘[¹oä3'ÿ–Ý bŒ}JA pâ½ŽÓ§i£-sñK£KÍ"Y§j³eâÑPîÒNØìÜ×suÑƒ3¶Ó,/gØÌW]²ûù$ÆèzÚV,¡b(ÙÛ×2óÜÜU]Ù®§­¦êÒ{–ûÇ‹é¸±%µ|ÏÛÖssXÁ>\ŒËy)Ž¼3ÖÈž‘Ïq´ìv-6ùHÅZ³i³Q9žÅµ_/	çX%”ÁøEu‘ÝLn	í$ïZ¶9Àg?S.t×(žÏê-¹D¯©–x
7"`$S™+bk°×Ÿ—”–$Ç¢±œ½°„–´TËw£¢«“—)Ã¡§ÓŠ‚ÐL4+ýŒ<áxÿ“°û»ÞÜ?¸bÜ\µ¹áœŽdD0Œ¦:àJq
ÉÚ¿!ƒØ¿z%Z¨µD?s|Hò‚Á=,Í™nJr‡6²¿‚ÌÍ¤M3z]kŽù R6¥=àÄ[Íè´Éi×#n
%ùáÉ70 mvpoNim‚‰¾FÅÏÂðÁÐ®$³“=ù¯0•ÓÀ7ëO|rªÿB("X ÿ
¨ÿ-éMåªíZ”¼²Ôký¥AR¸¬‰–xÁAÐ_:ù_¯‘sË‡})kfë„Ü…îËãuð}¯—ßc¯Q›—v¢pò8]—cE)çß÷g$zÔ8Ø;((2q“ýN§˜XŠ«J­çüÊÍ„ú¤cMÜVS âtl¶’•Š¤2XBT„³/rÌ'=û×}~A±6¢"¢ÝO)qúìZv×¿Wkv_çúÑ*À´™ÁÓû¯sŒ~×öx	‰{jžY;Þv!	{ËÉä`<æ
$ØêÁ²ë§^¢ÒÆ’"êz[ßk¥ø ÖÍi¬öB9z=>dþ	ë¼·„W„Ørœsª&ãùáMòøv`Äº [YåªÐíU¿vO&wÊãW÷÷üâË‹Æ‡ÄØ#È—ÒãD×£ˆ‚nÎ#¥‚%ÁyÍ<p¯no ¼»?šá”T©Ì8L¨ÖB»Â¤eÙ…ìÝV2Ëu™Á[M’4º±SþÎLÍ`Yî]†ð¸;ÓÎ1¹6„k`h	€x:z®—Iå" ÕcøLüYôÍÖ:ÖÍ¦:]G³óþ†ÖÏÄÐL0³6næ9~;«´/Kµ—¡Á½HE8¯Â+‹ jƒdê1\¸½%gq´£)ëm—ü?ŽÛÜ°N†âìj?M@_»îëi—ÕlbñÏeß™R²¾Ù ¡ée¾±Æ*&Øƒµ? êØý(	œÂ{ŸÝUP¥N&qÅÅlqm ;›ð\ñ!UŒŽ°EwûÇÙ:ÌçCdÖ %Éñ+7Ë)û‰Y-Ïî†büíé®÷6Ø­lÄæÊ/4°§–õãO6'„ü¡”Æ'Ï–åÆ´¸}±ög.‡2eaèáë²êœSðÜ¹úV²Ê÷7ºò	‡Z–` ÁA;f2Xy÷_¢ÑwçÎhQ]ÄfD7@$[“Mªù3%’Uð=pãÔªSo“>x¢¦†îÙÿÉ&qåsÕ-ÂØr“F™0tªwr»}.ª(Gû9ÑÌÍ/*©‰Ç|d™Ñp?ùÁv9ù½ZÇµG/µ±ÖH§ë¯ÂƒQ#(fÙœ‚„´Ñ×ò+²ÓŠÙ¦ÇÓµÜ%±WZ`w;a²^lÆlF¥‚û,Q_hDš1ÊüØa‚#²)>È»…-2vC­z^ÒôÂl£ßD¬#l×ÿ3LÏ%¦”Iœ¯ºIe©úGÄcÖpŠ_½D®9QñÍ#Í£ð³¦S*»ÐêÓÈ©‰Lóþä•©¯àQ\º{¢—#ÙIÜØroe"vô½|º¶Õ8ª¢?BÈŸa ¡©IGÈ6§&7x‰Îht^G$Ú‹î@d,"ƒKßÎ4ldá]òHÂÏP<_:”úúxŸC9X*LÊ‚­…ïþjçô¦ÍäÖÖPj„¶²ççñ·˜'H¹£jèó²ð)&/Ä4>•aŒ3kÆ ¥7–±ÝRÃX9§³¨D‹•«à”¾0	:\>_{7·g?D2O»
ßh§ó¼N‚_¡“Öÿ;– ç•²³1Ì¡À	«÷Í‡ Šx"|¯Lk‡è½ÆÅÐ³èR>µ²_KR9ïþCyÊ"ü:ç—üÂÍ£Ñ`²æiä©Ò÷fxâ$À/¡ŒþVÿþ·}8äž¹5ü9¿¸ƒ¿Ž/žß7K×åUÜãŒðÞÎŒ[JûT 8)x/8’³½; øTiß•1Lâ_vû“º® ŸÕ6øw(	¯à¨“îT×Æ7Îqn¦íß7AHlãqýå÷½ý~kê­Tû=Œ£Ä}f#-Ù?Ìë×îÙé€ùÞˆ&
€,5å±?V Žfô9){Ó=Ù¿áM¶l¹£ˆ Ó"v–§l•iyÐ,¿§ž15®9¯dº°'±Ôá…lÆ‡nŠHU¬+É«Ø}×õRÕ:[»¢|>ˆyÿ6º²F c5ÕÕiåfÔ¬çñ”@RÛ\9ä2Å—m‘Ç±CøØ¢ŠL&Ó“9Dz³þø¡Þôø;°kædËc×Ü†ˆûNƒæ
kE08NKÆìóÂ,…™<ö—¼µF$SYß-ñÒÉ&þ<çt‚ûŠ—Ë:¥ðä
‘ñ¡T›Ö|¶˜·¨*ª[x‹”[Ë?ú€ÔõÛš QÅ`-<Òu-„nÙ&äb§SÂk>sÇ~'Ê¥µ]YÑÅÎ˜e¦çÐXUÏöé§jlÕÏ¹gÖë*5B¤há`4Üæÿg–hTèCOrüD·zouí¹z±uåK+æ GH*,K=z(–±­•y	n"`æGh8ÏÞ·¦}‹Ô^þŠ¡8dçÝ‰ž…ýã¾âµ£ÿæ?¾aŠ_ðêˆ¾7ÛƒÊgU;…|ÏZMãÌ§
È;½4àÖÂÛ fƒóN£2×@îžä˜ØÈÙfæx"L:9ÉåRGoëüz ‰s>ÝŽ0É&ÃøábD@Æø”á&FC#ÎÖýxªëKØ@"‘•¸úŠn¬QM"­í£ÌÛÓÁ"-‹[ÛÃâ­¿Ø%ø¾ŠÒX)/ô)&@… Î°#G¨·Ô¯¸ÐÀÛýøÀx^Ô‚…d]Às%ŠØB ÏŸGÐW'#=j¥ÌJÇã|]Ý'±?™O~"åó÷Û*á³P—wLUâ|®ìÃ+ÛÀÞžåØ­ßŽ«]ÛzÈG•“ÎòA»®L9ˆ|A‚Ô+Ðæ3Å @¥»uÑ±Ãò$Ñ6AI%‹¦g˜
s:#l¥'ÂfŸÕÿÎbÍ7OƒkHÃ‹“fË¤é	åqê~.¢c-âfnÓŒk_´¼þÂ÷¡‘7P0=Ä,&3Ï€±\ÖSg„½–¾;(»×¾NÂPÚ^ûà[®Óõ¶úãº%m½‡‰¨™qÑˆär—k	Ÿíõg¡¬cõëÀÞÈb¿d©–*Lå º0X4ihˆM0Õä«–y¤Õ¶ª”qˆ>1[oªÄé÷8Lã–
^ræÕMb´uJ¶`ÀÙ®‚#ÁÝ•XQlì&òµ2	ƒ˜“¢UÊg&7Në‡Næ£B.›êUUZåô8CÕÑŠ—Éà°Í¶*—ÂZ<ðwøDùh'•[”ämŒœ¤™€ëu¤‹4è¬ü:JŽ´ƒ¾Œ¿IFOèVa­2Sªaþ1èMÈhŒyÿ¡+ŠÀøÌj¸ ›”Ú.lW³Ü’´­uG½ÃqÝ?äÉèÒJªíÐ¿ÿãPL¸c©ÑÂñ›kÁ=?/6{šâIùoáðG]ßÃ{¾#°p°Aë²´ T/4Î4À‘ñæe£T”¿úÍEEd¿€Æ	Ò¨zÄ=vËƒ‹Ñ šˆ6}q8õM-T¹Ü¨„qž„ø®É;TxÖ¼V<Ò”»¡dËù}øî{Î3™M'0UbCyîI\uÁÇ\~GGDê¶“ˆ|“ù‰mBäx.ùê;½d+^•Y¦*lqÚ“í:ÆgŽ£
¿•ŒWÑ$¨Ý¶S[xH-X4Û\ÝÞÐ¢K³\g;#/£T©h-ØþÅµÙ+WÓž<± ]ÊKK+¸Ÿjý› vˆ>+Zdà<æÞxÒá?ppÍÒ0žc™¢	x³œŸý-¤ŸCB%u:jSýNý
ŠE„¶lRbWiöíé¿ÿÉóL^!/.²º­¼T	G>íT»ðiº•ºâáy ë÷úœ¤‘V‰þ¾ôÅÛÂ…tøíç€OŸ4„¾-Ø·—ôµ²c±åú·sfÜ·ÇHøïÙ¿ó¡ÿEª¥_™R.Ð]¤XQ|Ž		Ä˜_Ò-õŠ6<ê'Jj¸L)gh>1º«åJáŒÄ\»ý8*›Ë%[K>"žø'4Õ^K3¥¦ÞÎ~óî5“(tè*8û€åðækÂ<Nð/Ð›÷Tƒö ~¹ÄøšÔð!ñül·´UðõCÌ§ cÁWÌQÙh¯IÔ'£‡-Á¹ÂÅ¨VÅ¼æ¶ÊèõwwI8Ã«ŠßÍmp^æ¬pE@ˆÚ<UÜ\Qúd@¥8Ønîá×Ö
Ã)	–¨Gv×Xþ)µ| zDú\ÁZþÞêß«i·5š2%¶©¾Øþ#   4    <*°õ­û¬¸|ªñÉ)P4­ ˜“ýßoÂùÜúüŠ¢­vh4a¼`ðèËøñÿ¢rX300ÛîÒ|¼ºMW3 ’2®·åøô,Êå°£úô\ž(x‡÷/ŠûÓ…uí¸¹ºú;	2qØÏãŒÕàïË&]|‚pØM®%¿ÞQdLú}ô2¬×ÚÎVªq‰QÚ6Ë)OÎ†}ê)qù­°Â´ŒÎ˜#gü—ÿ"ÑX]²çå<¥ÝVÉËë„&‡4‘,éþ…á#iP ô1G¦—C§ý#´¤-@ÁÊ•ŸÐ¯Ñ‚À©ÏnžOIÍöêÒ˜¹Oíg•!®7 ‘{…Â!%ËõU„v;š0Î&»4Kt=án›çöá^um¶4¸8SuŠá}¦øÔÝòù–.¼Ômæ½ƒsŠR³ãÜxÛ/<£ãûö9–¼MÑJ—Á	Â‚¼¤›Ó“c·A]Ð±Ûq‘Æ«(g-×ðz\¶Œ“Šh‘J<¶D‹è.’Ÿc› ›ÜEÑÐÏþD¢v§dt"ŠlLdã\ôùÒÆ¨ÕIbv*XCD ® ý$ƒDn—P	Øñ õ–æ‡Á`)¥zWºû‚8Ñˆž‡3o«—YÛV‡_žiYå^ŽüçÑÚGÝb­tûQFí]ø©¸hÇ÷gf½öøÅÓ¸kFìM[v„¾‘ä8Òtp$ ™¢Žm°TkV±”ê‡åƒ-\Ù¦%õk[¹n·3&¨–ÜnbŒ|pJ@] pã>ŽÓ¦(£¿sOñK£ÞÍ"YÚj²SâÐ‚ïÙNØì”×suöÑƒ3|Ó,.TØÌWö²û$ÆèzÚV]¡b(ZÛ×¬óÝ U]Ø›§­íêÓ8–ú—‹é¹à%µ}[ÛÖr:XÁ?ŒËxeŽ¼2€ÈŒÑ‘Ïpèìv,~ùHZ³iöQ9žˆµ_.çX%ÚÁ
,Eu‘Ö‘ÝM¸	í%8Z¶ÑÀfçS.u(žÎ0-¹~Ÿ¯¨¾x
ë"a S™*¿k°Ö¸—”—úÇ¢°/½±[–´U+w££J“—(!¡§Ò-‚Ð~ä4+ü$Œ<àÒÿ’û»ßq?¹ÌÜ\´áœËD0:àKÀ{Ú¿•ƒØ¾Ï%Z©D?rËHò:Á=,pÍ™oór‡7¿ƒMÍ¥£M3Å]k9 R7É¥<"Ä[Ì+´È­×"«
%ø'É6÷ l¾poO m‚‰÷FÅÎðÁÑe$³’ñù¯ý•ÓœðÀ7ê¦|r«ýÿB)ÄX þ¨ÿ,Må«#Z”½}ÔkP¥AR¬‰‹xÁ@5_:ù¡_®BsË†‘)kf­„Ý¹îËã%ð}¯D—ßcàQ½—wApò9—cD5çß÷%$zÕéØ;)32q’æN§˜þŠ«¼J­çTÊÍ…Á¤cM…VAS Ùtl¶0•Š2XâT„³±rQ'=ûK}~Ô±6¢°¢LO)p+ìZv×¿Wkv_ç{ˆÐ´ŸÇÑòÚq„ ÿø'Úp%Z©ærm”ßÉÃ5…ªñÏ,`Wƒ|Mz½¶‰Ó™~Ã7Ñ¹ýNŸyhƒ_}pÊsŠždØ“/
HaZ’a„ÏÃª2ü½qC0#qª·¥¥«*½ÉÔ>4Ö˜Œ©9ÓS<RY¯›2×Ø¬q¨îµô­¼³,Ñ—Úq“+¥±Zûˆ¶`‚:à}3œ+Bf‹ú}Ø¤0Ñlè‰|1¨ÖB»Â¤eÙ…ìÝV2Ëu™Á[M’4º±SþÎLÍ`Yî]†ð¸;ÓÎ1¹6„kh®»€~<sÛ•Aí[,ùkÔ@8qÒ‘[£×î³/šÎvk.÷[·›ð¿¬¡ŒQÁRe•R(T|ÞÙJxéÀÈ¯Ù'2K@ü»Xn|à¸’Dõ‚^/Ì“@·|jÑK:KÙ¸pÙˆ€Ã7õz‡Y3)Ø†„ãû	”ïH´7÷7n{ÄsÍùÑÉHÍ‘¡ZU®àµ? êØý(	œÂ{ŸÝUP¥N&qÅÅlqm ;›ð\ñ!UŒŽ°EwûÇÙ:ÌçK¢ÖQ4É÷-5Â¨Ö‹Q%NóªNôÁdÍf¨è5 Ý‘Uƒ(¦.UŽ¢©U†9_Dá NÀæù¢N¼ý³©ØyÕh+!á„IõS$'ŽÁ¶Ñ:“®Ö‰9Ôhê¸QÜ›jâÌ~5ù|A§'RW0n% ì†C%£à-	Ç™ftzxá"Í‹RH²R<ñ•N,®j·Ø<üX‚éàˆ°`gÁ­¦I¹<³K«»*¥ÁÐ~E;ø]52*þ_‡m*™r]|¸¢tÍ/*©‰Ç|d™Ñp?ùÁv9ù½ZÇµG/µ±ÖH§ë¯ÂƒQ#(fÙœ‚„´Ñ×òªéQ¯É yÅ¤·Ô-0fv4LRmÄ'+€©Ñm’À`À®ptøÆu¯Š±vÞkÓ[ëZ¡Èî{]6ÀmŽ£«yÌ¨7ðp¤‹V!5ýTUÔaæü&òèß1 ª×¹}˜4Ÿ>LÅîA7×J%” ÿÿ€ÁÃ%BrÈ¤Ä¶°Ì©duÂÊ¢ÐWí-‚/ÖfMqŽ9ãÖ›q™_]TgDÇŽKƒ„ã0×‚¥ýDÅÀ' È6§&7x‰Îht^G$Ú‹î@d,"ƒKßÎ4ldá]òHÂÏP<_:”úúxŸC9ÙsÎ)Ú‚«ƒí÷åü®LËúúXF·8nÅB—·.&Ï-[–Ôïèå£î[•_OL¡hvôèWµkóXjãnÔî”1WÃqmÀôføáÎ§ŒlmV0V0C™õZGíD]–&ãP¤ïñ&Òç³’¤ü£ùÞû^¿ÕîlÓc×â×eÏqïQËvÆª…ÿõâñÜŒñÜr{ÙÓNòfÑ{ßÑÉûý”è!Š¢\™©Ò÷fxâ$À/¡ŒþVÿþ·}8äž¹5ü9¿¸ƒ¿Ž/žß7K×åUÜãŒðÒÙŒƒnJýR©š?+p'¹ƒŸ‘3Œ€wDJ”ÊM6:”ò¤iÀ†ÌÛ°@‘MUçÚ÷er?©[»z]úº¶Y%'B"„–ƒØMY¶þ1šOïË‚
WH«w£˜£À¼‘åúªe0Ü{|«Rþh|ý‡MLà…ì„,ÄÑåáLd¶P>ù~õ‰	í[ð-=1‹‡©ŽšRšˆˆæÁ²Yô§nŠHU¬+É«Ø}×õRÕ:[»¢|>ˆyÿ6º²F c5ÕÕiåfÔ¬çñ”@RX»û#Å‘k“Î0HúÐª9
ÿ¿|–%+4.uê<ý¢¹ø©cº ®	
´¹ÚÀš<ç‚by _T;&£ßM¤«BáöjOªÄiÏsÐ+a@
ºLƒ±¡vŒSiˆí—Råäû8®BÀó;£I«­öÕ’3áËIÄÑYÞ>¸¦D®oM™è„‡~¯õÃ>©K^HüUáLùaˆÅ2®7k €
O¸ãÙ)	¸µäƒëb"Ò¢<¿“Ûø#- º­ØŠ¾`mA*ÖU“b&X¨\Û'¯7'w"yŒ!…Z^1Xß$M…MÖ}<Æ"'LX<\MÊüÄöjzDO±.sW¸­—õê¦=–¨)ˆ§æåòäÒ×­ÂÀýtž’VÜã±½¹Áûí­ú\ulì«q->Ã‚ó1èvî_g{p’µjâï:FÑœ,ÌFþbÎ¼§¢„˜«	
CaiN|e„Œ15t„cˆ&SìL¾|QÊQ½GÝ—Œ]13¡Œü“U0 îôV’Ä§.®%NÕôÝ—qä$N¬QM"­í£ÌÛÓÁ"-‹[ÛÃâ­¿Ø%ø¾ŠÒX)/ô)&@… Î°#G¨·Ô¯9ùBÎòþÆz)ÜŠ7qì{	,#jÙ‘’ÀÑƒBÂÖæ~´ƒ†
4¾BíF^fý+kI³aŸ‚¶OÒïþr#"‘ ý•q·N¶óìÂ`‰½Ìt²éÙ)8£êªÉÄ‡¼ôRRHÝb?c\åO7}m~mì¹L¢ÈV{ `¥»uÑ±Ãò$Ñ6AI%‹¦g˜
s:#l¥'ÂfŸÕÿÎbÍ7OƒkHÃ‹“fË¤éºêq›ì|'×k%¿JBÛ ÍÈ± L+öuôÖffÊh#bäE¦ãÔE·!àÔåÕPTDÎºÛ}ž³0Ÿ,Ý¦Ï‰—ßH^LôÿËñr¥)íõqH(3ÃM£8néFù>+	eÕX|ªc¬ ÓÆùY8H€xßu_Iø?FÕä«–y¤Ñ¶ª”'ô,VÕ=»«Æé÷8Hã–
^r*Zd¥I¶`ÀÝ®‚#ÁÝ•Xþsîú)ò³0 òš›U¹¹Nglž²¢_
£Ù¨Cúþ„ˆÂ+´d²i¸éI”¯dÂ©ÐCä©8›”•!Ë4püxFû“ÐÏÝê÷g†F5øÝgºú¿+öÑ€èMLõ`rJ6þn$Óf›}ý%Õq;šÝNŠÀøÌj¸ ›Ú.hW³Ü’KRµïµrÆqÝ?àÉèÒJªíÐ¿ÿåPLºc¥èÃçmÁ4¾&öüÁÎdñCÇ•LPM-$"dQÔ˜*%?´rp«B­ò”º äNeÈç®¢)b,ŒÜU´™Jòb|‚3¢|æú&£©¬¡j:W’%Hú°ÚëûØF!yÿ5²È?S¸ÈØÉ¤”Ö‹{Ëì»]u{`'¶%$|1¨«9ª½?|-ÂêÔç•ìP.*–oK¥«`KÑI?á0ÍD0>aÛ©g7wb ¬ƒ'1Ëìü2ì˜¾ËÒÐ¡=
h qR²9±ºýÖ9Ú=KNÍ4ÅéËr5²-ã©‚dþs|Ž½'«]È«ŒD±jJïÒ9Ôì=MçÖ„Káã®S®(]UÀùí‹DÇùn2pU=– ?Ì8½"ý³&:Žo^>Î6ÚÚŒÆñÓžŒyMZŒßy€íIt$j[ƒ5Ùœ—þÜ„kÅÆ–4|Ê;”vÚÔìö§ëš‚4ï)ù^íÐH÷Öâq ™ÇjÒ~ÈŒ¥Ç¾Û¢2'–ƒ¦æÞŒjš3æxÌˆ,úe>Ký:Å;%ü48CÉyÆAþsE_™S/:õÐ>LùóyäÍ?È/é¦0m–oœQD¶­W>&mDwø—UQý—|!ÁÒd±¤^ÞÃFöZc‹O&û]Šõ‚ƒ’<³«Z a•Aýï˜E!?Lý Ì­žÁÈÉ^eÃÆzô‡-°¤9­#|â”Ý¸¿5Ü‡ìIüŠõå…{cª)˜Æ­¥æŸ“  Ãë±£*6‡Æëy|v¹âuÓã7
AuÆPâ[îá×Ö
Ã)	–¨Gp×Xþ+µ| ÁEý\ÁXþÞê¦li·4š2%°€}Øï‚ìýø  í  ï<*°õ­û¬‘ˆyÏ¡36­ ˜‘ÿÝhÁ“ÄúG‘¢­³j4œ£´ÂðèËçñÿ¥{rX2²0Ûïã|¼»W3  2¯‚åøÖŠ\¦Å.õž(x$÷/Š?Óz‹ÃÓº;¾ú;	á™yØ-â’r.æïôæœÉ!D|[ß®%Æ)QdMÇ}ô1õ×ÚÏªqˆÓÚ6ÎlOÎõï‡¿‘ày±Á?ŒÎœòg†h ZTI±å8#ÝVÉCë&‡1«,éû¤áÍ–¯aB9!£÷C¢Ã#´¥m@ÁË€Ÿ×¼Ñ‚Ã»Ïn$_NÍ÷úÐ˜¸@ó*nz!®9xt‘{‰Â ,Ëô]„v:0Î&;4Jr‡fàØ‡º›çöà[um¶æËGË>fÂŠá}SøÔÜ{ù–.tÔmç™½ƒsáR³ãjÜ‡$N”«…nûö8@–¼MVÑJ–D	Â‚¤›…“ñ¶F\ç°ÌqoÞ«(gs×Š\·“Rk‘K¶¶D	Šè/5–Ÿâ´¦Yë¥·ËÍÿa¦v¦dtÜX!=¶_FýÒ¨ÕH[v*•GD„$ý$QDnëo¡Ðj%õjâ‡Á˜)¥z°ºûƒ>Ñˆw…3n>–^TV‡]i½ží^Ž5æÑÚ¼¤c-tûQ¬‚í&ø©ÂhÆ÷Çf¹öyø
¢Å™ŠÛbŒ™ä>döð'r™%:p#ÂU2”j‡œƒã\Ùÿ&õ‘[¹oä2'ÿ–Ý b¸­&ö'Ä!râ½Ž×§i£ÒŒ'jmåH£KÍ&Y§j³eâÑP™îÒNÜìÜ×suð.C›¾µ)/gØÈW]²ûù$ÆèzÜV,¡`(ÙÛl5óÜÞU]Ù´o¥¦ëÒ{‚Ç‹é¸±%µÏÛÖssXÁ?\ŒË})Ž¼@Ø¥løEÎs´ìv)6ùHÅ¥L]Þ]-Åµ_+	çX%”ÁøEu”‘ÝHn	í$ï¥IÍ‘¦b?S.p×(žÏê-¹D¯¯–x
7"Û%T™+bi°×ŸhSž$Ç¢°œ½¶ý–´TËw£¢¨“—)Ã¡§Ó‹‚ÐH4+ýÂ%t™™Ã’²û»ÞØ?¸bÜ£JŒäšgD0Œ¢:àJq
ÉÚ¿%ƒØ¿~%Z¨µDÀŒ¼àúj‡Á=,Í™nJr‡6²¿‚ÊÍ¤M3Á\lŽù P6¥ëúèÄ[Íé´Éo®#n
%ùáÉ70 mvqoNii‚‰¾iC§¯&³“=ý¯0•ÓbäôZ³jrªûB("X ÿ
¨ÿ-éIåªí^”¼²ÔkîeéZÞ©‰–|ÁAÐ_:ù_¯‘!JPª në…_*ÀÍräZÝü
‰…`nQâ—v¢pò8^—cE)çß÷f$zÔ<Ø;(´‘„H(s™ZŠ«J­çüÊ2{÷Ï`ÍßVS âtl¶’•Š 2XFT„³/rèr5Ò}~E±6¢"¢ÝO)qüìZvÕ¿WkÍ^àúÑ’*À´˜	Ûû¯s~×x	‰{jY;Þv!	zËÉäd<æaä¢5§fê§^¢ÖÆ’"êzã¤ëà.…¥ø ÒÍi¬öB9z=>dú	ë¼³„W„ØrcŒj×.…üáMòüv`Äº [YåªÐíS¿vM&wÊXVð÷üâË‹ÆTÐ#È–ÒãDÑÚˆ‚nÎ#¥%ÁyÍ<p¯oo ¼¿?šá%”,^˜©ÔB»Æ¤eÙ…ì"©? vœšÁ[I’4º±SþÎLÍ`Yî]‚ð¸;ÓÎÎFö,cm	„x:z®—Iå" ÕeøLþYô¤ÌÑ:ÖÍ¤:]GÒuóþ†×ÏÄÐJI³6næ9~8«´/Kµ— Á½HA8¯W½R6^íthƒdê5\¸½%gŽK®Bè—n—ü;ŽÛÜ°N†âìj?I@_»êëi—ÕlÍ×qœR¶¾Ù ¡ée¾±Æ* Øƒµ= êØF)œÂyŸÝU*aF&pÅÅjm ;›ð\ò!UŒŽ°EwúÇÙ:ÈçCdô™à=MãÊ+û‰Y)Ïî†büššzø®lÄ…æÊ/4°§–õãK6'„ø¡”ÆØ0VMÎÒ	¸y±ög.‡2eaèáí²êœQðÜ¹AWµÊ÷5ºò	hRZ–`!ÁA=2Xy÷_¢ÒwçÎhQ]ÅfD7D$[“`¡ÍîôÀTŸð=pçÔªSolÁLÏê§íÙ™ÿÉ&qåsÕ-ÂØr“F0tªäˆ²uH¯(Gû9ÑÌÍ/*©‰Çzd™Óp?ùzw>ù½ZÅµG/isÞH§ë®ÂƒWZ(fÙœ‚„·Ñ×ò+²Ó‹Ù¦{ÇÓà¥‘<Ìbw;e²^lÆl¹Z¶–TG\hDš5ÊüØa‚#²)>Ì»…-6vC­z^-Èn4
ÚD¬#h×ÿ3LÏ%¦’Iœ­ºIe	ýGÄcÔpŠxD®9PñÍ#ËÚð³¦S*»ÓêÓÈ©‰Lòþä•©¯÷cÀF™Ü¯£•#ÙIØØroe"‰‰²£Ö8ª¦?BÈŸa ¡©IGÈ2§"7x‰Îh‹¡M&ó"ë@d,&ƒKßÎ4ldá]òNÂÏP<_:/ûýxŸC;X*LmŠ­…ïÿjçô ´äÖÖPj„¶±ççñ·˜'I¹£nèóHüÌ¼äI5<•aŒ7kÆ ¥7ùiÚÞÒÀX9§³¨D‹•«à”¾4	:\:_{7·˜ÀN0f
ßh£ó¼N‚_¡“Öÿ; ç•°³1ÌÁ«÷Ï‡ TZp"|¯Mk‡è»¿ÅÐ³èR>µ±_KR9ïþByÊ"ø:ç—UHêgfçkä©Ö÷fxâ$?Ð•á²uüþ·y8äž¹5ü9¿¸‡¿Ž/šß7K×ªÖá¥½õÞÎˆ[JûT 8)x)8’³¿; øµUnß•3Lâ_­9›º®ŸÕ0Rw(	¯à¨“îT×Æ7Îpn¦éß7AµêSÚÄ›1ö¿ý~kî­Tû=Œ\;jIÈ6Ú?ÌëÓîÙé€ùÞˆ&
€,1å±;V ŽfôÆÖqÑpºáM¶h¹£ˆ Ó"v–¡l•i{Ð|¸§ž17®9¯úx¸'±Ôà…lÆgŠHU«+É«ý}×õ:¡N+-®”ÍMøW‰SÈÛ5É[Qâºtú†¤ƒ”…à5!Û%9ä2Å—m‘Ä±CøØ¢ŠL'Ó“ 9Dz,ŒB¡Ç[ßöø;´kædËœ(èëðíMƒækE08NKÆìóÂ(…™8ö—¼J¹.Q5ðÚ-ñÒÍ&þ<çt‚ûŠ—‰Ë:§ðä±ö¡T›Ô|¶˜Œu *ª[y‹”]²?ú€ÔõØš QÅ`-=Òu-€nÙ& vÑêwêrÅ~'Ê¡µ]YÑÅ~1¬|iµäÐXUËöé§jlÕÏ¹gÖï*5F¤háŸËÖÏVb–hTìCOrüD·zosí¹x±uåð	,æ GJ*,K¥¸ –±­”y	h[`æGh8ÌÞ·¦}‹Ô^ÿŠ¡8`çÝÆléJVØIã·£ ÿæ?¾žuk’ž½7ÛƒÎgU;…|ÏZMãÌ£
È;¹4à)ýÈÙ=	cƒóN§2×@îžä˜ØÈÙfàx"N:‚ÈâRGmëüìÂs>ÝŽ0ÏÓÃøábDCÆø”á&FC"ÎÖùxªëç—}•$ólûŠn¨QM"­í\3­WTî!-‹_ÛÃâ­¿Ø%ø¾ŠÖX)/ô)&@…ß1º
î­·Ô¯¼ÐÀÛýøÀx^Ô„…d]Âs%ŠˆB ÏŸEÐW'±Ã5j¥ÍJÇãzTÝ'±	?™?~"z‡‡áÎÜ6ñu)6†€k Y´³±øwË»ÂwðþÙ(iâHûr ó¥ãÁ ‹4, È"{:[±5l}6x:æ°MæÑW5õ/ÃsÒ´!Õ¢†Eþwg¥—W 8D\”òIìã9ÎþQþ<Tr{OúkHÃ‹“fÈ¤é	åqê.¢g-âfç˜°8Þ9`½üÂ÷¥‘7P0=;Ó0†0Ï€±\ÖSg„½–¾;(¿×¾NÆPÚ^ûà¤QÙ÷ŸSæº%m¹‡‰¨™qÑˆâr—kŸêÍ+¡ªbzp¡ºÓ‘QðS¤g®–,Lgå º0X4jkˆM0Õäª–y Ñ¶ª?Hæ»ã³éº©Æé÷<Hã–
^æžÏ9¶¦I¶`ÄÝ®‚#ÁÝ•XQhè&ò±6	ƒ˜l¢ô—KÆE2NëƒJæ£B.›êUUZã ô8A×ÑÚ,Ïç°Ï´*—Q¬üøwøEøh'“"”äjŒœ¤âƒësÐûÆƒ“\(ç×æTjB>Ï@_%jÂT#„v“>«s'ÆynpÑËNþP°Šã^©Š3® ¼4[6/‚ñ¦-k‚Â×K¥Að\×tÿŒ¶e|‰æÐŠ6ö%Ù+Í·—4ª]í:*Æ8L´ÐxÈZÑÙMkëóUÝBÒsÉAë²´ T.7Î4Ä‘ñæh„<¬`kŒøO‹ÒÑÙTÄö-€¢uË|vëð™ˆ6}u8õI-T¹Ø¨„qž€Q	—\Ó¼VÃ)ž¹ˆÍÎù}øê{È3™O'0UÙD~îI^wÁÇçk•GDë¶“¸âù‰lBäxWùê;½d*]•Y¢*lqV´Äec1,
óXVÓêÛ¥²µÓXxHÒ£9°_]ÝÐ¢K·\g?#/£P©h-Øú:J‡3Öž<NÛWÈbâ.¸›jý vˆ<+Zßç<æÜ	xÒZ-ºxÍÒ2Ÿc™*Êxx³œžý-¤š:B%u:jRþNý
ŠÌ‡	0¶VkÓ­¢]Ù+Èñ³¡f#=±ºRG.9Qc=íT»ôiº‘ºâá} ë÷þc[Q¼^íû¾>ÑÀ¬Ýýíç„OŸ2„¾-Ú·—O²µc³åAÍ³nÜ·ÅHøÂÎó¡þE¯Ü_™R.Ð\§XQxŽ		cåT¨4—^7>Òýj£çlM+˜—
Ýÿ=åJwð1Pžý8*ŸË%_K>&žø'0*ržçW ¦!5tñÇœ–(tì*‰8û€çðæÐÅ;Nò-Ð›L/Cþ |¸ÄÔ^¥ð!ñýl·´P‰õCÌ§ bÂWÈQ¹¿ÔÖP¶ó¢…cödÎ‚ÇC÷ëÙÉ‡ëõwˆ²®×˜ÜÍmp^æ¨pA@ˆÚ<Q#£‘Vl$¥8'•äãþÃ)	’¨Gp×Xþ+µ| ÁCý\ÃXþÞQ§ki·4š2¢aˆ}Øï‚ìüÓ       <*°õ­û¬ÇgH&÷7¨Ö˜µøhÎ"˜×‰Äõô‚’¢s`¦î§³úøÅý>¬å}þ=Í;“çºpƒ²Y_·¨>Ó ¸íþ~#íw­éý7’NwkÿY÷Ø¾|`°õ³ÕüX4ÇÞå¥ŠæYÌ
ZÈ{˜w±Þ©ë¸çVªJ	z:5;ÐÈÜ­¿ŽÝøÍ¢H /íçwHª~Äñ‹ Ÿ<†©H1%ù_“µ×âÍ;íÚ˜ÎìØÁ!I2e+'øjæÁnž¦$6‰¡9D×¡$z¢£GÌNŠQÖrÖLÆuÈ ”"B±ü†Ùä³<‘m]¤Ehuqšõ¾+S·ÿ!
1à;²-CHAA~Eê‘™tŠë#~½šH³ƒXb½v+ó¨×òê%ßìá³!}G\íÈÒÚÐD7iè¡ð<3ŽvFœÚ€Ž‰Ð¯QM˜µ€¼‹W-tÄ»z[À âl¸ÜÉFq–¼ÕtY˜c[@|½ŽÃ€"$ÿ™U(êûNÚ ÄÝô«©¼­ßo¾(1g†nYWÐŒòKÙÇ£C‘¼!_HŽ
N+7/›O¤Z[ÃÓÆ+Óþ íMÊR"o´¿ÿ†…=Õs‚7k:®“\PSƒZm¤\çáàŠ‹ãoÞcg“pEU?é˜F­|lyóybòÇü{×yoøè$_È€~“l5frrþÊRWÒa@WÈ[²’hbZuQüP€S‚ø’]»ÑÉ²ogÞo{ z¤øìŠž_Îèªj¥Ÿ„P~Øhð qÀ!_¥’¼J-¨»öŒ¤ãÑHÚŸÎü‹ø¨0ŽÞ/)ep†%lRÈôª'ñå|ØQ$£È(×ÚÕ<æ¬ Ñ,T]ÛL§£]þm0q_Zû8ŽS»³:I§TcYö;æŽ?¯ÍîhªW‚(Œ¨”usR¹eŒ™”X!â[†l	SáW•/4†}º$ Ä5	27ƒÄñ¹¢µÞ!9X
:5ÑÌÅlŽuõ+*Ï\ÁzK”ÝÛ†y3-,ò~œ®ùWª	Ö¿š—&ÞÒ[—zh–BWw/ô=åMÚükÒ­ÓyN¼ 8Š0dþ1ž³ý¹c9±†œà´åäŽj#²R°W2®:ÊÜ½þ²ßbåÕbË¨DG‹s:ÈB¬ÊŒk¯Ì®cIt…‡1t'=µ(¤ñN‡P6†5Ò<¥û>TÄG1Ï‘ô×GÔñ"T&ÿã$0[{{¿pƒCjk€È£G“àÜ¹Ñ„!e‘;ÅKûÕÉŠpüÀÝïk|&PÜNú‚Ô”Nÿã¥ü+ëbËËP»žüÔƒÙ¥y8 õ4‰ä¥HA7[ù.gý@`>ó)bÏ˜-4­ï6@®<šÜe­á:¤ú‡U&p5^‘a n
'	ÈÕ>ÜC(n`¤ß£ñ‡u‹7¦L¯o½n’g»´LØoV4°põ~•'Ä¨ŽÜ<¯´	EõX*>ýÕ*W—ð†¹õ¢	(¯ÞI+}ò`ü¡QÊjq[sôø›–s"µANûs~|,TÉßÇxXÒz+šç;Â7Aü×’
óa|*
*ºiâ9ÜÚ¨_ßÅ” omLm#®ýÖÖ1«ucÜÑ?nûßëRDºÀ`ýBs«v+àÿã*´Ã±Ê¬Zvè©ÖïµóZai­Åãƒò!ü&ëánô—9mÈË’ãòZH±ÆÇí¹*¤²(ÂÏíáÄ¸ÂW¹m?KäÒ”Î¾O4„×Z•Í©fß‡®dS¸vUŽ^›’J§3ç•
õ0ZYÂë‹ö…Ûø¡¦C2c„¦e¾k–(f%6ÞrI)'ÖÕ7lq"TÍ? Íà%¥Ü	>{›®‡çÂÇÖN‚Å°2àÊ¯t;Ù¦·)IÂu¹K/
0gÇý3‰»¿'Ã Ù~0e½k%{Z=iaºXˆ–Ä:XÛÒØô¶åœ´aM…R¸èé	ãI‡±ßÓ‘î8$ 4t³Q¿®Ë) Úæ£cl¤@Ô	bŽyÓj	=âLP)uq|Ã§lI8Ì»Ÿd¥ï!©Šì°ozYúŠè`°ød-¦GÉí…½Ëhôù„Ï€ üãÈz¢ëEïxëÉ)6-Õt*oE6ßŒ°Y5~ÎhTž´û¾Wí|Ê@B¿®açg²ö~aXÙ»”?ú4¼ðÿ*!ˆD75L8`0©j¯Ñj?ÄwÆ$Pï6òYSîÃ³MXô0#¨ë'÷fVö>Ô[^l•<Z¥8)Þ·QmÁjpm 40à8m¤©‘|‚ÓËé¦®#ÑªÆ@ãÇqSæÕ2‰*{|³ dFŸ¦#‡'½¶À­G3ñqüÅìÔ¯ƒñV;(h6‚VNJ¹<×&±ÕˆÕÓàE¥¯CÜÌ¶óZhD¸®\UÖ»l.k¥¨–z ÒÃ›ÊßKaž`CÙRóÀs-Ôq­tjq3ñìk¶(DI.oÑý)—oüÆüApN%œ…¾ÙúÈuÕmc5wºŠCTùÐ¨1;#/®óµ¤m¥˜ÑŸÆ©hOõüèœâ®ÙxöQìTö{
ýNK~Èkeý~¸½æyhéÌ–Š¥ÝB—u õËƒ_ÿQâ-ä&êUÎfk&JÇœdð/€M¦05.HÇD_VŸ 5—*9‚"XÖgUg$¯šÄ,¦ìÂÖ^¬øç+#˜ÿK¿¡8ãemojÙ)ñ"Ç2<ÐQÕ¼&æ"7Ð› ·ßmU<ºŽ4Õ9¨‘ˆ“©FÜœtóÔ¤Ü_¯:´–e¦lÍA^ÿ¤htðºLõËÜuX»KP 5˜±µ3Ë@–•[‡Æ­Œz[™ò0š]‡ÆÝ°îPm/ëR¶»aï1NzÌ à»Mtg1Ñ®¿åoæiÕÀÑHl¢Àâ¬øTÜ„--mùIžu8ÿ?½Îzstöt´?7€Éå”Î‹f¡DÄŒ‘vTû¨Î€±5†•(’z´% À}µt&ßG{â—å“/Øã4Dm©wï ±à´wS¡!†Å”Î·p¦ãdJƒ,Xîºlý ú¾û|üŽ*Dˆõa­ÄõoQ-áÖº«áú*4—øÑT
ªK¶ÉábœhýK)gj’R´r5}Ÿ¸®RÝ}Áù	’²øhuÝ.q“Å‰Ä¯ç·³!³u®œMÉ|H×¥žÉ5¼ã…i’÷‚½ Fªé¼½³G²j€{ž°¼Ïˆæà!@\zš!×„N×½@þÙå¸=†uÃDNg³¥Œ*`¬ŒØöª¬x”’-;~{ÝÁ†ÎlMâç ùˆ1/DVÆâ]JÁ¦¿íò÷¿·ê—¢uô™ÉÓÜ,ã<ŠAÓk”Øñß0¸ðê2Ñ0$õ°ã}¥”±©çÿ°ò‘4ù¾ìú)šÌ¹Zë?–°wl.:Ó¼¨PÊÏÔYk±ò{ÿ¦ÁRÒ+ÝÆ‡Ïµ^¨M)	¾üE§7Úd”?óÁÚˆc ™BƒÚØb'<x‰õË–r¼q Bt,ôàEÎ–éÂ§pbu£e!Ìÿ%ØÁF-V7©(®“"çedraì”h=[ê)˜ªF_å€r8xÁi»ÍqºHi¿‡èf£»ÆÃþ…yØ‹FüéŽ¿û·T’æ\>ÄKZ%*¯ÌE±TuRæ×îqb¦ªJ+¯1ÑA#Édêœ.v«‚swþ$óM¿¶¸3xd}ñ½ûÛ8
¼xgì<Ê €¨[xÜsn–™˜â G@ˆèþÜA	‹@@/Pu–</°+ó]N$¬†•742¸>UK!ˆ]Ú;8X<ÕòÕ×žXÃ”×ÿtˆÜ’¼%F‘Â9b[ÀM“¦nÀuÍŸrÈÍÖ¼C))‰ÞClm|×Û²èz
ÌjÔE¤¼RÃÑºÄ?3ì¢Iù2£\ ×)ç²è8ÝG“øàÅ*L[¥¦ÒVVäOÓ­uÜÊ7ø[}Ÿšù°»xôQ‡Q°õñ/	ÂaÄáÛ4QÉ#F¤O˜!}Þ°ãŒuÝoA¤Ø”Ÿ•4Jìs`¦!Ã€•|=²XÀ];J{ñ¶™;¿›üï…¨êäˆzù×´a`î?nÉ¢¯n¨ðŽûø‘:Hþ¢H»™+Ê?Ì†°[ûY¯‰Pô
H(¡í]T²F¤©WÝÓøF„×B’>Ï…«Ÿ"Âƒ¦þ1è
,w¤X—Ð<Ü\|P­‰¾à#óìé g	jæ»À“‡k¹ÌtÙç­Ü^í[_­· UÓ<‚>ÇCï`ÜªÔ¶r„CRö6
íÑ­„"6ªäê[lð¨*ñ³7<–ð7
]cC4O8âŒ<WÐ…¥äuTqwL¦^.ûÍ· ËáŸìê{>`±ÄzäDõ‡mN-—Â<X·§Ÿ‚J¹qÉÃ#Æò–òNy‡]‚X:	S[g¬ -œ¢/•·«/Mþk°.Ñmm÷ôÏl‘‹ÿj>eKÎa°Ú“tñ*Î$Ñ‡eè2qÓ:ø„e~ßWËéS–M»ùßß.^E¨gÀ;>3þÖ6Û›©Ëcâð?Gâ:_€„‚ºOË²6!‹q/‰Ç¼ØÁë·…&1à:¹±ø£´8L»¿›mÄ ˜å¡[Ç…ˆØ;Bš€‹\5*I $§˜!/7;+]
•]mï„‘üÔhbŠjëùIo¤ébf'îe+3SØY·¹8÷Öªuf‰“F¶¢é¡ò_¡Ñ…kC®ßè#ÍÌZ§šh)X”ŽpC„¶€'âAPw›98Irrß¾Û¿VÃÉiÂDÕÝÓs>øâïÎÖH¯S¯iè£‡ Æ…g³ÙÆ<[ØÔ¬ ì¬*z>“iûœÃ¬pç½ÓWNÁ´æÓ›C«è¸YRÍ<œe˜ÒqûF­ Ô3ª Ceézq@J²S+
.ÁtÖÅì¯ðw õ	èh¡è‚ˆøy/>·»In—²Ç&§÷ZKðN}Rº\¤Š òûùeZ®&rGK—!ÙÕ2…yÒD‹6R“².Ü¶ääRoP2¥½ú‚qwiè§GêúÐâó¬¡êÂÄ<ùÀQ(Ñ€ºusl[Ýpë¨.£ÝÈë‰Œ|6ŽàL$H½ZôäÇÕ†[í‹Â]Ú»jÖM‹†5SH?ÈS’‘ïQ¡õf.ˆ8¾«'™tòÌâ›xYÙëÓÅ:%1¦Æ£aËHò#p"ó:žõÄúðo±µ‹Ùþ¶/eÔhÎäÒa•u9Ý˜¼Ø»ü… 80«1‰KûèÛÈ[
à°Ø¿8á¬ŠßÍ`4âƒmÀ~úGL‹Ü=ø«^vêà0Y 4Ûlï-3ì¬¥
¬KsÑY¿óBÜ»)F_œÂ^ÿAß¢¢[6‘1#¹OBgÜƒC%c  ;  ï<*°õ­û¬n–áeÙê­ ˜‘óÙhÍóŸÀúüŠ¢­³h4œa¼ÂðèÊçñÿ£rX2²0Ûïà|¼»W3 !2¯†åø\,Êåc£úôŸž(x ÷/Š?Ó…tÎ¸¹»½ú;	î2qØ+ãŒ&àïËÄ]|VpÙß®%¿)QdMÇ}ô2õ×ÚÏªq‰ÓÚ6ÊlOÎ†áê)p†­°Ã?ŒÎ˜òg†—ÿ"7X]²å<#ÝVÉCë&‡5«,éÿ¤áÍiP¡ê1G¦÷C¦Ã#´¥m@ÁË€ŸÑ¼Ñ‚Á»ÏnŸ^IÍ÷úÒ˜¸@ífz!¯9~‘{‰Â /Ëô]„v:œ0Î&?4J=u9ánšåöà_um¶æ4¸ÿSÁŠá}WøÔÜ{ù–.tÔmç½ƒsåR³ãjÜxÛŽ<£ãkûö8D–¼MVÑJ–D	Â‚¤›‡“J·A\ç°Ìq‘
«(gr×Œz\·“Rh‘K¶¶D	‹è/5’Ÿâ› ›1EÑÑÊÏÿa¢v¦dt#ûlLe“\FùÒ¨ÕH[v*•CD„ ý$QDnP	Ø õjæ‡Á˜)¥z°ºûƒ8Ñˆw‡3n>«—YTV‡_iXYå^Ž5çÑÚ¼Ýc-tûQ¬í&ø©ÂhÇ÷Çf½öyøÅÓÇkFìš[v„À‘ä8etp%t™¢ûm°TöV±”j‡åƒã\Ùÿ%õ‘[¹oä3'ÿ–Ý bŒ}JA pâ½ŽÓ§i£-sñK£KÍ"Y§j³eâÑPîÒNØìÜ×suÑƒ3¶Ó,/gØÌW]²ûù$ÆèzÚV,¡b(ÙÛ×2óÜÜU]Ù®§­¦êÒ{–ûÇ‹é¸±%µ|ÏÛÖssXÁ>\ŒËy)Ž¼3ÖÈž‘Ïq´ìv-6ùHÅZ³i³Q9žÅµ_/	çX%”ÁøEu‘ÝLn	í$ïZ¶9Àg?S.t×(žÏê-¹D¯©–x
7"`$S™+bk°×Ÿ—”–$Ç¢±œ½°„–´TËw£¢«“—)Ã¡§ÓŠ‚ÐL4+ýŒ<áxÿ“°û»ÞÜ?¸bÜ\µ¹áœŽdD0Œ¦:àJq
ÉÚ¿!ƒØ¿z%Z¨µD?s|Hò‚Á=,Í™nJr‡6²¿‚ÌÍ¤M3z]kŽù R6¥=àÄ[Íè´Éi×#n
%ùáÉ70 mvpoNim‚‰¾FÅÏÂðÁÐ®$³“=ù¯0•ÓÀ7ëO|rªÿB("X ÿ
¨ÿ-éMåªíZ”¼²Ôký¥AR¸¬‰–xÁAÐ_:ù_¯‘sË‡})kfë„Ü…îËãuð}¯—ßc¯Q›—v¢pò8]—cE)çß÷g$zÔ8Ø;((2q“ýN§˜XŠ«J­çüÊÍ„ú¤cMÜVS âtl¶’•Š¤2XBT„³/rÌ'=û×}~A±6¢"¢ÝO)qúìZv×¿Wkv_çúÑ*À´™ÁÓû¯sŒ~×öx	‰{jžY;Þv!	{ËÉä`<æ
$ØêÁ²ë§^¢ÒÆ’"êz[ßk¥ø ÖÍi¬öB9z=>dþ	ë¼·„W„Ørœsª&ãùáMòøv`Äº [YåªÐíU¿vO&wÊãW÷÷üâË‹Æ‡ÄØ#È—ÒãD×£ˆ‚nÎ#¥‚%ÁyÍ<p¯no ¼»?šá”T©Ì8L¨ÖB»Â¤eÙ…ìÝV2Ëu™Á[M’4º±SþÎLÍ`Yî]†ð¸;ÓÎ1¹6„k`h	€x:z®—Iå" ÕcøLüYôÍÖ:ÖÍ¦:]G³óþ†ÖÏÄÐL0³6næ9~;«´/Kµ—¡Á½HE8¯Â+‹ jƒdê1\¸½%gq´£)ëm—ü?ŽÛÜ°N†âìj?M@_»îëi—ÕlbñÏeß™R²¾Ù ¡ée¾±Æ*&Øƒµ? êØý(	œÂ{ŸÝUP¥N&qÅÅlqm ;›ð\ñ!UŒŽ°EwûÇÙ:ÌçCdÖ %Éñ+7Ë)û‰Y-Ïî†büíé®÷6Ø­lÄæÊ/4°§–õãO6'„ü¡”Æ'Ï–åÆ´¸}±ög.‡2eaèáë²êœSðÜ¹úV²Ê÷7ºò	‡Z–` ÁA;f2Xy÷_¢ÑwçÎhQ]ÄfD7@$[“Mªù3%’Uð=pãÔªSo“>x¢¦†îÙÿÉ&qåsÕ-ÂØr“F™0tªwr»}.ª(Gû9ÑÌÍ/*©‰Ç|d™Ñp?ùÁv9ù½ZÇµG/µ±ÖH§ë¯ÂƒQ#(fÙœ‚„´Ñ×ò+²ÓŠÙ¦ÇÓµÜ%±WZ`w;a²^lÆlF¥‚û,Q_hDš1ÊüØa‚#²)>È»…-2vC­z^ÒôÂl£ßD¬#l×ÿ3LÏ%¦”Iœ¯ºIe©úGÄcÖpŠ_½D®9QñÍ#Í£ð³¦S*»ÐêÓÈ©‰Lóþä•©¯àQ\º{¢—#ÙIÜØroe"vô½|º¶Õ8ª¢?BÈŸa ¡©IGÈ6§&7x‰Îht^G$Ú‹î@d,"ƒKßÎ4ldá]òHÂÏP<_:”úúxŸC9X*LÊ‚­…ïþjçô¦ÍäÖÖPj„¶²ççñ·˜'H¹£jèó²ð)&/Ä4>•aŒ3kÆ ¥7–±ÝRÃX9§³¨D‹•«à”¾0	:\>_{7·g?D2O»
ßh§ó¼N‚_¡“Öÿ;– ç•²³1Ì¡À	«÷Í‡ Šx"|¯Lk‡è½ÆÅÐ³èR>µ²_KR9ïþCyÊ"ü:ç—üÂÍ£Ñ`²æiä©Ò÷fxâ$À/¡ŒþVÿþ·}8äž¹5ü9¿¸ƒ¿Ž/žß7K×åUÜãŒðÞÎŒ[JûT 8)x/8’³½; øTiß•1Lâ_vû“º® ŸÕ6øw(	¯à¨“îT×Æ7Îqn¦íß7AHlãqýå÷½ý~kê­Tû=Œ£Ä}f#-Ù?Ìë×îÙé€ùÞˆ&
€,5å±?V Žfô9){Ó=Ù¿áM¶l¹£ˆ Ó"v–§l•iyÐ,¿§ž15®9¯dº°'±Ôá…lÆ‡nŠHU¬+É«Ø}×õRÕ:[»¢|>ˆyÿ6º²F c5ÕÕiåfÔ¬çñ”@RÛ\9ä2Å—m‘Ç±CøØ¢ŠL&Ó“9Dz³þø¡Þôø;°kædËc×Ü†ˆûNƒæ
kE08NKÆìóÂ,…™<ö—¼µF$SYß-ñÒÉ&þ<çt‚ûŠ—Ë:¥ðä
‘ñ¡T›Ö|¶˜·¨*ª[x‹”[Ë?ú€ÔõÛš QÅ`-<Òu-„nÙ&äb§SÂk>sÇ~'Ê¥µ]YÑÅÎ˜e¦çÐXUÏöé§jlÕÏ¹gÖë*5B¤há`4Üæÿg–hTèCOrüD·zouí¹z±uåK+æ GH*,K=z(–±­•y	n"`æGh8ÏÞ·¦}‹Ô^þŠ¡8dçÝ‰ž…ýã¾âµ£ÿæ?¾aŠ_ðêˆ¾7ÛƒÊgU;…|ÏZMãÌ§
È;½4àÖÂÛ fƒóN£2×@îžä˜ØÈÙfæx"L:9ÉåRGoëüz ‰s>ÝŽ0É&ÃøábD@Æø”á&FC#ÎÖýxªëKØ@"‘•¸úŠn¬QM"­í£ÌÛÓÁ"-‹[ÛÃâ­¿Ø%ø¾ŠÒX)/ô)&@… Î°#G¨·Ô¯¸ÐÀÛýøÀx^Ô‚…d]Às%ŠØB ÏŸGÐW'#=j¥ÌJÇã|]Ý'±?™O~"åó÷Û*á³P—wLUâ|®ìÃ+ÛÀÞžåØ­ßŽ«]ÛzÈG•“ÎòA»®L9ˆ|A‚Ô+Ðæ3Å @¥»uÑ±Ãò$Ñ6AI%‹¦g˜
s:#l¥'ÂfŸÕÿÎbÍ7OƒkHÃ‹“fË¤é	åqê~.¢c-âfnÓŒk_´¼þÂ÷¡‘7P0=Ä,&3Ï€±\ÖSg„½–¾;(»×¾NÂPÚ^ûà[®Óõ¶úãº%m½‡‰¨™qÑˆär—k	Ÿêv*¦ªbxp¡=™QðS¥g®–*Lfå º0X4ikˆM0Õä«–y¤Ñ¶ª”'ô,VÕ=»«Æé÷8Hã–
^r*Zd¥I¶`ÀÝ®‚#ÁÝ•XQlè&òµ6	ƒ˜“]þ•bo@2Në‡Jæ£B.›êUUZå ô8C×ÑŠ,Èç°Í´*—ÂnôøwøDøh'•"”ämŒœ¤™ƒëu¤‹4é¬ü:NŽ´ƒ7,¢)<Wa­2WªaþÎ@£kyÿ¥+ŠÀøÌj¸ ›Ú.hW³Ü’KRµïµrÆqÝ?àÉèÒJªíÐ¿ÿåPLºc©jÃö›kÃ=?þ
{šâHùoáö>]ßÃ{¾#°s°Aë²´ T.4Î4Ä‘ñæh…<¬dkŒøÍEAd¿€Æö-€¢uËƒ‰‹Ñ šˆ6}q8õI-T¹Ø¨„qž„Q	“\Ó¼V<Ö”»¡dËù}øî{È3™O'0UÙB~îI\wÁÇ\jŠ•GDë¶“ˆzêù‰Ã³i(ÿèº¶f"V:Žd]ijUIœÄg3dZÅî~/žvÙ¶ŸÀÀÜ°=$ L.P¿5®µ†Í'Æ1xÊ>ÍZ«¢i3j÷^å¬`ÓE>¡,9DÍã{ËŽöèûH?ºâÐ¥äU/›¯hò/žŽCæíÇeCJïþþžFÃî,7wA![9µ=vÓoò~ï§:ZQÄ ~¸n„Œ…¼¦ÇÀ{vtŸûgÝàcc%
7‰”‰úÙÛÒcÃÞÇÃj5ˆ?ŸqÜÆ¹õ¯æøíç€OŸ2„¾-Ú·—O´µc³åúÌ´nÜ·ÇHøïßÆó¡ÿE©¥_™… ¯†M¤^Wz‡ˆaì\)}¿r?¯áZ¡¾Ë(Lñî€ö(GÊæ½Ì¡*aØg¡pKé¯A68chI~ë•Bv%÷q8×ÕQ³‡‹X I‰I`îJ”õ‰”²p£¨s!|„µã’-›Üc4"ïÞ¼Óú£}‚…ÃÑ;ÃÇŽÆcö³8
¦5MñÊÕœ¹#ÆÝÇýHã™ë–Í$x 1¹Š°¬½©”*J¬Þä»™l‰Ûû;'$"É¹_:VÍåKwÌ\½îá×Ö
Ã)	–¨Gp×Xþ+µ| ÁEý\ÁXþÞê¦li·4š2%°€}Øï‚ìýø       <*°õ­û¬ã¤A¥ÛL"¨Ö˜´øhÅˆêô®ƒÆ¥ƒgô¶ÿ%À÷þ¯Ã}¿=?¤ä¨tæ·0^e¨¥:½£HêÆZ$èê¹«îúŒ—€tFød‚IÖMõ°³ñòSX4ÇÞå¥ŠæYÌ
ZÈ{˜w±Þ©ë¸çVªJ	z:5;ÐÈÜ­¿ŽÝøÍ¢H /íçwHª~Äñ‹ Ÿ<†©H1%ù_“µ×âÍ;íÚ˜ÎìØÁ!I2e+'øjæÁnž¦$6‰¡9D×¡$z¢£GÌNŠQÖrÖLÆuÈ ”"B±ü†Ùä³<‘m]¤Ehuqšõ¾+S·ÿ!
1à;²-CHAA~Eê‘™tŠë#~½šH³ƒXb½v+ó¨×òê%ßìá³!}G\íÈÒÚÐD7iè¡ð<3ŽvFœÚ€Ž‰Ð¯QM˜µ€¼‹W-tÄ»z[À âl¸ÜÉFq–¼ÕtY˜c[@|½ŽÃ€"$ÿ™U(êûNÚ ÄÝô«©¼­ßo¾(1g†nYWÐŒòKÙÇ£C‘¼!_HŽ
N+7/›O¤Z[ÃÓÆ+Óþ íMÊR"o´¿ÿ†…=Õs‚7k:®“\PSƒZm¤\çáàŠ‹ãoÞcg“pEU?é˜F­|lyóybòÇü{×yoøè$_È€~“l5frrþÊRWÒa@WÈ[²’hbZuQüP€S‚ø’]»ÑÉ²ogÞo{ z¤øìŠž_Îèªj¥Ÿ„P~Øhð qÀ!_¥’¼J-¨»öŒ¤ãÑHÚŸÎü‹ø¨0ŽÞ/)ep†%lRÈôª'ñå|ØQ$£È(×ÚÕ<æ¬ Ñ,T]ÛL§£]þm0q_Zû8ŽS»³:I§TcYö;æŽ?¯ÍîhªW‚(Œ¨”usR¹eŒ™”X!â[†l	SáW•/4†}º$ Ä5	27ƒÄñ¹¢µÞ!9X
:5ÑÌÅlŽuõ+*Ï\ÁzK”ÝÛ†y3-,ò~œ®ùWª	Ö¿š—&ÞÒ[—zh–BWw/ô=åMÚükÒ­ÓyN¼ 8Š0dþ1ž³ý¹c9±†œà´åäŽj#²R°W2®:ÊÜ½þ²ßbåÕbË¨DG‹s:ÈB¬ÊŒk¯Ì®cIt…‡1t'=µ(¤ñN‡P6†5Ò<¥û>TÄG1Ï‘ô×GÔñ"T&ÿã$0[{{¿pƒCjk€È£G“àÜ¹Ñ„!e‘;ÅKûÕÉŠpüÀÝïk|&PÜNú‚Ô”Nÿã¥ü+ëbËËP»žüÔƒÙ¥y8 õ4‰ä¥HA7[ù.gý@`>ó)bÏ˜-4­ï6@®<šÜe­á:¤ú‡U&p5^‘a n
'	ÈÕ>ÜC(n`¤ß£ñ‡u‹7¦L¯o½n’g»´LØoV4°põ~•'Ä¨ŽÜ<¯´	EõX*>ýÕ*W—ð†¹õ¢	(¯ÞI+}ò`ü¡QÊjq[sôø›–s"µANûs~|,TÉßÇxXÒz+šç;Â7Aü×’
óa|*
*ºiâ9ÜÚ¨_ßÅ” omLm#®ýÖÖ1«ucÜÑ?nûßëRDºÀ`ýBs«v+àÿã*´Ã±Ê¬Zvè©ÖïµóZai­Åãƒò!ü&ëánô—9mÈË’ãòZH±ÆÇí¹*¤²(ÂÏíáÄ¸ÂW¹m?KäÒ”Î¾O4„×Z•Í©fß‡®dS¸vUŽ^›’J§3ç•
õ0ZYÂë‹ö…Ûø¡¦C2c„¦e¾k–(f%6ÞrI)'ÖÕ7lq"TÍ? Íà%¥Ü	>{›®‡çÂÇÖN‚Å°2àÊ¯t;Ù¦·)IÂu¹K/
0gÇý3‰»¿'Ã Ù~0e½k%{Z=iaºXˆ–Ä:XÛÒØô¶åœ´aM…R¸èé	ãI‡±ßÓ‘î*$ 4t³Q¿®Ë) Úæ£cl¤@Ô	bŽyÓj	=âLP)uq|Ã§lI8Ì»Ÿd¥ï!©Šì°ozYúŠè`°ød-¦GÉí…½Ëhôù„Ï€ üãÈz¢ëEïxëÉ)6-Õt*oE6ßŒ°Y5~ÎhTž´û¾Wí|Ê@B¿®açg²ö~aXÙ»”?ú4¼ðÿ*!ˆD75L8`0©j¯Ñj?ÄwÆ$Pï6òYSîÃ³MXô0#¨ë'÷fVö>Ô[^l•<Z¥8)Þ·QmÁjpm 40à8m¤©‘|‚ÓËé¦®#ÑªÆ@ãÇqSæÕ2‰*{|³ dFŸ¦#‡'½¶À­G3ñqüÅìÔ¯ƒñV;(h6‚VNJ¹<×&±ÕˆÕÓàE¥¯CÜÌ¶óZhD¸®\UÖ»l.k¥¨–z ÒÃ›ÊßKaž`CÙRóÀs-Ôq­tjq3ñìk¶(DI.oÑý)—oüÆüApN%œ…¾ÙúÈuÕmc5wºŠCTùÐ¨1;#/®óµ¤m¥˜ÑŸÆ©hOõüèœâ®ÙxöQìTö{
ýNK~Èkeý~¸½æyhéÌ–Š¥ÝB—u õËƒ_ÿQâ-ä&êUÎfk&JÇœdð/€M¦05.HÇD_VŸ 5—*9‚"XÖgUg$¯šÄ,¦éÕÐRí‰U“ÿlr˜ÿK¿¡8ãemojÙ)ñ"Ç2<ÐQÕ¼&æ"7Ð› ·ßmU<ºŽ4Õ9¨‘ˆ“©FÜœtóÔ¤Ü_¯:´–e¦lÍA^ÿ¤htðºLõËÜuX»KP 5˜±µ3Ë@–•[‡Æ­Œz[™ò0š]‡ÆÝ°îPm/ëR¶»aï1NzÌ à»Mtg1Ñ®¿åoæiÕÀÑHl¢Àâ¬øTÜ„--mùIžu8ÿ?½Îzstöt´?7€Éå”Î‹f¡DÄŒ‘vTû¨Î€±5†•(’z´% À}µt&ßG{â—å“/Øã4Dm©wï ±à´wS¡!†Å”Î·p¦ãdJƒ,Xîºlý ú¾û|üŽ*Dˆõa­ÄõoQ-áÖº«áú*4—øÑT
ªK¶ÉábœhýK)gj’R´r5}Ÿ¸®RÝ}Áù	’²øhuÝ.q“Å‰Ä¯ç·³!³u®œMÉ|H×¥žÉ5¼ã…i’÷‚½ Fªé¼½³G²j€{ž°¼Ïˆæà!@\zš!×„N×½@þÙå¸=†uÃDNg³¥Œ*`¬ŒØöª¬x”’-;~{ÝÁ†ÎlMâç ùˆ1/DVÆâ]JÁ¦¿íò÷¿·ê—¢uô™ÉÓÜ,ã<ŠAÓk”Øñß0¸ðê2Ñ0$õ°ã}¥”±©çÿ°ò‘4ù¾ìú)šÌ¹Zë?–°wl.:Ó¼¨PÊÏÔYk±ò{ÿ¦ÁRÒ+ÝÆ‡Ïµ^¨M)	¾üE§7Úd”?óÁÚˆc ™BƒÚØb'<x‰õË–r¼q Bt,ôàEÎ–éÂ§pbu£e!Ìÿ%ØÁF-V7©(®“"çedraì”h=[ê)˜ªF_å€r8xÁi»ÍqºHi¿‡èf£»ÆÃþ…yØ‹FüéŽ¿û·T’æ\>ÄKZ%*¯ÌE±TuRæ×îqb¦ªJ+¯1ÑA#Édêœ.v«‚swþ$óM¿¶¸3xd}ñ½ûÛ8
¼xgì<Ê €¨[xÜsn–™˜â G@ˆèþÜA	‹@@/Pu–</°+ó]N$¬†•742¸>UK!ˆ]Ú;8X<ÕòÕ×žXÃ”×ÿtˆÜ’¼%F‘Â9b[ÀM“¦nÀuÍŸrÈÍÖ¼C))‰ÞClm|×Û²èz
ÌjÔE¤¼RÃÑºÄ?3ì¢Iù2£\ ×)ç²è8ÝG“øàÅ*L[¥¦ÒVVäOÓ­uÜÊ7ø[}Ÿšù°»xôQ‡Q°õñ/	ÂaÄáÛ4QÉ#F¤O˜!}Þ°ãŒuÝoA¤Ø”Ÿ•4Jìs`¦!Ã€•|=²XÀ];J{ñ¶™;¿›üï…¨êäˆzù×´a`î?nÉ¢¯n¨ðŽûø‘:Hþ¢H»™+Ê?Ì†°[ûY¯‰Pô
H(¡í]T²F¤©WÝÓøF„×B’>Ï…«Ÿ"Âƒ¦þ1è
,w¤X—Ð<Ü\|P­‰¾à#óìé g	jæ»À“‡k¹ÌtÙç­Ü^í[_­· UÓ<‚>ÇCï`ÜªÔ¶r„CRö6
íÑ­„"6ªäê[lð¨*ñ³7<–ð7
]cC4O8âŒ<WÐ…¥äuTqwL¦^.ûÍ· ËáŸìê{>`±ÄzäDõ‡mN-—Â<X·§Ÿ‚J¹qÉÃ#Æò–òNy‡]‚X:	S[g¬ -œ¢/•·«/Mþk°.Ñmm÷ôÏl‘‹ÿj>eKÎa°Ú“tñ*Î$Ñ‡eè2qÓ:ø„e~ßWËéS–M»ùßß.^E¨gÀ;>3þÖ6Û›©Ëcâð?Gâ:_€„‚ºOË²6!‹q/‰Ç¼ØÁë·…&1à:¹±ø£´8L»¿›mÄ ˜å¡[Ç…ˆØ;Bš€‹\5*I $§˜!/7;+]
•]mï„‘üÔhbŠjëùIo¤ébf'îe+3SØY·¹8÷Öªuf‰“F¶¢é¡ò_¡Ñ…kC®ßè#ÍÌZ§šh)X”ŽpC„¶€'âAPw›98Irrß¾Û¿VÃÉiÂDÕÝÓs>øâïÎÖH¯S¯iè£‡ Æ…g³ÙÆ<[ØÔ¬ ì¬*z>“iûœÃ¬pç½ÓWNÁ´æÓ›C«è¸YRÍ<œe˜ÒqûF­ Ô3ª Ceézq@J²S+
.ÁtÖÅì¯ðw õ	èh¡è‚ˆøy/>·»In—²Ç&§÷ZKðN}Rº\¤Š òûùeZ®&rGK—!ÙÕ2…yÒD‹6R“².Ü¶ääRoP2¥½ú‚qwiè§GêúÐâó¬¡êÂÄ<ùÀQ(Ñ€ºusl[Ýpë¨.£ÝÈë‰Œ|6ŽàL$H½ZôäÇÕ†[í‹Â]Ú»jÖM‹†5SH?ÈS’‘ïQ¡õf.ˆ8¾«'™tòÌâ›xYÙëÓÅ:%1¦Æ£aËHò#p"ó:žõÄúðo±µ‹Ùþ¶/eÔhÎäÒa•u9Ý˜¼Ø»ü… 80«1‰KûèÛÈ[
à°Ø¿8á¬ŠßÍ`4âƒmÀ~úGL‹Ü=ø«^vêà0Y 4Ûlï-3ì¬¥
¬KsÑY¿óBÜ»)F_œÂ^ÿAß¢¢[6‘1#¹OBgÜƒC%c  ;  ï<*°õ­û¬Ã¾ë:úÀ­ ˜ðÔhÍóŸÀŒðüŠ¢­³h4œa¼ÂðèÊçñÿ£rX2²0Ûïà|¼»W3 !2¯†åø\,Êåc£úôŸž(x ÷/Š?Ó…tÎ¸¹»½ú;	î2qØ+ãŒ&àïËÄ]|VpÙß®%¿)QdMÇ}ô2õ×ÚÏªq‰ÓÚ6ÊlOÎ†áê)p†­°Ã?ŒÎ˜òg†—ÿ"7X]²å<#ÝVÉCë&‡5«,éÿ¤áÍiP¡ê1G¦÷C¦Ã#´¥m@ÁË€ŸÑ¼Ñ‚Á»ÏnŸ^IÍ÷úÒ˜¸@ífz!¯9~‘{‰Â /Ëô]„v:œ0Î&?4J=u9ánšåöà_um¶æ4¸ÿSÁŠá}WøÔÜ{ù–.tÔmç½ƒsåR³ãjÜxÛŽ<£ãkûö8D–¼MVÑJ–D	Â‚¤›‡“J·A\ç°Ìq‘
«(gr×Œz\·“Rh‘K¶¶D	‹è/5’Ÿâ› ›1EÑÑÊÏÿa¢v¦dt#ûlLe“\FùÒ¨ÕH[v*•CD„ ý$QDnP	Ø õjæ‡Á˜)¥z°ºûƒ8Ñˆw‡3n>«—YTV‡_iXYå^Ž5çÑÚ¼Ýc-tûQ¬í&ø©ÂhÇ÷Çf½öyøÅÓÇkFìš[v„À‘ä8etp%t™¢ûm°TöV±”j‡åƒã\Ùÿ%õ‘[¹oä3'ÿ–Ý bŒ}JA pâ½ŽÓ§i£-sñK£KÍ"Y§j³eâÑPîÒNØìÜ×suÑƒ3¶Ó,/gØÌW]²ûù$ÆèzÚV,¡b(ÙÛ×2óÜÜU]Ù®§­¦êÒ{–ûÇ‹é¸±%µ|ÏÛÖssXÁ>\ŒËy)Ž¼3ÖÈž‘Ïq´ìv-6ùHÅZ³i³Q9žÅµ_/	çX%”ÁøEu‘ÝLn	í$ïZ¶9Àg?S.t×(žÏê-¹D¯©–x
7"`$S™+bk°×Ÿ—”–$Ç¢±œ½°„–´TËw£¢«“—)Ã¡§ÓŠ‚ÐL4+ýŒ<áxÿ“°û»ÞÜ?¸bÜ\µ¹áœŽdD0Œ¦:àJq
ÉÚ¿!ƒØ¿z%Z¨µD?s|Hò‚Á=,Í™nJr‡6²¿‚ÌÍ¤M3z]kŽù R6¥=àÄ[Íè´Éi×#n
%ùáÉ70 mvpoNim‚‰¾FÅÏÂðÁÐ®$³“=ù¯0•ÓÀ7ëO|rªÿB("X ÿ
¨ÿ-éMåªíZ”¼²Ôký¥AR¸¬‰–xÁAÐ_:ù_¯‘sË‡})kfë„Ü…îËãuð}¯—ßc¯Q›—v¢pò8]—cE)çß÷g$zÔ8Ø;((2q“ýN§˜XŠ«J­çüÊÍ„ú¤cMÜVS âtl¶’•Š¤2XBT„³/rÌ'=û×}~A±6¢"¢ÝO)qúìZv×¿Wkv_çúÑ*À´™ÁÓû¯sŒ~×öx	‰{jžY;Þv!	{ËÉä`<æ
$ØêÁ²ë§^¢ÒÆ’"êz[ßk¥ø ÖÍi¬öB9z=>dþ	ë¼·„W„Ørœsª&ãùáMòøv`Äº [YåªÐíU¿vO&wÊãW÷÷üâË‹Æ‡ÄØ#È—ÒãD×£ˆ‚nÎ#¥‚%ÁyÍ<p¯no ¼»?šá”T©Ì8L¨ÖB»Â¤eÙ…ìÝV2Ëu™Á[M’4º±SþÎLÍ`Yî]†ð¸;ÓÎ1¹6„k`h	€x:z®—Iå" ÕcøLüYôÍÖ:ÖÍ¦:]G³óþ†ÖÏÄÐL0³6næ9~;«´/Kµ—¡Á½HE8¯Â+‹ jƒdê1\¸½%gq´£)ëm—ü?ŽÛÜ°N†âìj?M@_»îëi—ÕlbñÏeß™R²¾Ù ¡ée¾±Æ*&Øƒµ? êØý(	œÂ{ŸÝUP¥N&qÅÅlqm ;›ð\ñ!UŒŽ°EwûÇÙ:ÌçCdÖ %Éñ+7Ë)û‰Y-Ïî†büíé®÷6Ø­lÄæÊ/4°§–õãO6'„ü¡”Æ'Ï–åÆ´¸}±ög.‡2eaèáë²êœSðÜ¹úV²Ê÷7ºò	‡Z–` ÁA;f2Xy÷_¢ÑwçÎhQ]ÄfD7@$[“Mªù3%’Uð=pãÔªSo“>x¢¦†îÙÿÉ&qåsÕ-ÂØr“F™0tªwr»}.ª(Gû9ÑÌÍ/*©‰Ç|d™Ñp?ùÁv9ù½ZÇµG/µ±ÖH§ë¯ÂƒQ#(fÙœ‚„´Ñ×ò+²ÓŠÙ¦ÇÓµÜ%±WZ`w;a²^lÆlF¥‚û,Q_hDš1ÊüØa‚#²)>È»…-2vC­z^ÒôÂl£ßD¬#l×ÿ3LÏ%¦”Iœ¯ºIe©úGÄcÖpŠ_½D®9QñÍ#Í£ð³¦S*»ÐêÓÈ©‰Lóþä•©¯àQ\º{¢—#ÙIÜØroe"vô½|º¶Õ8ª¢?BÈŸa ¡©IGÈ6§&7x‰Îht^G$Ú‹î@d,"ƒKßÎ4ldá]òHÂÏP<_:”úúxŸC9X*LÊ‚­…ïþjçô¦ÍäÖÖPj„¶²ççñ·˜'H¹£jèó²ð)&/Ä4>•aŒ3kÆ ¥7–±ÝRÃX9§³¨D‹•«à”¾0	:\>_{7·g?D2O»
ßh§ó¼N‚_¡“Öÿ;– ç•²³1Ì¡À	«÷Í‡ Šx"|¯Lk‡è½ÆÅÐ³èR>µ²_KR9ïþCyÊ"ü:ç—üÂÍ£Ñ`²æiä©Ò÷fxâ$À/¡ŒþVÿþ·}8äž¹5ü9¿¸ƒ¿Ž/žß7K×åUÜãŒðÞÎŒ[JûT 8)x/8’³½; øTiß•1Lâ_vû“º® ŸÕ6øw(	¯à¨“îT×Æ7Îqn¦íß7AHlãqýå÷½ý~kê­Tû=Œ£Ä}f#-Ù?Ìë×îÙé€ùÞˆ&
€,5å±?V Žfô9){Ó=Ù¿áM¶l¹£ˆ Ó"v–§l•iyÐ,¿§ž15®9¯dº°'±Ôá…lÆ‡nŠHU¬+É«Ø}×õRÕ:[»¢|>ˆyÿ6º²F c5ÕÕiåfÔ¬çñ”@RÛ\9ä2Å—m‘Ç±CøØ¢ŠL&Ó“9Dz³þø¡Þôø;°kædËc×Ü†ˆûNƒæ
kE08NKÆìóÂ,…™<ö—¼µF$SYß-ñÒÉ&þ<çt‚ûŠ—Ë:¥ðä
‘ñ¡T›Ö|¶˜·¨*ª[x‹”[Ë?ú€ÔõÛš QÅ`-<Òu-„nÙ&äb§SÂk>sÇ~'Ê¥µ]YÑÅÎ˜e¦çÐXUÏöé§jlÕÏ¹gÖë*5B¤há`4Üæÿg–hTèCOrüD·zouí¹z±uåK+æ GH*,K=z(–±­•y	n"`æGh8ÏÞ·¦}‹Ô^þŠ¡8dçÝ‰ž…ýã¾âµ£ÿæ?¾aŠ_ðêˆ¾7ÛƒÊgU;…|ÏZMãÌ§
È;½4àÖÂÛ fƒóN£2×@îžä˜ØÈÙfæx"L:9ÉåRGoëüz ‰s>ÝŽ0É&ÃøábD@Æø”á&FC#ÎÖýxªëKØ@"‘•¸úŠn¬QM"­í£ÌÛÓÁ"-‹[ÛÃâ­¿Ø%ø¾ŠÒX)/ô)&@… Î°#G¨·Ô¯¸ÐÀÛýøÀx^Ô‚…d]Às%ŠØB ÏŸGÐW'#=j¥ÌJÇã|]Ý'±?™O~"åó÷Û*á³P—wLUâ|®ìÃ+ÛÀÞžåØ­ßŽ«]ÛzÈG•“ÎòA»®L9ˆ|A‚Ô+Ðæ3Å @¥»uÑ±Ãò$Ñ6AI%‹¦g˜
s:#l¥'ÂfŸÕÿÎbÍ7OƒkHÃ‹“fË¤é	åqê~.¢c-âfnÓŒk_´¼þÂ÷¡‘7P0=Ä,&3Ï€±\ÖSg„½–¾;(»×¾NÂPÚ^ûà[®Óõ¶úãº%m½‡‰¨™qÑˆär—k	Ÿêv*¦ªbxp¡=™QðS¥g®–*Lfå º0X4ikˆM0Õä«–y¤Ñ¶ª”'ô,VÕ=»«Æé÷8Hã–
^r*Zd¥I¶`ÀÝ®‚#ÁÝ•XQlè&òµ6	ƒ˜“]þ•bo@2Në‡Jæ£B.›êUUZå ô8C×ÑŠ,Èç°Í´*—ÂnôøwøDøh'•£íe5œ¢Ÿj~¬ƒµ¾€Ð2b	¹`§õg(è1á
°¶‰=ºu¸~#Æ7Dfq›ÌXá)¯”¹õ‹\Ìù´Jv$Cà¥á?7ØÜ‡.¤¾T‡e¦¼gËž»÷–$¾)ÂvH€ÌGÌ"yp\I¢yé–-”\Óª\<¼¨ÌLÅÔŠÁßh;]@àQ¼ôÓÆEÖYsÚÂ™  - Ï®D¶†uíwrÃûx±ý`é·Y­ã +U6¼Åþ¯þoa'þ?}Ó¼V<Ö”»¡dËù}øî{È3™O'0UÙB~îI\wÁÇ\jŠ•GDë¶“ˆzêù‰Ã³i(ÿèº¶f"V:Žd]ijUIœÄg3dZÅî~/žvÙ¶ŸÀÀÜ°=$ L.P¿5®µ†Í'Æ1xÊ>ÍZ«¢i3j÷^å¬`ÓE>¡,9DÍã{ËŽöèûH?ºâÐ¥äU/›¯hò/žŽCæíÇeCJïþþžFÃî,7wA![9µ=vÓoò~ï§:ZQÄ ~¸n„Œ…¼¦ÇÀ{vtŸûgÝàcc%
7‰”‰úÙÛÒcÃÞÇÃj5ˆ?ŸqÜÆ¹õ¯æøíç€OŸ2„¾-Ú·—O´µc³åúÌ´nÜ·ÇHøïßÆó¡ÿE©¥_™… ¯†M¤^Wz‡ˆaì\)}¿r?¯áZ¡¾Ë(Lñî€ö(GÊæ½Ì¡*aØg¡pKé¯A68chI~ë•Bv%÷q8×ÕQ³‡‹X I‰I`îJ”õ‰”²p£¨s!|„µã’-›Üc4"ïÞ¼Óú£}‚…ÃÑ;ÃÇŽÆcö³8
¦5MñÊÕœ¹#ÆÝÇýHã™ë–Í$x 1¹Š°¬½©”*J¬Þä»™l‰Ûû;'$"É¹_:VÍåKwÌ\½îá×Ö
Ã)	–¨Gp×Xþ+µ| ÁEý\ÁXþÞê¦li·4š2%°€}Øï‚ìýø       <*°õ­û¬=‹#ôoÚ[üLñç˜þ­‹ü¦õÚÏ  @     P“ýß‡Âùóúý™Š¢­‡h4˜a¼ÂðèÊçñÿ£rX2²0Ûïà|¼»W3 !2¯†åø\,Êåc£úôŸ–…xè÷ª¦Ö…tÎº¶M½ú;	Æ=Š×Ýï2‡QêÂiT,tëxß°«Í»ÃU:O¸û3ËÖìÏªq‰ÓÚ6ÊlOÎ†áê)p†­°Ã?ŒÎ˜òg†—ÿ"7X]²å<#ÝVÉCë&‡5«,éÿ¤áÍiP¡ê1G¦÷C¦Ã#´¥m@ÁË€ŸÑ¼Ñ‚Á»ÏnŸ^IÍ÷úÒ˜¸@ífz!¯9~‘{‰Â /Ëô]„v:œ0Î&?4J=u9ánšåöà_um¶æ4¸ÿSÁŠá}WøÜÜ{ù–¯:ÆjüÒš‹ñÔ&ÁŠ»©Ú{ñ¼)–¦J+õÙ>%˜$ð+V†çvÁïÏúmÂ-9”=cà¾ò|yØa	¸@EÉ;ò?+Á^/Ô–âTVÉ…Gýü`‘èiõW*Ž•¯£r‹êÖf6Ožl$Õ_Wf½ÄžHühUVhø6nçEŽW*{°LŸEn9Õ.£Ë„Ìl…<âõ¶£ÃU™áôaRÎÞ729§S,Hyµ,áV‚¢©õv»dÛlŒQÍ©W¼’â-‰³©wºá@ßÄRÎ(ˆÿ#?À˜Î¦U5FêÑN•ß¦øÐ(êµñŒ?¼tŒÈK“þJú=¡VgbßßE™e:¬4FJ}pÞë Ô Íû}r#CNQÑíkO	Õ`	Ö‘^¿6àµ¾‚œ¤½¤\Œ“4¡ô.á¯¸6?ÞžW”t´‡n©%eÏG›¶‡w]}¯¯3¿Á·îGãÜ¾—[J×¹‹ÎÉúÜuÇ¬¾¥ :6§QtÅY`Àèv‘T­ÎÃ†<õ¾/}¼/‹çIýuÒé•@£Dç©¯B¶0×FÃý!]Íjºú!^²\6]„\ÿ½žyÐ!MçÂ=MXe#l/ps×~.'œ÷ÏÞÐ¶m‰öôÛFïÊÙàt…"ïî‡³Ä]±ÔÄ§Üç¢%[EÝÄÂh¤?ºE¿²Ö¿»yÑ¹ÔÔ„¼À2bÏî{²fQ[`d ®ÐhHí¿ùD=ÛêS;ÒEÌ•xkTŸ¹ &§xçIó®ì€àÊw9\g3È•Á5EWóxu¶?¨†àø¹÷Ym:Ob¼³%‡xd Z8:<Cn$ìàÊ)·¦¬—‡¼ÏCÀÌxÊs@á…ðS¡E*ÎN†2MQxi±^í¸h»m«å¹zÚéþ˜G1¾Ê,?ÙÂíKÿ¤ ¢8IÙJ	@ýÒ;ŠÕQ	#›}Í²ï‹¨—Ÿæc·/à£=ÝmûÇµM<åMÇ»(fVô~´ZO[8Ý©àÝ
¦+çh+Þ”®¯¬à·Á¸#UQ‰ƒë{k¡ß.”m\<',Í÷w-Uà„NMˆ…bßPÍ}XdÍ¾*Z³‚<žÛ>6—f¨¤üO‰Úÿ®Ü¸ý6Í\;÷B¸<LD©M?2ÁV–tR[§¬­Z|¹ZVv»²Á¢tÁ1ë¶æÝlÊ8q¶xAaÉI¸«rAü„-ZNM-o„õÓ­ÖÿÖu‹Zƒ~¥Ò€/ž4ŒÓÐ(Æµ¤;Ùt4"nº‚›™«p­äÈÄ–bœ]·†¢›æ<Êï&§SÖÐP­„RÀF&dœòqÎ¤CÑ‰œjå—â-‰á<ùË£‰v|ž9Pµ5‘)"ñQÉÂ¨#„yE 	HÁµêe™xÑ',D%[eì{`Çø'ÅkN&¿XÜJ ?ƒƒvšá†h(+cúcÓ¼Ê™èðCòCe’7uDîój•Ùî•tP†n\XØÇô8Z¯D£BÖÕ@ÀÆMË^#VÒ»zÜû’ÿ¦@· &	ú»§=·åEãõÂbÈ*¬S›+Û~Ú¼Xè­=áó«lO´ææKÁ˜¬ˆXHÿl«ñ®=ã'|juà·±,COòŸ2lübÉÏä W²‰”Ç
 Žÿg¤·B[®zè+YºžÇˆ„†À„—ãL†ýÇ¦JgaÑÕâ€a¢,BNë’u‰ÒôªBŸ÷‘®ümËe1þ¿ú`9²d^œ€‰ÞÞ>¶µÕŸÆ}¸ƒBÊ³jóùc4å"MH¨-^5F`*‚/ã²xˆ ]–#cÒæ¼gÿR<ñ•n‚q ß#.ðJÍÈõÆG"Ô±c6 !õ}‘?ZÁÄfÒuKTäVOW<î1b†’{\H°¸{…NYÁ©Ž20CÞ”"·”:uÕ®Ù"ÔÅ¾hé½îÀRq(ŒÐÎ¨7õ²û¾Eô¿ë¾ã†6‰O–ò™w‘TYLWk}Ñ;µ?2Äðo>*0ºx„¨R$Ðü|Ãrä›ÃI@0/Ì-ò½Œ8XäšŒm9›³l@«zqÎvõõ=ÓùÌÕ;àlÚŠ7“7MØóHâq´»B¡Ö‘ÇÉ!~uÉµ‹§šÌêp>—·€µ[çAêZ¥,ï7î»;©«
k‡ÉÖÚ°ulÂÐZ#¼ÍCÎÛ-¬RéDcp=Ûî&!úÇ¨0DPæ*a«W…–h‹[{h´´¯4Ój¸b!K£¥öŠŽ“Á¨–‚‘5ÆÛôŽ‹”þöAtýÆkv²Ñk™FH\†Yxüéz oæeC×Wôý‘~à[H“üMÔ×Æ¦ýÒU@T:Q ?RÛõZQ;Òwd¬HæY§ù¢äß“«~¶ï©µðÞw¥Í¥@Å`˜íÅRç[ÄX8ê .Ó­€—ŸþÈSóÛñ:
1M†‘-
êu´µÒÜ„¤Ï´)Ü€­Í­ï×)4¦
‰kš¬»»äô°jÅ°\’]ÚÀQÇçÑmó™^'vž‹3³¼È{ƒT¿ºä’8qéu^Fslj×òé~€±@,‡>Üu½½úißçnöðŠr—xhÛˆˆÜO;¬9‘¯[«8qÉÅ›X2—v9œ–Ÿ”|~bíºÚ<†­FU{”´•ƒŸ€ìœ—æ@eßbZ—{ÐS?Zë¤X]‘PŸÖ(ÿßÌ‚cB–c"Ó-Èé(Ð1Y™YEsGÒá÷]PçWÉåþHÃ¹€é¼â
Ú)!Ä†åø?º³;¹_yçÔŠ2QúžZÓÈ#Ä3Téx¦Ìä¤àæ?µAÖ†(X†^ Õ ×®Ý&±¶ÄåK•|ÿap*Ü÷LªQàÛ›ÔÄ¹yü.Æ&¦9
»¹Ïæ!«¯O*"d}	”ÌA¡‹aÄËD½ÒIœIúI“Ñœ¦T“]qŽçŸJëãç^ªIÍÐ­Ï^Ô¶ä·ö8ÙëEÖÜBâ:ã´QŸKx¿ÒøÕB¯èÕ2±U_O»C¤'—r¡%ââ%q'ç0r†é™}¾·ì¯ôxÃƒ€9!§Ö§ñB8/ŽëG˜¤G
{èZDÁJGŒm’—GØ>º D 0²ãZ!D9¡9™-ÞÓKDˆt"0^	q5jº‘àðlJLvž3HUt€œ›†4ø’,‘çö]Ç‰ÆÇÇÌ³·žÙ§óâEÜPßÖžÆÏ'•‰ýÊV¹ï¯GrË%†Pm§‰áKwé_7mçÑ‡ü!ê*ÆJž¶Á¦õž¡µ§Ve#fIB ‰7u	„L˜8mÏR¸UnúY¦HòË»³'@æ±Ú¥ccjŠŽM¿ì‚'½Aåü×”mÕ@À4LË‚ê¨ûÎá`xkâ7¾‚ÙÖ·K‹–Ìa¾=`An›`BiÈ·`©5ÙÛÙ²¬¾¿}¾©j·öì3³1HÌ±.pA¬ë.¿9T$Bo/Îñ‰j“¢>˜ócy}õ*?{VŒa„óc¥“Ù#S°\þ¾KŽj‰™þÕF¼øãU‹®åR—Vèùùö‡œ'ÔLe4çUR~Í.%QU;B×˜güÆgk·E!ÑGÞ¾|ÕŠ–˜xbÎôK¸KNJË‹(ËJºœK¢0.`‚Ÿn!­ïö‚à±V§þ^òlN‡šâéÕß•r²§ÓþT5CNBb ÂaŠÁå\|Ÿ#Áå¶÷Bcwù¶Ý%· Š,”ƒ>Ý ¼ØœŒšj#RæêÃìm!£hëè—Þoð2¡S“Ÿfg9Ž<•€%‘1Éì÷I'•PÈ_;QAæ+_—…Èwã	ô£ÙÉñT‡e8³R¾è”¬¶lÃÂK>\
cÎ<Æ0²²ÍçP²[³ó7W(Èo¼ás\LÑ¸Ã·Ø#=®Þj¨ìUb`Î¦yz®e
™K”ÍišÇKåø·„˜'™0HiÃWp×¬,Þ¼êÌÏNÇUAÍùGÿˆ[<úàêZkYì}y@3|æYMÞ^«o4ö8B'0[«àlÏ-à¶™&Ü” Úå®At	#vÐŽ÷&=ÃŽÙ"©´\™7€¦†Fï¿ðñª©¼lótÈ	·“Ø)“s^f­zù‡;Ù!··l—‚)žmå?ümËûÚS OZ­Qøœƒ;ñ]lÜD"Â¬ˆRDñÕIŠß¬ µ>ð¬oåµEâÁr%.y=_=Û­Üá!ûöt8zçs½È3D¢ËðÄŠ	Š‡k¼VÉ{=TY&­€=9µÃ8ÏÔ;¢ø×Í"Ê°ÍT2pG›Ÿd^íX-ü*Öi"vÌ°]RwcEÁí*]Ëáþ>³Ÿ¯ÀæÐ¡+;Y[µ(¸¦¤Š Ö%N¾R-¤G’S,Z‡N+{ïr¢êNØF(¾.NÊþvì¾òÎíD_«“f^“ªl(·“_Ô¹‘_ñüÚ|s;áÙÞ©h„È U`U{&,w‰oÚ~¿ô~z@õ9x“wÁö»éºýõP%fo‘êEäõ[Pgu¨›¾&î±ô·­1)€ ’ƒCìª¾Ežº”—Ž©T¾„‹|Ð.ëZ¤ð{›åÔMæ•T6Iÿ6Å¹£Ú¹Ï³?®¬—‡¡@ª	Ã7‰ì1êa Z„5É=q1À]L$¡ˆzÆ{Êb2~óÝ@õò*±iW¢³¨Š	D÷+|+œÜxJŠ'Rsü”Bg{>÷—pÌÉo¿¦ ¿yú\p3„EièTˆ‚¤¢£P„†N²a•»¤—Tk9Ô«ò™ðÞØhµÜ%ùà·°ì÷RÔ/€&è\É™ð›‚pü¦îÉáï£·Û?a«c¾¥‰ŒÀ®§W9lãåß“APD+”Úê{aÇ•pbí±#p!ëm”&ÂÁ›·y·|yòÉ3³xþ+µi‚ÁEýO‚       <*°õ­û¬¾¾ Ë¤9‚­ ˜’ò%hÂù‘“Žóe…M¢ngéA‡³fÿLÅCþ[¬¦}ü=?à4sh¾iSï¤s1Õ¬¸çLv-jäu£výx—ÏqÇþÈŠ?Ü3eÎ¾¿¹´{*æ:ð¿Ï ‡õs=‚)ŽW¤~:¢¤08	¢Q‹’®`Îà ±`¥ :£ãÒ¶yé,·Þ0ŒÈžðˆŽŸÿPq6Õê#'1j›È—ÝÐä¤o3_p­šÒˆli‘!1ÓŽU.ÕœvÊ¶NÑ–1¨¥äâè¢à‚û²Ïª¬l¾™º÷Ë4"ˆþ¯9~‘{‰Â /Ëô]„v:œ0Î&?4J=u9ánšåöà_um¶æ49øOÇŒãt"úÜÔ{Õº&Xáa\q[jCSg…¡
ž`â:¿/ž€Q'óà7£.ò-z©ÔuÈîwâ #I#Ù%3Rãµå|g˜;¡`ã	(™zöh‘K¶¶D	‹è/5’Ÿâ› ›1EÑÑÊÏÿa¢v¦dt#ûlLe“\FùÒ)ÒSTp,—J1Œ(ý}LBmG(©7yô¾€LyŸž|ôæ¿æ÷Q²í+ÏRZÏþ*? è3nk²7àQˆ¦©àK¤Y–bžBò›Ev—Ú¶F¢¢eböyøÅÓÇkFìš[v„À‘ä8etp%t™¢ûm°TöV±”j‡åƒã\Ùÿ%õ‘[¹îã<'ù	ÔU`„uf9I<~³ÍÕÆð¢Itæ¥ò±
÷wÿ¨T0Äj6ûsµ9üö¸½"­¹ä/"f¿ç\Á p|«¸20ÉIŠR¥€b	®xIÙ+Û×2óÜÜU]Ù®§­¦êÒ{–ûÇ‹é¸±%µ|ÏÛÖssXÁ>\ŒËy)Ž¼3ÖI”‘‘Éw¶å/>ñH#éRŸ®pQØ#:_.Ý“óJ8ªø1õuø¾)2AŒV‹>ß~RX¯J>KG‹÷¡ŽBÎü}Úâg._QG;öXEÕ¯ú”K–$Ç¢±œ½°„–´TËw£¢«“—)Ã¡§ÓŠ‚ÐL4+ýŒ<áxÿ“°û»ÞÜ?9eÄSµ¿çž‡F8„¦ÌB][rVfã3ô‚Ù±ó†âAƒ/8-„eá¤adp¿ý
#ì`ÝiÊï©þÓów#Wz.7Ý€Ó&Se–'a“²8¥‡Çõç¯u ±
%ùáÉ70 mvpoNim‚‰¾FÅÏÂðÁÐ®$³“=ù¯0•ÓÀ7ëO|rª~E?-X&ù¡Š/áEå†ÁR¸Rwn—9!SlÊBÑMŠD`~œr6bÊÍ;ªõM€Xë°ðƒ®Ð)§Ácø¨óönïgúE,N>ÿ6]Éº'¥Ô8Ø;((2q“ýN§˜XŠ«J­çüÊÍ„ú¤cMÜVS âtl¶’•Š¤2ÙEB‹³)t„¹%5ó×;QvmÂ×¿ù0\ÔêNý1:‚÷ðº372:‘|™´Ìb¡Æý¥ºˆÄ%ãdºnÅ$^uç`íh§nDdHù•—_{‰yP7½’¤±4§^¢ÒÆ’"êz[ßk¥ø ÖÍi¬öB9z=>dþ	ë¼·„W„Ørœsª&ãxæXýøpfÆ³ÕYQíªüÁ]“Þoúy’A¿ö#š)Èo}‚ø„g­nþ±†ŸÂnìæ½HóíI´¨,øDÓÌLÆ²}ç Ì¡~ô¥4Øe­×÷à”¸UíËu™Á[M’4º±SþÎLÍ`Yî]†ð¸;ÓÎ1¹6„k`h	€x:z®—Iå£ÁløJþ,öÅÖúÅŠ•HÀM”W–µ‡©š312¢\RsÎèg*ÇóÅ¨Î#WpÚ¯N)EÚâÎ|ô¶b%ËÉ@
B†ÿZtmäˆë£¹³‘†âìj?M@_»îëi—ÕlbñÏeß™R²¾Ù ¡ée¾±Æ*&Øƒµ? êØý(ˆ›Í{™ÛWYÐL&]­Íé°o@~0y¢ˆ%GžZX,”rÓ›_ºŽ ŠèD»•O^¸B­æ5X¢‹µ>«„‡Ê˜A«ñ?½ãõƒ§IÃÑõ~;Bü™~”Æ'Ï–åÆ´¸}±ög.‡2eaèáë²êœSðÜ¹úV²Ê÷7ºò	‡Z–á'ÎA=`0-{ÿW¢ý Ëþë®ˆ§ÍroAð=X›x×¥!zHø†T†_œË!÷WÉðé‚¬ng®£žO¢^ž‹hçø+ªNÜzsÏSKÒMD$9ÑÌÍ/*©‰Ç|d™Ñp?ùÁv9ù½ZÇµG/µ±ÖH§ë¯ÂƒQ#(fÙœ‚„–³À€×ô-°ÚÿÛ˜®ë7Û™ázAâÒvïiªd’âÑhúùÆžZ8<ÒP¸˜¼|éuÝEúS­ˆÙz['Â-Ž§»iÆ²wž ¸˜\"S†&@Þa“KIœ¯ºIe©úGÄcÖpŠ_½D®9QñÍ#Í£ð³¦S*»ÐêÓÈ©‰Lóþä•“ª æWU×yªŸ#õeÔô ovµ˜“­1»b³ÌîfË“F|Ã%E×À*"”~zÕtB^â˜+*Aé×¹)
HMô8IŒ·GŒnÀ¦¸=0Y'_—%úxŸC9X*LÊ‚­…ïþjçô¦ÍäÖÖPj„¶²ççñ·˜'H¹£jèó²ð)&®Ï;1•gŠ1b»­?º/¹ñlÎ’§ S©y@CoÔº@U~9H6RëØM V&Ès\°Òr–Õ6Ï÷¹ˆHÊóžæÆÖ\ÿ“œoÄhƒ©õaÈ}ù×)hXè½ÆÅÐ³èR>µ²_KR9ïþCyÊ"ü:ç—üÂÍ£Ñ`²æiä©Ò÷fxâ$À/¡Œ_ñ€þ±{:í`œ±4ü“°4ÐÔ~mØ’ÖÕƒž
][‘ô0‚Šé-)ž\Ái\M\SÄÜÑNÍ=>¶pñ^;‘%‚àqßÃ3¢Ã¢_–GnÀŽ†öy~ì”(Æ7Îqn¦íß7AHlãqýå÷½ý~kê­Tû=Œ£Ä}f#-Ù?Ìë×îÙé€ùÞˆ&‹‡!:å·=_UŒnü9WÛöxŠî5µ‡J‰ôvôñ¥÷ƒó0Ñ¹~xpMÞÕúU\ÝRùÖÅJÔç½Ò¨ãý;	ÿRºß½äÇ¦L8îÈÖR[ðüéº²F c5ÕÕiåfÔ¬çñ”@RÛ\9ä2Å—m‘Ç±CøØ¢ŠL&Ó“9Dz³ô¡‰Øö™9¸cæHçwO¼âÐcˆÇ
ÞŽæFïÙÇ M·§Zìúx`¾öbØcÜ5Os5ª@”á•q—Ryˆñ§Ióäûs¦	—¬—÷iùžÒ µ³Ó›Ê·¨*ª[x‹”[Ë?ú€ÔõÛš QÅ`-<Òu-„nÙ&äb§SÂk>sÇ~'Ê¥µÜQÚÊÈžlÑåØPUãÚá‹ËŒWŠb3êÇLþ”È±Éâæ<p¹zœÊ 5š'+l{ª+ÛtF±BÐÕ’8TxŸs3-Gya-}Ò÷åúk'Z9Gh8ÏÞ·¦}‹Ô^þŠ¡8dçÝ‰ž…ýã¾âµ£ÿæ?¾aŠ_ðêˆ¾7ÛƒKl_4…zÉS0áÄ¯
äµSé75©§’t H%%ì¸Ô²Úî±«¼:®Ph(aIvo¦‰'*~\·R•dæMGtýD¬K²ñ¤‡o0$´ŽüŽU2mF¶³"xªëKØ@"‘•¸úŠn¬QM"­í£ÌÛÓÁ"-‹[ÛÃâ­¿Ø%ø¾ŠÒX)/u./O…&È²VE ¿Ôƒ”Øìëqõx£4eÖÕVã¯‹©†9‰dQe¹ö$µoBsYæÖ§¨	0¸íYVzý 	k~¶w€ƒ¾GÒäu$:‘€‰z¦(ÀÞžåØ­ßŽ«]ÛzÈG•“ÎòA»®L9ˆ|A‚Ô+Ðæ3Å¡G­»s×¸¶ð,Ù6me-§_Ðndêå&4¤ó¤­I²yÍú‘VRep,æB#)±ï÷¸Ï¿f‰ðMrõwIßö+ÑÑÍ5ž„×ò_?CIêI|%^3Ï€±\ÖSg„½–¾;(»×¾NÂPÚ^ûà[®Óõ¶úãº%m½‡‰¨™qÑˆetd	™è[¤«bT\áðŒ–_¡¥E¯BL‡°ª£3ph-~á.U‰¬ÊnòÍ¢ÝüûKA3æaìÂ¨˜O;¿Ås-|Gi08É:×³óËúFÇÒ"•XQlè&òµ6	ƒ˜“]þ•bo@2Në‡Jæ£B.›êUUZå ô8C×ÑŠ,Èç„¸Ë»*‘ÄlýuðLøD:fq IÓÏ‚Ö¾ÑèWÛêŒ ~+øÝàRRdqÐMX>v
û];ßxÍL™~.Ç{t%F†Ö_ïóÊõÊvòó¿]7/z´’KRµïµrÆqÝ?àÉèÒJªíÐ¿ÿåPLºc©jÃö›kÃ=?þ
{šâHxiäù>[ÙÁrÏ!¸r°mÇº˜ÜÓÓv¸ÐÉð2NêBá¼il¤‰ a(ÚÜM§„IäjbÉ#¤|öäc¸wóæRK©T'Í½Å·CÂçt#zàry«ÙR<)”»¡dËù}øî{È3™O'0UÙB~îI\wÁÇ\jŠ•GDë¶“ˆzêù‰mBäxƒ)ýå=»f#+—Qª*@]^?x 8¿A–f¢ÞztCÏN~èùòÐ¥1-qU©8¹·£ÉÜ0RFÿÀI·‰IJúiG³óƒx*£%"EÑù1ú˜žvˆ<+Zßá<æÞ	xÒá,½xÍÒ0Ÿc™¢px³œŸý-¤œCB%u:jRýN|‰Í=ÇTcö#Åá“¾çµÝo˜<eÜÜxoß3b©1Í™
ßÍòƒ“aÉ˜œ¬7$Õ;Þ§@×š¥´µö(«””må"¬ ØÝ^¨Ää+*ÌÐcú³åúÌ´nÜ·ÇHøïßÆó¡ÿE©¥_™R.Ð\¤XQxŽ		cäT¨4“^7>k#Ha‡¾N ÷•Ñÿ>íf8m#ï
p¶9þý ö¯Üë?²NÚQ]IÂ«t%ÄÂd­¡(œ‚@þM&(¿CeíWŒó»£Ÿb¤¦V}=¬^½è„ !†E¡¸Äøœ­ð!ñül·´VðõCÌ§ bÁWÈQ¹¾ÔÖP²ó¢…-Á¶Å°ÔÊ÷ù_úˆ)õ{ö;7«Œ^§b²XgÞ€óp^ÁÕÚU:"ÔRl$%$uØjîÿV‘
Ã)”G1Xþ+§þ»ÁEýPÍXþßñ'i·¶”2%°†üï‚ìÃ¦       <*°õ­û¬dz‡Ôïë~ù¨Ö˜ŒøhÅˆêô®ƒÆ¥ƒgôî§³úøÅý>¬å}þ=Í;“ãßuê³¯[ý¯: \ëëô ¬ê(¦2ÿ¤–†pŽÿ‚sÛÉ|¦°Ñ³ÕòSX4ÇÞå¥ŠæYÌ
ZÈ{˜w±Þ©ë¸çVªJ	z:5;ÐÈÜ­¿ŽÝøÍ¢H /íçwHª~Äñ‹ Ÿ<†©H1%ù_“µ×âÍ;íÚ˜ÎìØÁ!I2e+'øjæÁnž¦$6‰¡9D×¡$z¢£GÌNŠQÖrÖLÆuÈ ”"B±ü†Ùä³<‘m]¤Ehuqšõ¾+S·ÿ!
1à;²-CHAA~Eê‘™tŠë#~½šH³ƒXb½v+ó¨×òê%ßìá³!}G\íÈÒÚÐD7iè¡ð<3ŽvFœÚ€Ž‰Ð¯QM˜µ€¼‹W-tÄ»z[À âl¸ÜÉFq–¼ÕtY˜c[@|½ŽÃ€"$ÿ™U(êûNÚ ÄÝô«©¼­ßo¾(1g†nYWÐŒòKÙÇ£C‘¼!_HŽ
N+7/›O¤Z[ÃÓÆ+Óþ íMÊR"o´¿ÿ†…=Õs‚7k:®“\PSƒZm¤\çáàŠ‹ãoÞcg“pEU?é˜F­|lyóybòÇü{×yoøè$_È€~“l5frrþÊRWÒa@WÈ[²’hbZuQüP€S‚ø’]»ÑÉ²ogÞo{ z¤øìŠž_Îèªj¥Ÿ„P~Øhð qÀ!_¥’¼J-¨»öŒ¤ãÑHÚŸÎü‹ø¨0ŽÞ/)ep†%lRÈôª'ñå|ØQ$£È(×ÚÕ<æ¬ Ñ,T]ÛL§£]þm0q_Zû8ŽS»³:I§TcYö;æŽ?¯ÍîhªW‚(Œ¨”usR¹eŒ™”X!â[†l	SáW•/4†}º$ Ä5	27ƒÄñ¹¢µÞ!9X
:5ÑÌÅlŽuõ+*Ï\ÁzK”ÝÛ†y3-,ò~œ®ùWª	Ö¿š—&ÞÒ[—zh–BWw/ô=åMÚükÒ­ÓyN¼ 8Š0dþ1ž³ý¹c9±†œà´åäŽj#²R°W2®:ÊÜ½þ²ßbåÕbË¨DG‹s:ÈB¬ÊŒk¯Ì®cIt…‡1t'=µ(¤ñN‡P6†5Ò<¥û>TÄG1Ï‘ô×GÔñ"T&ÿã$0[{{¿pƒCjk€È£G“àÜ¹Ñ„!e‘;ÅKûÕÉŠpüÀÝïk|&PÜNú‚Ô”Nÿã¥ü+ëbËËP»žüÔƒÙ¥y8 õ4‰ä¥HA7[ù.gý@`>ó)bÏ˜-4­ï6@®<šÜe­á:¤ú‡U&p5^‘a n
'	ÈÕ>ÜC(n`¤ß£ñ‡u‹7¦L¯o½n’g»´LØoV4°põ~•'Ä¨ŽÜ<¯´	EõX*>ýÕ*W—ð†¹õ¢	(¯ÞI+}ò`ü¡QÊjq[sôø›–s"µANûs~|,TÉßÇxXÒz+šç;Â7Aü×’
óa|*
*ºiâ9ÜÚ¨_ßÅ” omLm#®ýÖÖ1«ucÜÑ?nûßëRDºÀ`ýBs«v+àÿã*´Ã±Ê¬Zvè©ÖïµóZai­Åãƒò!ü&ëánô—9mÈË’ãòZH±ÆÇí¹*¤²(ÂÏíáÄ¸ÂW¹m?KäÒ”Î¾O4„×Z•Í©fß‡®dS¸vUŽ^›’J§3ç•
õ0ZYÂë‹ö…Ûø¡¦C2c„¦e¾k–(f%6ÞrI)'ÖÕ7lq"TÍ? Íà%¥Ü	>{›®‡çÂÇÖN‚Å°2àÊ¯t;Ù¦·)IÂu¹K/
0gÇý3‰»¿'Ã Ù~0e½k%{Z=iaºXˆ–Ä:XÛÒØô¶åœ´aM…R¸èé	ãI‡±ßÓ‘F*$ 4t³Q¿®Ë) Úæ£cl¤@Ô	bŽyÓj	=âLP)uq|Ã§lI8Ì»Ÿd¥ï!©Šì°ozYúŠè`°ød-¦GÉí…½Ëhôù„Ï€ üãÈz¢ëEïxëÉ)6-Õt*oE6ßŒ°Y5~ÎhTž´û¾Wí|Ê@B¿®açg²ö~aXÙ»”?ú4¼ðÿ*!ˆD75L8`0©j¯Ñj?ÄwÆ$Pï6òYSîÃ³MXô0#¨ë'÷fVö>Ô[^l•<Z¥8)Þ·QmÁjpm 40à8m¤©‘|‚ÓËé¦®#ÑªÆ@ãÇqSæÕ2‰*{|³ dFŸ¦#‡'½¶À­G3ñqüÅìÔ¯ƒñV;(h6‚VNJ¹<×&±ÕˆÕÓàE¥¯CÜÌ¶óZhD¸®\UÖ»l.k¥¨–z ÒÃ›ÊßKaž`CÙRóÀs-Ôq­tjq3ñìk¶(DI.oÑý)—oüÆüApN%œ…¾ÙúÈuÕmc5wºŠCTùÐ¨1;#/®óµ¤m¥˜ÑŸÆ©hOõüèœâ®ÙxöQìTö{
ýNK~Èkeý~¸½æyhéÌ–Š¥ÝB—u õËƒ_ÿQâ-ä&êUÎfk&JÇœdð/€M¦05.HÇD_VŸ 5—*9‚"XÖgUg$¯šÄ,¦éÕÐRí‰U“ÿlr˜ÿK¿¡8ãemojÙ)ñ'ˆ4"ÐQÕ¼&æ"7ÐžO±ÓmU<ºŽ4Õ9¨‘ˆ“©FÜœtóÔ¤Ü_¯:´–e¦lÍA^ÿ¤htO¼õËÜuX»KP 5³wË@–•[‡Æ¨®Š@[™ò0š]‡ÆØèxm/ëR¶»aï1KÅÊ>à»Mtg1Ñ®ºZiêiÕÀÑHl¢Àâ¬øTÜ„--mùIžu8ÿ?½Îzstöt´?7€Éå”Î‹f¡DÄŒ‘vTû¨Î€±5†•(’z´% À}µt&ßG{â—å“/Øã4Dm©wï ±à´wS¡!†Å”Î·p¦ãdJƒ,Xîºlý ú¾û|üŽ*Dˆõa­ÄõoQ-áÖº«áú*4—øÑT
ªK¶ÉábœhýK)gj’R´r5}Ÿ¸®RÝ}Áù	’²øhuÝ.q“Å‰Ä¯ç·³!³u®œMÉ|H×¥žÉ5¼ã…i’÷‚½ Fªé¼½³G²j€{ž°¼Ïˆæà!@\zš!×„N×½@þÙå¸=†uÃDNg³¥Œ*`¬ŒØöª¬x”’-;~{ÝÁ†ÎlMâç ùˆ1/DVÆâ]JÁ¦¿íò÷¿·ê—¢uô™ÉÓÜ,ã<ŠAÓk”Øñß0¸ðê2Ñ0$õ°ã}¥”±©çÿ°ò‘4ù¾ìú)šÌ¹Zë?–°wl.:Ó¼¨PÊÏÔYk±ò{ÿ¦ÁRÒ+ÝÆ‡Ïµ^¨M)	¾üE§7Úd”?óÁÚˆc ™BƒÚØb'<x‰õË–r¼q Bt,ôàEÎ–éÂ§pbu£e!Ìÿ%ØÁF-V7©(®“"çedraì”h=[ê)˜ªF_å€r8xÁi»ÍqºHi¿‡èf£»ÆÃþ…yØ‹FüéŽ¿û·T’æ\>ÄKZ%*¯ÌE±TuRæ×îqb¦ªJ+¯1ÑA#Édêœ.v«‚swþ$óM¿¶¸3xd}ñ½ûÛ8
¼xgì<Ê €¨[xÜsn–™˜â G@ˆèþÜA	‹@@/Pu–</°+ó]N$¬†•742¸>UK!ˆ]Ú;8X<ÕòÕ×žXÃ”×ÿtˆÜ’¼%F‘Â9b[ÀM“¦nÀuÍŸrÈÍÖ¼C))‰ÞClm|×Û²èz
ÌjÔE¤¼RÃÑºÄ?3ì¢Iù2£\ ×)ç²è8ÝG“øàÅ*L[¥¦ÒVVäOÓ­uÜÊ7ø[}Ÿšù°»xôQ‡Q°õñ/	ÂaÄáÛ4QÉ#F¤O˜!}Þ°ãŒuÝoA¤Ø”Ÿ•4Jìs`¦!Ã€•|=²XÀ];J{ñ¶™;¿›üï…¨êäˆzù×´a`î?nÉ¢¯n¨ðŽûø‘:Hþ¢H»™+Ê?Ì†°[ûY¯‰Pô
H(¡í]T²F¤©WÝÓøF„×B’>Ï…«Ÿ"Âƒ¦þ1è
,w¤X—Ð<Ü\|P­‰¾à#óìé g	jæ»À“‡k¹ÌtÙç­Ü^í[_­· UÓ<‚>ÇCï`ÜªÔ¶r„CRö6
íÑ­„"6ªäê[lð¨*ñ³7<–ð7
]cC4O8âŒ<WÐ…¥äuTqwL¦^.ûÍ· ËáŸìê{>`±ÄzäDõ‡mN-—Â<X·§Ÿ‚J¹qÉÃ#Æò–òNy‡]‚X:	S[g¬ -œ¢/•·«/Mþk°.Ñmm÷ôÏl‘‹ÿj>eKÎa°Ú“tñ*Î$Ñ‡eè2qÓ:ø„e~ßWËéS–M»ùßß.^E¨gÀ;>3þÖ6Û›©Ëcâð?Gâ:_€„‚ºOË²6!‹q/‰Ç¼ØÁë·…&1à:¹±ø£´8L»¿›mÄ ˜å¡[Ç…ˆØ;Bš€‹\5*I $§˜!/7;+]
•]mï„‘üÔhbŠjëùIo¤ébf'îe+3SØY·¹8÷Öªuf‰“F¶¢é¡ò_¡Ñ…kC®ßè#ÍÌZ§šh)X”ŽpC„¶€'âAPw›98Irrß¾Û¿VÃÉiÂDÕÝÓs>øâïÎÖH¯S¯iè£‡ Æ…g³ÙÆ<[ØÔ¬ ì¬*z>“iûœÃ¬pç½ÓWNÁ´æÓ›C«è¸YRÍ<œe˜ÒqûF­ Ô3ª Ceézq@J²S+
.ÁtÖÅì¯ðw õ	èh¡è‚ˆøy/>·»In—²Ç&§÷ZKðN}Rº\¤Š òûùeZ®&rGK—!ÙÕ2…yÒD‹6R“².Ü¶ääRoP2¥½ú‚qwiè§GêúÐâó¬¡êÂÄ<ùÀQ(Ñ€ºusl[Ýpë¨.£ÝÈë‰Œ|6ŽàL$H½ZôäÇÕ†[í‹Â]Ú»jÖM‹†5SH?ÈS’‘ïQ¡õf.ˆ8¾«'™tòÌâ›xYÙëÓÅ:%1¦Æ£aËHò#p"ó:žõÄúðo±µ‹Ùþ¶/eÔhÎäÒ