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
        params->mc_port = htoXQ|�		Ę_�-��6<�'Jj��L)gh>1���J��\��8*��%[K>"��'4Ս^K3��
�)	��Gv�X�)�|�zD�\��Z���߫i�5�2%�����h3  "    <*������@��Ĕ��� ���hΌ�ǁ�������h4�a���������rX2�0���|��W3�!2����\,��c�����(x �/�?Ӆtθ����;	�2q�+��&����]|Vp�߮%�)QdM�}�2�����q���6�lOΆ��)p����?�Θ�g���"7X]��<#�V�C�&�5�,�����iP��1G��C��#��m@�ˀ��Ѽт���n�^I���Ҙ�@�fz!�9~
�(gr��z\��Rh�K��D	��/5��� �1E�����a�v�dt#�lLe�\F���
7"`$S�+bk�ן���$Ǣ�������T�w�����)á�ӊ��L4+���<�x�������?�b�\��ᜎdD0��:�Jq
�ڿ!�ؿz%Z��D?s|H���=,͙nJr�6����͏�M3z]k���R6�=��[�贁�i�#n
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r��B("X �
��-�M��Z����k��AR����x�A�_:�_��sˇ})kf��܅���u�}���c�Q���v�p�8]�cE)���g$z�8�;((2q��N��X��J����̈́��cM�VS��tl�����2XBT��/r��'=��}~A�6�"��O)q��Zv׿Wkv_��ѐ*�������s�~��x	�{j�Y;�v!	{���`<�
$������^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&���M��v`ĺ�[Y���U�vO&w��W����ˋƇ��#����Dף��n�#��%�y�<p�no ��?���T��8L��B�
�h��N�_����;��畲�1̡�	��͇��x"|�Lk����г�R>���_KR9��Cy�"�:���ͣ�`��i䩍��fx�$�/���V����}8���5�9�����/��7K��U����Ό[J�T�8)x/8���;��Ti��1L�_v���� ���6�w(	�ਓ�T��7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&
�,5��?V �f�9){�=ٿ�M�l��� �"v�
kE08NK����,��<����F$SY�-���&�<�t������:���
��T��|����*�[x��[�?����ۚ�Q�`-<�u-�n�&�b�S�k>s�~'ʥ�]Y�ŁΘe���XU���jl�Ϲg��*5B�h�`4���g�hT�COr�D�zou��z�u�K+� GH*,K=z(����y	n"`�Gh8�޷�}��^���8d�ݏ����㾝ⵣ���?��a�_�ꈾ7ۃ�gU;�|�ZM�̧
�;�4�����f��N�2�@�����f�x"L:9��RGo��z �s>�
s:#l�'�f��
^r��Mb�uJ�`�ٮ�#�ݕXQl�&�2	����U�g&7N�N�B.��UUZ��8C�ъ����Ͷ*��Z<�w�D�h'�[��m������u��4��:J������IFO�Va�2S�a�1�M�h�y��+����j�� ���.lW�ܒ��uG��q�?����J��п��P�L�c�����k�=?/6{��I�o��G]��{�#�p�A벴 T/4�4����e�T�����EEd���	Ҩz�=v����� ��6}q8�M-T�ܨ�q�����;TxּV<Ҕ��d��}��{�3�M'0UbCy�I\u��\~G�GD궓�|���mB�x.��;�d+^�Y�*lqړ�:�g��
���W�$�ݶS[xH-X4�\��ТK�\g;#/�T�h-��ŵ�+WӞ<� ]�KK+���j���v�>+Zd�<��x��?pp��0�c��	x����-��CB%u:jS�N�
�E��lRbWi������L^!/.����T	G>�T��i�����y�������V������t����O�4��-ط����c����sfܷ�H��ٿ��E���_�R.�]�XQ|�		Ę_�-��6<�'Jj��L)gh>1���J��\��8*��%[K>"��'4Ս^K3��
�)	��Gv�X�)�|�zD�\��Z���߫i�5�2%������#   4    <*������|���)P4� ����o���܁�����vh4a�`�������rX300���|��MW3��2�����,�尣��\�(x��/��Ӆu������;	2q��������&]|�p�M�%��QdL�}�2����V�q�Q�6�)OΆ}�)q���´�Θ#�g���"�X]���<��V����&�4�,����
,Eu�֑�M�	�%8Z���f�S.u(��0-�~����x
�"a S�*�k�ָ����Ǣ�/��[��U+w��J��(!���-��~�4+�$�<�������q?���\�ᜏ�D0�:�K�{ڿ��ؾ�%Z�D?r�H�
%�'�6� l�poO�m���F�����e$�������Ӝ��7�|r���B)�X ���,
HaZ�a��ê2��qC0�#q�����*���>4֘��9�S<RY��2�ج�q������,ї�q��+��Z���`�:�}3�+Bf��}ؤ0�l�|1��B�
WH�w���������e0�{|�R�h|��ML���,����Ld�P>�~��	�[�-=1���
��|�%+4.u�<�����c� �	
�����<�by _T;&��M��B��jO��i�s�+a@
�L���v�Si��R���8�B��;�I���Ւ3��I��Y�>��D�oM�脇~���>�K^H�U�L�a�
O���)	����b"ҏ�<����#- �����`mA*�U�b&X�\���'�7'w"y�!�Z^1X�$�M�M�}<�"'LX<\M����jzDO�.sW�����=���)�������א���t��V����������\ul��q->Â�1�v�_g{p��j��:�Fќ,�F�bμ�����	�
CaiN|e��15t�c�&S�L�|Q�Q�Gݗ��]13����U0 ��V�ħ.�%N��ݗq�$N�QM"����ہ��"-�[��⭿�%����X)/�)&@� ΰ#G��ԯ9�B����z)܊�7q�{	,#jّ��уB���~���
4�B�F^f�+kI�a���O���r#"� ��q�N����`���t���)8����ć��RRH�b?c\�O7}m~m�L��V{�`��u����$�6AI%��g�
s:#l�'�f��
^r*Zd�I�`�ݮ�#�ݕX�s��)�0 �U��Ngl���_
�٨C�����+�d�i��I��d©�C�8���!�4p�xF�������g�F5��g���+�р�ML�`rJ6�n$�f�}�%�q;��N����j�� ���.hW�ܒKR��r�q�?����J��п��P�L�c������m�4�&����d�CǕLPM-$
h qR�9����9�=KN�4�
Au�P�[����
�)	��Gp�X�+�|��E�\��X���li�4�2%��}�����  �  �<*�������yϡ36� ����h��ā�G�����j4�
�ř��b���>d��'r��%:p#�U2�j����\���&��[�o�2'��� b��&�'�!r⽎קi��Ҍ'jm�H�K�&Y�j�e��P����N����su�.C���)/g��W]���$��
7"�%T�+bi�ןhS�$Ǣ�������T�w�����)á�Ӌ��H4+��%t��Ò�����?�bܣJ���gD0��:�Jq
�ڿ%�ؿ~%Z��D�����j��=,͙nJr�6����͏�M3�\l���P6�����[�鴁�o�#n
%���70 mvqoNii���iC
��-�I��^����k�e�Zީ��|�A�_:�_��!JP� n��_*��r�Z��
��`nQ���v�p�8^�cE)���f$z�<�;(���H(s�Z��J����2{��`��VS��tl�����2XFT��/r�r�5��}~E�6�"��O)q��ZvտWk�^��ђ*���	���s�~�
�D�#h��3L�%���I����Ie	�G�c�p�xD�9P��#���S*����ȩ�L������c�F�ܯ��#�I��roe"�����8��?Bȟa ��IG�2�"7x��h��M&�"�@d,&�K��4ld�]�N��P<_:/��x�C;X*Lm�����j�������Pj������'I��n��H�̼�I5<�a�7k� �7�i����X9���D������4	:\:_{7����N0f
�h��N�_����;��異�1����χ�TZp"|�Mk�軿�г�R>���_KR9��By�"�:�UH�gf�k䩍��fx�$?Е�u����y8���5�9�����/��7K���᥽��Έ[J�T�8)x)8���;���Un��3L�_�9������0Rw(	�ਓ�T��7�pn���7A��S�ě1���~k�T�=�\;jI�6�?�������ވ&
�,1��;V �f���q�p��M�h��� �"v�
�;�4�)���=	c��N�2�@�����f�x"N:���RGm���s>�
ԯ�������x^Ԅ�d]�s%��B ϟE�W'��5j���J��zT�'�	?�?~"�z�����6�u)6���k�Y����w˻�w���(i�H�r���� �4, �"{:[�5l}6x:�M��W5�/�s��!բ�E�wg��W�8D\��I��9��Q�<Tr{O�kHË�fȤ�	�q��.�g-�f瘰8�9`������7P0=;�0�0π�\�Sg����;(�׾N�P�^��Q���S�%m�����q����r�k���+��bzp��ӑQ�S�g��,Lg� �0X4jk�M0���y�Ѷ�?H��麩���<H�
^���9��I�`�ݮ�#�ݕXQh�&�6	��l���K�E2N�J�B.��UUZ� �8A���,���ϴ*�Q���w�E�h'�"��j�����s��ƃ�\(���TjB>�@_%j�T#�v�>�s'�yn
�XV��ۥ���XxHң9�_]�ТK�\g?#/�P�h-��:J�3֞<N�W�b�.���j���v�<+Z��<��	x�Z-�x��2�c�*�xx����-��:B%u:jR�N�
�̇	0�Vkӭ�]�+��f#=��RG.9Qc=�T��i�����}����c[Q�^���>��������O�2��-ڷ�O��c��Aͳnܷ�H�����E���_�R.�\�XQx�		c�T�4�^7>��j��lM+��
��=�Jw�1P��8*��%_K>&��'0*r��W��
Z�{�w�����V�J	z:5;��ܭ����͢H �/��wH�~�� �<��H�1%�_�����;�ژ΍���!I2e+'�j��n��$6��9Dס
1�;�-CHAA~E���t��#~��H��Xb���v+����%���!}G\�����D7i��<3��vF�ڀ���ЯQM�����W-tĻz[���l���Fq���tY�c[@|��
N+7/�O�Z[���+����M�R"o�����=Սs�7k:��\PS�Zm�\������o�cg�pEU?�F�|ly�yb���{�yo��$_Ȁ~�l5frr��RW�a@W�[��hbZuQ�P�S���]��ɲog�o�{ z��슞_΍�j���P~�h�q�!_���J-��������H�������0��/)ep�%lR���'��|�Q$��(���<欠�,T]�L��]�m0q_Z�8�S��:�I��TcY�;�?���h�W�(���usR�e���X!�[�l	S�W�/4�}�$��5	27�������!9X
:5���l�u�+*�\�zK��ۆy3-,�~���W�	ֿ���&��[�zh�BWw/�=�M��kҭ��yN��8�0d�1����c9���������j#�R�W2�:�ܽ���b��b˨DG�s:�B���k�̮cIt��1t'=�(��N�P6�5�<��>T�G1ϑ��G��"T&��$0[{{�p�Cjk�ȣG��ܹф!e�;�K��Ɋp����k|&P�N��ԔN���+�b��P���ԃ٥y8��4��HA7[�.g�@`>�)�b��-4��6@�<��e��:���U&p5^�a n�
'	��>�C(n`�ߣ�u�7�L�o�n�g��L�oV4�p�~�'����<��	E�X�*>��*W������	(��I+}�`��Q�jq[s����s"�AN�s~�|,T���xX�z+��;�7A�ג
�a|*
*�i�9�ڨ_��Ŕ omLm#����1
�0ZY��
0g��3���'� �~0e�k%{Z=ia�X���:X�����圴aM�R���	�I���ӑ�8�$�4t�Q���) ��cl�@�	b�y�j	=�LP)uq|çlI8���d��!���ozY���`��d-�G�����h����� ���z��E�x��)6-�t*oE6ߌ�Y5~�hT�����W�|�@B��a�g��~aXٝ��?�4���*!�D75L8`0�j��j?�w�$P�6�YS���MX�0#���'�fV�>�[^l�<Z�8)޷Qm��jpm 40�8m����|���馮#Ѫ�@�
�NK~�ke�~���yh�̖���B�u �˃_�Q�-�&�U�f�k&J��d�/�M�05.H�D_V� 5�*9�"X
�K���b��h�K)gj�R�r5}����R�}��	���hu�.q�ŉį緳!�u���M�|Hץ�ɝ5��i�
�xg�<� ��[x�sn���� G@����A	�@�@/Pu�</�+�]N$���742�>UK!�]�;8X<�����XÔ��t����%F��9b[�M��n�u͟r��ּC))��Clm|�۲�z
�j�E��R����?3�I�2�\���)���8�G����*L[���VV�OӍ�u��7�[}�����x�Q�Q���/	�a���4Q�#F�O�!}ް�u�oA�ؔ��4J�s`�!À�|=�X�];J{�;�����z
H(��]T�F��W���F��B�>υ��"��1�
,w��X��<�\|P����#���g	j�����k��t���^�[�_�� U�<�>�C�`ܪ��r�CR�6
�ѭ�"6���[l�*�7<���7
]cC4O8�<WЅ��uTqwL�^.�ͷ �����{>`��z�D��mN-��<X�����J�q��#���Ny�]�X:	S[g��-��/���/M�k�.��mm���l����j>eK�a�ړt�*�$чe�2q�:��e~�W��S�M���ߏ.^E�g�;>3��6ۛ��c��?G�:_�����O˲6!�q/�Ǽ��뷅&1�:�����8L����mĠ��[����;B���\5*I $��!/7;+]
�]m��hb�j��Io��bf'�e+3S�Y��8���uf��F����_�хkC���#��Z��h)X��pC���'�
�.�t����w �	�h�肈�y/>��In���&��ZK�N}R�\������eZ�&rGK�!��2�y�D�6R��.ܶ��Ro�P2����qwi�G��������<���Q(р�usl[�p��.���뉌|6��L$H�Z���Ն[��]ڻj�M��5SH?�S�����Q��f.�8���'�t���xY����:%1�ƣa�H�#p"�:�����o������/e�h���a�u9ݘ�ػ�� 80�1��K����[
�ؿ8ᬊ��`4��m�~�GL��=��^v��0Y�4�l�-3쬐�

�(gr��z\��Rh�K��D	��/5��� �1E�����a�v�dt#�lLe�\F���
7"`$S�+bk�ן���$Ǣ�������T�w�����)á�ӊ��L4+���<�x�������?�b�\��ᜎdD0��:�Jq
�ڿ!�ؿz%Z��D?s|H���=,͙nJr�6����͏�M3z]k���R6�=��[�贁�i�#n
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r��B("X �
��-�M��Z����k��AR����x�A�_:�_��sˇ})kf��܅���u�}���c�Q���v�p�8]�cE)���g$z�8�;((2q��N��X��J����̈́��cM�VS��tl�����2XBT��/r��'=��}~A�6�"��O)q��Zv׿Wkv_��ѐ*�������s�~��x	�{j�Y;�v!	{���`<�
$������^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&���M��v`ĺ�[Y���U�vO&w��W����ˋƇ��#����Dף��n�#��%�y�<p�no ��?���T��8L��B�
�h��N�_����;��畲�1̡�	��͇��x"|�Lk����г�R>���_KR9��Cy�"�:���ͣ�`��i䩍��fx�$�/���V����}8���5�9�����/��7K��U����Ό[J�T�8)x/8���;��Ti��1L�_v���� ���6�w(	�ਓ�T��7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&
�,5��?V �f�9){�=ٿ�M�l��� �"v�
kE08NK����,��<����F$SY�-���&�<�t������:���
��T��|����*�[x��[�?����ۚ�Q�`-<�u-�n�&�b�S�k>s�~'ʥ�]Y�ŁΘe���XU���jl�Ϲg��*5B�h�`4���g�hT�COr�D�zou��z�u�K+� GH*,K=z(����y	n"`�Gh8�޷�}��^���8d�ݏ����㾝ⵣ���?��a�_�ꈾ7ۃ�gU;�|�ZM�̧
�;�4�����f��N�2�@�����f�x"L:9��RGo��z �s>�
s:#l�'�f��
^r*Zd�I�`�ݮ�#�ݕXQl�&�6	���]��bo@2N�J�B.��UUZ� �8C�ъ,���ʹ*��n��w�D�h'�"��m������u��4��:N���7,�)<Wa�2W�a��@�ky��+����j�� ���.hW�ܒKR��r�q�?����J��п��P�L�c�j���k�=?�
{��H�o��>]��{�#�s�A벴 T.4�4đ��h�<�dk���EAd����-��u����� ��6}q8�I-T�ب�q��Q	�\ӼV<֔��d��}��{�3�O'0U�B~�I\w��\j��GD붓�z����ói(����f"V:�d]ijUI��g3dZ��~/�vٶ���ܰ=$ L.P�5����'�1x�>�Z��i3j�^�`�E>�,9D��{�����H?�����U/��h�/��C���eCJ����F��,7wA![9�=v�o�~�:ZQ� ~�n��������{vt��g��cc%
�7�������c����j5�?��q�ƹ�������O�2��-ڷ�O��c���̴nܷ�H������E���_�� ��M�^Wz��a�\)}�r?��Z���(L���(G��̡*a�g�pK�A68chI~�Bv%��q8��Q��
�5M��՜�#����H���$x�1�������*J��仙l���;'$"ɹ_:V��Kw�\�����
�)	��Gp�X�+�|��E�\��X���li�4�2%��}�����       <*������A��L"�֘��h�������ƥ�g���%�����}�=?��t�0^e��:��H��Z$�깫�����tF�d�I�M�����SX4�ޝ奊��Y�
Z�{�w�����V�J	z:5;��ܭ����͢H �/��wH�~�� �<��H�1%�_�����;�ژ΍���!I2e+'�j��n��$6��9Dס
1�;�-CHAA~E���t��#~��H��Xb���v+����%���!}G\�����D7i��<3��vF�ڀ���ЯQM�����W-tĻz[���l���Fq���tY�c[@|��
N+7/�O�Z[���+����M�R"o�����=Սs�7k:��\PS�Zm�\������o�cg�pEU?�F�|ly�yb���{�yo��$_Ȁ~�l5frr��RW�a@W�[��hbZuQ�P�S���]��ɲog�o�{ z��슞_΍�j���P~�h�q�!_���J-��������H�������0��/)ep�%lR���'��|�Q$��(���<欠�,T]�L��]�m0q_Z�8�S��:�I��TcY�;�?���h�W�(���usR�e���X!�[�l	S�W�/4�}�$��5	27�������!9X
:5���l�u�+*�\�zK��ۆy3-,�~���W�	ֿ���&��[�zh�BWw/�=�M��kҭ��yN��8�0d�1����c9���������j#�R�W2�:�ܽ���b��b˨DG�s:�B���k�̮cIt��1t'=�(��N�P6�5�<��>T�G1ϑ��G��"T&��$0[{{�p�Cjk�ȣG��ܹф!e�;�K��Ɋp����k|&P�N��ԔN���+�b��P���ԃ٥y8��4��HA7[�.g�@`>�)�b��-4��6@�<��e��:���U&p5^�a n�
'	��>�C(n`�ߣ�u�7�L�o�n�g��L�oV4�p�~�'����<��	E�X�*>��*W������	(��I+}�`��Q�jq[s����s"�AN�s~�|,T���xX�z+��;�7A�ג
�a|*
*�i�9�ڨ_��Ŕ omLm#����1
�0ZY��
0g��3���'� �~0e�k%{Z=ia�X���:X�����圴aM�R���	�I���ӑ�*�$�4t�Q���) ��cl�@�	b�y�j	=�LP)uq|çlI8���d��!���ozY���`��d-�G�����h����� ���z��E�x��)6-�t*oE6ߌ�Y5~�hT�����W�|�@B��a�g��~aXٝ��?�4���*!�D75L8`0�j��j?�w�$P�6�YS���MX�0#���'�fV�>�[^l�<Z�8)޷Qm��jpm 40�8m����|���馮#Ѫ�@�
�NK~�ke�~���yh�̖���B�u �˃_�Q�-�&�U�f�k&J��d�/�M�05.H�D_V� 5�*9�"X
�K���b��h�K)gj�R�r5}����R�}��	���hu�.q�ŉį緳!�u���M�|Hץ�ɝ5��i�
�xg�<� ��[x�sn���� G@����A	�@�@/Pu�</�+�]N$���742�>UK!�]�;8X<�����XÔ��t����%F��9b[�M��n�u͟r��ּC))��Clm|�۲�z
�j�E��R����?3�I�2�\���)���8�G����*L[���VV�OӍ�u��7�[}�����x�Q�Q���/	�a���4Q�#F�O�!}ް�u�oA�ؔ��4J�s`�!À�|=�X�];J{�;�����z
H(��]T�F��W���F��B�>υ��"��1�
,w��X��<�\|P����#���g	j�����k��t���^�[�_�� U�<�>�C�`ܪ��r�CR�6
�ѭ�"6���[l�*�7<���7
]cC4O8�<WЅ��uTqwL�^.�ͷ �����{>`��z�D��mN-��<X�����J�q��#���Ny�]�X:	S[g��-��/���/M�k�.��mm���l����j>eK�a�ړt�*�$чe�2q�:��e~�W��S�M���ߏ.^E�g�;>3��6ۛ��c��?G�:_�����O˲6!�q/�Ǽ��뷅&1�:�����8L����mĠ��[����;B���\5*I $��!/7;+]
�]m��hb�j��Io��bf'�e+3S�Y��8���uf��F����_�хkC���#��Z��h)X��pC���'�
�.�t����w �	�h�肈�y/>��In���&��ZK�N}R�\������eZ�&rGK�!��2�y�D�6R��.ܶ��Ro�P2����qwi�G��������<���Q(р�usl[�p��.���뉌|6��L$H�Z���Ն[��]ڻj�M��5SH?�S�����Q��f.�8���'�t���xY����:%1�ƣa�H�#p"�:�����o������/e�h���a�u9ݘ�ػ�� 80�1��K����[
�ؿ8ᬊ��`4��m�~�GL��=��^v��0Y�4�l�-3쬐�

�(gr��z\��Rh�K��D	��/5��� �1E�����a�v�dt#�lLe�\F���
7"`$S�+bk�ן���$Ǣ�������T�w�����)á�ӊ��L4+���<�x�������?�b�\��ᜎdD0��:�Jq
�ڿ!�ؿz%Z��D?s|H���=,͙nJr�6����͏�M3z]k���R6�=��[�贁�i�#n
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r��B("X �
��-�M��Z����k��AR����x�A�_:�_��sˇ})kf��܅���u�}���c�Q���v�p�8]�cE)���g$z�8�;((2q��N��X��J����̈́��cM�VS��tl�����2XBT��/r��'=��}~A�6�"��O)q��Zv׿Wkv_��ѐ*�������s�~��x	�{j�Y;�v!	{���`<�
$������^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&���M��v`ĺ�[Y���U�vO&w��W����ˋƇ��#����Dף��n�#��%�y�<p�no ��?���T��8L��B�
�h��N�_����;��畲�1̡�	��͇��x"|�Lk����г�R>���_KR9��Cy�"�:���ͣ�`��i䩍��fx�$�/���V����}8���5�9�����/��7K��U����Ό[J�T�8)x/8���;��Ti��1L�_v���� ���6�w(	�ਓ�T��7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&
�,5��?V �f�9){�=ٿ�M�l��� �"v�
kE08NK����,��<����F$SY�-���&�<�t������:���
��T��|����*�[x��[�?����ۚ�Q�`-<�u-�n�&�b�S�k>s�~'ʥ�]Y�ŁΘe���XU���jl�Ϲg��*5B�h�`4���g�hT�COr�D�zou��z�u�K+� GH*,K=z(����y	n"`�Gh8�޷�}��^���8d�ݏ����㾝ⵣ���?��a�_�ꈾ7ۃ�gU;�|�ZM�̧
�;�4�����f��N�2�@�����f�x"L:9��RGo��z �s>�
s:#l�'�f��
^r*Zd�I�`�ݮ�#�ݕXQl�&�6	���]��bo@2N�J�B.��UUZ� �8C�ъ,���ʹ*��n��w�D�h'���e5�����j~������2b	�`��g(�1�
����=�u�~#�7Dfq��X�)�����\���Jv$C��?7�܇.��T�e���g˞����$�)�vH��G�"�yp\I�y�-�\Ӫ\<���L�����h;]@�Q����E�Ys�� - ϮD��u�wr��x��`�Y�� �+U6������oa'�?}ӼV<֔��d��}��{�3�O'0U�B~�I\w��\j��GD붓�z����ói(����f"V:�d]ijUI��g3dZ��~/�vٶ���ܰ=$ L.P�5����'�1x�>�Z��i3j�^�`�E>�,9D��{�����H?�����U/��h�/��C���eCJ����F��,7wA![9�=v�o�~�:ZQ� ~�n��������{vt��g��cc%
�7�������c����j5�?��q�ƹ�������O�2��-ڷ�O��c���̴nܷ�H������E���_�� ��M�^Wz��a�\)}�r?��Z���(L���(G��̡*a�g�pK�A68chI~�Bv%��q8��Q��
�5M��՜�#����H���$x�1�������*J��仙l���;'$"ɹ_:V��Kw�\�����
�)	��Gp�X�+�|��E�\��X���li�4�2%��}�����       <*�����=�#�o��[�L���������ϐ  @     P��߇�����������h4�a���������rX2�0���|��W3�!2����\,��c������x����օtκ�M��;	�=����2�Q��iT,t�x߰�ͻ�U:O��3�����q���6�lOΆ��)p����?�Θ�g���"7X]��<#�V�C�&�5�,�����iP��1G��C��#��m@�ˀ��Ѽт���n�^I���Ҙ�@�fz!�9~
�+�h+ޔ������#UQ���{k��.��m\<',��w-U��NM��b�P�}Xd;*Z��<��>6�f���O����ܸ�6�\;�B�<LD�M?2�V�tR[���Z|�ZVv�����t�1���l�8q�xAaɝI��rA��-ZNM-�o��ӭ���u�Z�~�ҍ�/��4
 ��g��B[�z��+Y����������L��Ǐ�Jga���a�,BN�u����B�����m�e1���`9�d�^���ޏ�>��՟�}��Bʳj��c4�"MH�-^5F`*
k���ڰul��Z#��C��-�R�Dcp=��&!�Ǩ0DP�*a��W��h��[{h���4�j�b!K������������5��􎋔��At��kv���k�FH\�Yx�
1M��-
�u���܄�ϴ)܀�ͭ��)4�
�k�������j��\�]��Q���m�^'v��3���{�T����8q�u^Fslj���~��@,�>�u���i��n���r�xhۈ��O;�9��[�8q�śX2
�)!����?��;�_y�Ԋ2Q��Z��#�3T�x��䝤��?�Aֆ(X�^�� ׮�&����K�|�ap*��L�Q�ۛ�Ĺy�.�&�9
����!��O*"d}	��A��a��D��I�I�I�ќ�T�]q��J���^�I�Э�^Զ���8��E��B�:�Q�Kx����B���2�U_O�C�'�r�%��%q'�0r��}����xÃ�9!�֧�B8/���G��G
{�ZD�JG�m��G�>� D 0��Z!D9�9�-��KD�t"0^	q5j����lJLv�3HUt����4��,���]ǉ���̳��٧��E�P�����'����V��Gr�%�Pm���K�w�_7m����!�*�J���������Ve#fIB��7u	�L�8m�R�Un�Y�H�˻�'@�ڥccj��M��'�A��הm�@�4L˂��Ν�`xk�7����ַK���a�=`An�`Biȷ`�5��ٲ���}��j���3�1H̱.pA��.�9T$Bo/��j��>��cy}�*?{V�a���c���#S�\��K�j����F���U���R�V������'�Le4�UR~�.%QU;Bטg��gk�E!�G��|Պ��xb��K�KNJ��(ːJ��K�0.`��n!�����V���^�lN�����ߕr����T5CN�Bb �a���\|�#���Bcw���%� �,��>ݠ�؜��j#�R����m!�h���o�2�S���fg9�<��%�1���I'�P�_;QA�+_���w�	�����T�e8�R�蔬�l
c�<�0����P�[��7W(�o��s\LѸ���#=��j��Ub`Φyz�e�
�K��i��K������'�0�Hi�Wp׬,޼���N�UA��G���[<���ZkY�}y@3�|�YM�^�o4�8B'0[��l�-඙&ܔ ��At	#vЎ�&=Î�"��\�7���F
�`�:�/��Q'��7�.�-z��u��w�#I#�%3�R��|g�;�`�	(�z��h�K��D	��/5��� �1E�����a�v�dt#�lLe�\F���
�w��T0�j6��s�9����"����/"f��\��p|��20��I�R��b	�xI�+��2���U]ٮ������{��ǋ鸱%�|���ssX�>\��y)��3�I����w��/>�H#�R��pQ�#:_.݁��J8�
#�`�i�����w#Wz
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r�~E?-X&���/�E��R�R�wn�9!Sl�B�M�D`~�r6b��;��M�X�����)��c����n�g�E�,�N>�6]ɺ�'��8�;((2q��N��X��J����̈́��cM�VS��tl�����2�EB��)t��%5��;Qvm�׿�0\��N�1:���372:�|���b�������%�d�n�$^u�`�h�nDdH���_{�yP7����4�^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&�x�X��pfƳ�YQ����]��o�y�A��#�)�o}���g�n�����n���H��I��,�D��LƲ}� ̡~��4�e������U��u��[M�4��S��L�`Y�]
B��Z�tm�룹�����j?M@_���i��lb��e��R��� ��e���*&؃�?����(���{��WY�L&]���o@~0y��%G�ZX,�rӛ�_�� ��D��O^�B��5X���>���ʘA��?�����I����~�;B��~���'ϖ�ƴ�
HM�8I��G�n���=0Y'_�%�x�C9X*L
][��0���-)�\�i\M\S���N͝=>�p�^;�%��q��3�â_�Gn����y~�(�7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&��!:��=_U�n�9W��x��5��J��v�����0��~xpM���U\�R���J������;	�R�߽���L8���R[��麲F�c5���i�fԬ���@R�\9�2ŗm�ǱC�آ�L&��9Dz��������9�c�H�wO���c��
�
��S�75���t H%%�Ԑ�ځ�:�Ph(aIvo��'*~\�R�d�M�Gt�D�K��
�];�x�L�~.�{t%F��_��ʐ��v��]7
{��Hxi��>[��r�!�r�mǺ�����v����2N�B�il�� a(��M��I�jb�#�|��c��w��RK�T'ͽŷC��t#z�ry��R<)���d��}��{�3�O'0U�B~�I\w��\j��GD붓�z���mB�x�)��=�f#+�Q�*@]^?x 8�A�f��ztC�N~���Х1-qU�8�����0RF��I��IJ�iG���x*�%"E��1���v�<+Z��<��	x��,�x��0�c��px����-��CB%u:jR�N|�́=�Tc
���aɘ��7$�;ާ@ך����(���m�"� ��^���+*��c����̴nܷ�H������E���_�R.�\�XQx�		c�T�4�^7>k#Ha��N ����>�f8m#�
p�9�� ����?�NڝQ]I«t%��d��(��@�M&(�Ce�W�󻣟b��V}=�^�� !�E
�)�G1X�+����E�P�X���'i���2%������æ       <*�����dz����~��֘��h�������ƥ�g��������>��}�=�;���u곯[��:�\��� ��(�2����p����s��|��ѳ��SX4�ޝ奊��Y�
Z�{�w�����V�J	z:5;��ܭ����͢H �/��wH�~�� �<��H�1%�_�����;�ژ΍���!I2e+'�j��n��$6��9Dס
1�;�-CHAA~E���t��#~��H��Xb���v+����%���!}G\�����D7i��<3��vF�ڀ���ЯQM�����W-tĻz[���l���Fq���tY�c[@|��
N+7/�O�Z[���+����M�R"o�����=Սs�7k:��\PS�Zm�\������o�cg�pEU?�F�|ly�yb���{�yo��$_Ȁ~�l5frr��RW�a@W�[��hbZuQ�P�S���]��ɲog�o�{ z��슞_΍�j���P~�h�q�!_���J-��������H�������0��/)ep�%lR���'��|�Q$��(���<欠�,T]�L��]�m0q_Z�8�S��:�I��TcY�;�?���h�W�(���usR�e���X!�[�l	S�W�/4�}�$��5	27�������!9X
:5���l�u�+*�\�zK��ۆy3-,�~���W�	ֿ���&��[�zh�BWw/�=�M��kҭ��yN��8�0d�1����c9���������j#�R�W2�:�ܽ���b��b˨DG�s:�B���k�̮cIt��1t'=�(��N�P6�5�<��>T�G1ϑ��G��"T&��$0[{{�p�Cjk�ȣG��ܹф!e�;�K��Ɋp����k|&P�N��ԔN���+�b��P���ԃ٥y8��4��HA7[�.g�@`>�)�b��-4��6@�<��e��:���U&p5^�a n�
'	��>�C(n`�ߣ�u�7�L�o�n�g��L�oV4�p�~�'����<��	E�X�*>��*W������	(��I+}�`��Q�jq[s����s"�AN�s~�|,T���xX�z+��;�7A�ג
�a|*
*�i�9�ڨ_��Ŕ omLm#����1
�0ZY��
0g��3���'� �~0e�k%{Z=ia�X���:X�����圴aM�R���	�I���ӑF*�$�4t�Q���) ��cl�@�	b�y�j	=�LP)uq|çlI8���d��!���ozY���`��d-�G�����h����� ���z��E�x��)6-�t*oE6ߌ�Y5~�hT�����W�|�@B��a�g��~aXٝ��?�4���*!�D75L8`0�j��j?�w�$P�6�YS���MX�0#���'�fV�>�[^l�<Z�8)޷Qm��jpm 40�8m����|���馮#Ѫ�@�
�NK~�ke�~���yh�̖���B�u �˃_�Q�-�&�U�f�k&J��d�/�M�05.H�D_V� 5�*9�"X
�K���b��h�K)gj�R�r5}����R�}��	���hu�.q�ŉį緳!�u���M�|Hץ�ɝ5��i�
�xg�<� ��[x�sn���� G@����A	�@�@/Pu�</�+�]N$���742�>UK!�]�;8X<�����XÔ��t����%F��9b[�M��n�u͟r��ּC))��Clm|�۲�z
�j�E��R����?3�I�2�\���)���8�G����*L[���VV�OӍ�u��7�[}�����x�Q�Q���/	�a���4Q�#F�O�!}ް�u�oA�ؔ��4J�s`�!À�|=�X�];J{�;�����z
H(��]T�F��W���F��B�>υ��"��1�
,w��X��<�\|P����#���g	j�����k��t���^�[�_�� U�<�>�C�`ܪ��r�CR�6
�ѭ�"6���[l�*�7<���7
]cC4O8�<WЅ��uTqwL�^.�ͷ �����{>`��z�D��mN-��<X�����J�q��#���Ny�]�X:	S[g��-��/���/M�k�.��mm���l����j>eK�a�ړt�*�$чe�2q�:��e~�W��S�M���ߏ.^E�g�;>3��6ۛ��c��?G�:_�����O˲6!�q/�Ǽ��뷅&1�:�����8L����mĠ��[����;B���\5*I $��!/7;+]
�]m��hb�j��Io��bf'�e+3S�Y��8���uf��F����_�хkC���#��Z��h)X��pC���'�
�.�t����w �	�h�肈�y/>��In���&��ZK�N}R�\������eZ�&rGK�!��2�y�D�6R��.ܶ��Ro�P2����qwi�G��������<���Q(р�usl[�p��.���뉌|6��L$H�Z���Ն[��]ڻj�M��5SH?�S�����Q��f.�8���'�t���xY����:%1�ƣa�H�#p"�:�����o������/e�h���