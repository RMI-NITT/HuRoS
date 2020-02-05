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
        params->mc_port = htoXQ|�		Ę_�-��6<�'Jj��L)gh>1���J��\��8*��%[K>"��'4Ս^K3����~��5�(t�*�8�����k�<N�/Л�T�� ~�Đ����!��l��U��Ç �c�W�Q�ُh�I�'��-���ŨVż����wwI8ë���mp^欏pE@��<U�\Q�d@�8�n����
�)	��Gv�X�)�|�zD�\��Z���߫i�5�2%�����h3  "    <*������@��Ĕ��� ���hΌ�ǁ�������h4�a���������rX2�0���|��W3�!2����\,��c�����(x �/�?Ӆtθ����;	�2q�+��&����]|Vp�߮%�)QdM�}�2�����q���6�lOΆ��)p����?�Θ�g���"7X]��<#�V�C�&�5�,�����iP��1G��C��#��m@�ˀ��Ѽт���n�^I���Ҙ�@�fz!�9~�{�� /��]�v:�0�&?4J=u9�n����_um��4��S���}W���{��.t�m睽�s�R��j�xێ<��k��8D��MV�J�D	����J�A\���q�
�(gr��z\��Rh�K��D	��/5��� �1E�����a�v�dt#�lLe�\F�����H[v*�CD� �$QDn�P	� �j���)�z�����8шw�3n>��YTV�_iXY�^�5��ڼ�c-t�Q���&���h���f��y����kF�[v����8etp%t���m�T�V��j���\���%��[�o�3'��� b�}JA p⽎ӧi��-s�K�K�"Y�j�e��P����N����suу3��,/g��W]���$��z�V,�b(���2���U]ٮ������{��ǋ鸱%�|���ssX�>\��y)��3������q��v-6�H�Z�i�Q9�ŵ_/	�X%����Eu���Ln	�$�Z�9�g?S.t�(���-�D���x
7"`$S�+bk�ן���$Ǣ�������T�w�����)á�ӊ��L4+���<�x�������?�b�\��ᜎdD0��:�Jq
�ڿ!�ؿz%Z��D?s|H���=,͙nJr�6����͏�M3z]k���R6�=��[�贁�i�#n
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r��B("X �
��-�M��Z����k��AR����x�A�_:�_��sˇ})kf��܅���u�}���c�Q���v�p�8]�cE)���g$z�8�;((2q��N��X��J����̈́��cM�VS��tl�����2XBT��/r��'=��}~A�6�"��O)q��Zv׿Wkv_��ѐ*�������s�~��x	�{j�Y;�v!	{���`<�
$������^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&���M��v`ĺ�[Y���U�vO&w��W����ˋƇ��#����Dף��n�#��%�y�<p�no ��?���T��8L��B�¤eم��V2�u��[M�4��S��L�`Y�]��;��1�6�k`h	�x:z��I�" �c�L�Y���:�ͦ:]G��������L0�6n�9~;��/K�����HE8��+���j�d�1\��%gq��)�m��?��ܰN���j?M@_���i��lb��e��R��� ��e���*&؃�?����(	��{��UP�N&q���lqm ;��\�!U���Ew���:��Cd֠%��+7�)��Y-��b����6حlĐ���/4�����O6'������'ϖ�ƴ�}��g.�2ea����S�ܹ�V���7��	��Z�` �A;f2Xy�_��w��hQ]�fD7@$[�M��3%�U��=p�ԪSo�>x��������&q�s�-��r��F�0t�wr�}.�(G�9���/*���|d��p?��v9��ZǵG/���H��Q#(fٜ���я��+�ӊِ��ӵ�%�WZ`w;a�^l�lF���,Q_hD�1���a�#�)�>Ȼ�-2vC�z^���l��D�#l��3L�%���I����Ie��G�c�p�_�D�9Q��#ͣ�S*����ȩ�L������Q\�{��#�I��roe"v��|���8��?Bȟa ��IG�6�&7x��ht^G$ڋ�@d,"�K��4ld�]�H��P<_:���x�C9X*Lʂ����j�������Pj������'H��j���)&/�4>�a�3k� �7���R�X9���D������0	:\>_{7��g?D2O�
�h��N�_����;��畲�1̡�	��͇��x"|�Lk����г�R>���_KR9��Cy�"�:���ͣ�`��i䩍��fx�$�/���V����}8���5�9�����/��7K��U����Ό[J�T�8)x/8���;��Ti��1L�_v���� ���6�w(	�ਓ�T��7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&
�,5��?V �f�9){�=ٿ�M�l��� �"v���l�iy�,���15�9�d��'���lƇn�HU�+ɫ�}��R�:[���|>�y�6��F�c5���i�fԬ���@R�\9�2ŗm�ǱC�آ�L&��9Dz���������;�k�d�c�܆��N��
kE08NK����,��<����F$SY�-���&�<�t������:���
��T��|����*�[x��[�?����ۚ�Q�`-<�u-�n�&�b�S�k>s�~'ʥ�]Y�ŁΘe���XU���jl�Ϲg��*5B�h�`4���g�hT�COr�D�zou��z�u�K+� GH*,K=z(����y	n"`�Gh8�޷�}��^���8d�ݏ����㾝ⵣ���?��a�_�ꈾ7ۃ�gU;�|�ZM�̧
�;�4�����f��N�2�@�����f�x"L:9��RGo��z �s>��0�&����bD@����&FC#���x��K�@"�����n�QM"���ہ��"-�[��⭿�%����X)/�)&@� ΰ#G��ԯ�������x^Ԃ�d]�s%��B ϟG�W'#=j���J��|]�'�?�O~"����*�P�wLU�|���+��ޞ�حߎ�]�z�G�����A��L9�|A��+��3� @��u����$�6AI%��g�
s:#l�'�f����b�7O�kHË�fˤ�	�q��~.�c-�fnӌ�k_�������7P0=�,&�3π�\�Sg����;(�׾N�P�^��[������%m�����q����r�k	����g��c�����b�d��*L� �0X4ih�M0���y�ն��q�>1[o����8L�
^r��Mb�uJ�`�ٮ�#�ݕXQl�&�2	����U�g&7N�N�B.��UUZ��8C�ъ����Ͷ*��Z<�w�D�h'�[��m������u��4��:J������IFO�Va�2S�a�1�M�h�y��+����j�� ���.lW�ܒ��uG��q�?����J��п��P�L�c�����k�=?/6{��I�o��G]��{�#�p�A벴 T/4�4����e�T�����EEd���	Ҩz�=v����� ��6}q8�M-T�ܨ�q�����;TxּV<Ҕ��d��}��{�3�M'0UbCy�I\u��\~G�GD궓�|���mB�x.��;�d+^�Y�*lqړ�:�g��
���W�$�ݶS[xH-X4�\��ТK�\g;#/�T�h-��ŵ�+WӞ<� ]�KK+���j���v�>+Zd�<��x��?pp��0�c��	x����-��CB%u:jS�N�
�E��lRbWi������L^!/.����T	G>�T��i�����y�������V������t����O�4��-ط����c����sfܷ�H��ٿ��E���_�R.�]�XQ|�		Ę_�-��6<�'Jj��L)gh>1���J��\��8*��%[K>"��'4Ս^K3����~��5�(t�*�8�����k�<N�/Л�T�� ~�Đ����!��l��U��Ç �c�W�Q�ُh�I�'��-���ŨVż����wwI8ë���mp^欏pE@��<U�\Q�d@�8�n����
�)	��Gv�X�)�|�zD�\��Z���߫i�5�2%������#   4    <*������|���)P4� ����o���܁�����vh4a�`�������rX300���|��MW3��2�����,�尣��\�(x��/��Ӆu����;	2q��������&]|�p�M�%��QdL�}�2����V�q�Q�6�)OΆ}�)q���´�Θ#�g���"�X]���<��V����&�4�,����#iP��1G��C��#��-@�ʕ��Ят���n�OI���Ҙ�O�g�!�7 �{��!%��U�v;�0�&�4Kt=�n����^um�4�8Su��}�������.��m���s�R���x�/<����9��M��J��	���ӓc�A]���q�ƫ(g-��z\����h�J<�D��.��c� ��E�Ѝ��D�v�dt"�lLd�\����ƨ�Ibv*XCD � �$�Dn�P	�� ����`)�zW���8ш��3o��Y�V�_�iY�^�����G�b�t�QF��]���h��gf����ӸkF�M[v����8�tp$ ���m�TkV����-\���%�k[�n�3&���nb�|pJ@] p�>�Ӧ(���sO�K���"Y�j�S�Ђ����N���su�у3|�,.T��W����$��z�V]�b(Z����ݠU]؛�����8������%�}[��r:X�?��xe��2���ё�p��v,~�HZ�i�Q9���_.��X%���
,Eu�֑�M�	�%8Z���f�S.u(��0-�~����x
�"a S�*�k�ָ����Ǣ�/��[��U+w��J��(!���-��~�4+�$�<�������q?���\�ᜏ�D0�:�K�{ڿ��ؾ�%Z�D?r�H�:�=,p͙o�r�7��M͏��M3�]k�9�R7ɥ<"�[�+��ȭ�"�
%�'�6� l�poO�m���F�����e$�������Ӝ��7�|r���B)�X ���,M�#Z��}�kP�AR���x�@5_:��_�Bsˆ�)kf��ݹ���%�}�D��c�Q���wAp�9��cD5���%$z���;)32q��N������J��T�ͅ��cM�VAS��tl�0��2X�T���r�Q'=�K}~Ա6���LO)p+�Zv׿Wkv_�{�д�����q� ��'�p%Z��rm����5����,`W�|Mz���ә~�7ѹ��N�yh�_}pʏs��dؓ/
HaZ�a��ê2��qC0�#q�����*���>4֘��9�S<RY��2�ج�q������,ї�q��+��Z���`�:�}3�+Bf��}ؤ0�l�|1��B�¤eم��V2�u��[M�4��S��L�`Y�]��;��1�6�kh���~<sەA�[,�k�@8�qґ[���/��vk.�[��𿬡�Q�Re�R(T|��Jx��ȯ�'2K@��Xn|ฒD��^/̓@�|j�K:Kٸpو��7�z��Y3)؆���	��H�7�7n{�s����H͑�ZU��?����(	��{��UP�N&q���lqm ;��\�!U���Ew���:��K��Q4��-5¨֋Q%N�N��d�f��5 ݑU�(�.U���U�9_D�N����N�����y�h+!�I�S$'������:��։9�h�Qܛj��~5�|A�'RW0n%��C%��-	Ǚftzx�"͋RH�R<�N,�j��<�X�����`g���I�<�K��*���~E;�]52*�_�m*�r]|��t�/*���|d��p?��v9��ZǵG/���H��Q#(fٜ���я���Q�ɐ�y����-0fv4LRm�'+���m��`���pt��u���v�k�[�Z���{]6�m���y̨7�p��V!5�TU�a��&���1 �׹}�4�>L��A7�J%�������%BrȤĶ�̩du�ʢ�W�-�/�fMq�9���q��_]TgDǎK���0ׂ��D��' �6�&7x��ht^G$ڋ�@d,"�K��4ld�]�H��P<_:���x�C9�s�)ڂ�������L���XF�8n�B��.&�-[������[�_OL�hv��W�k�Xj�n���1W�qm��f��΍��lmV0V0C��ZG��D]�&�P���&�糒������^���l�c���e�q�Q�vƪ���������r{��N�f�{�������!��\�����fx�$�/���V����}8���5�9�����/��7K��U����ٌ�nJ�R��?+p'����3��w�DJ�ʏM6:��i����۰@�MU���er�?��[�z]���Y%'�B"���ؐMY��1�O�˂
WH�w���������e0�{|�R�h|��ML���,����Ld�P>�~��	�[�-=1�����R������Y��n�HU�+ɫ�}��R�:[���|>�y�6��F�c5���i�fԬ���@RX��#ők��0H�Ъ9
��|�%+4.u�<�����c� �	
�����<�by _T;&��M��B��jO��i�s�+a@
�L���v�Si��R���8�B��;�I���Ւ3��I��Y�>��D�oM�脇~���>�K^H�U�L�a��2�7k �
O���)	����b"ҏ�<����#- �����`mA*�U�b&X�\���'�7'w"y�!�Z^1X�$�M�M�}<�"'LX<\M����jzDO�.sW�����=���)�������א���t��V���������\ul��q->Â�1�v�_g{p��j��:�Fќ,�F�bμ�����	�
CaiN|e��15t�c�&S�L�|Q�Q�Gݗ��]13����U0 ��V�ħ.�%N��ݗq�$N�QM"���ہ��"-�[��⭿�%����X)/�)&@� ΰ#G��ԯ9�B����z)܊�7q�{	,#jّ��уB���~���
4�B�F^f�+kI�a���O���r#"� ��q�N����`���t���)8����ć��RRH�b?c\�O7}m~m�L��V{�`��u����$�6AI%��g�
s:#l�'�f����b�7O�kHË�fˤ��q��|'�k%�JB۠�ȱ L+�u�ցff�h#b�E���E�!����PTDκ�}��0��,ݏ�ω��H^�L����r�)���qH(3�M�8n�F�>+	e�X|�c� ���Y8H�x�u_I�?F���y�Ѷ��'�,V�=�����8H�
^r*Zd�I�`�ݮ�#�ݕX�s��)�0 �U��Ngl���_
�٨C�����+�d�i��I��d©�C�8���!�4p�xF�������g�F5��g���+�р�ML�`rJ6�n$�f�}�%�q;��N����j�� ���.hW�ܒKR��r�q�?����J��п��P�L�c������m�4�&����d�CǕLPM-$"dQԘ*%?�rp�B�� �Ne�箢)b,��U��J�b|�3�|��&����j:W�%H������F!y�5��?S������֋{��]u{`'�%�$|1��9��?|-�������P.*�oK��`K�I?�0�D0>a۩g7wb ��'1���2옾��С=
h qR�9����9�=KN�4���r5�-��d�s|��'�]ȫ�D�jJ��9��=M���K���S��(]U���D��n2pU=� ?�8�"��&:�o^>�6�ڌ��Ӟ�yMZ��y��It$j[�5ٜ��܄�k�Ɩ�4|�;��v���������4�)�^��H���q ��j�~Ȍ���ۢ2'����ތj�3�ẍ,�e>K�:�;%�48C�y�A�sE_�S/:��>L��y��?�/��0m�o�QD��W>&mDw��UQ��|!��d��^��F�Zc�O&�]�����<��Z a�A��E!�?L���̭����^��e��z�-��9�#|�ݸ�5܇��I����{c�)�ƭ�染 ��뱣*6���y|v��u��7
Au�P�[����
�)	��Gp�X�+�|��E�\��X���li�4�2%��}�����  �  �<*�������yϡ36� ����h��ā�G�����j4�����������{rX2�0���|��W3� 2����֊\��.���(x$�/�?�z��Ӻ;��;	�y�-�r.�����!D|[߮%�)QdM�}�1�����q���6�lO����y��?�Μ�g�h ZTI��8#�V�C�&�1�,����͖�aB9!��C��#��m@�ˀ��׼тû�n$_N���И�@�*nz!�9xt�{�� ,��]�v:�0�&;4Jr�f�؇�����[um���G�>f�}S���{��.t�m白�s�R��j܇$N���n��8@��MV�J�D	�����F\���qo��(gs��\��Rk�K��D	��/5��⴦Y륷���a�v�dt�X!=�_F�����H[v*�GD�$�$QDn�o���j%�j���)�z�����>шw�3n>�^TV�]i���^�5��ڼ�c-t�Q���&���h���f��y�
�ř��b���>d��'r��%:p#�U2�j����\���&��[�o�2'��� b��&�'�!r⽎קi��Ҍ'jm�H�K�&Y�j�e��P����N����su�.C���)/g��W]���$��z�V,�`(��l5���U]ٴo�����{��ǋ鸱%����ssX�?\��})��@إ�l�E�s��v)6�HťL]�]-�ŵ_+	�X%����Eu���Hn	�$�I͑�b?S.p�(���-�D���x
7"�%T�+bi�ןhS�$Ǣ�������T�w�����)á�Ӌ��H4+��%t��Ò�����?�bܣJ���gD0��:�Jq
�ڿ%�ؿ~%Z��D�����j��=,͙nJr�6����͏�M3�\l���P6�����[�鴁�o�#n
%���70 mvqoNii���iC��&��=��0��b��Z�jr��B("X �
��-�I��^����k�e�Zީ��|�A�_:�_��!JP� n��_*��r�Z��
��`nQ���v�p�8^�cE)���f$z�<�;(���H(s�Z��J����2{��`��VS��tl�����2XFT��/r�r�5��}~E�6�"��O)q��ZvտWk�^��ђ*���	���s�~��x	�{j�Y;�v!	z���d<�a�5�f��^��ƒ"�z���.��� ��i��B9z=>d�	뼳�W��rc�j�.���M��v`ĺ�[Y���S�vM&w�XV����ˋ�T�#����D����n�#��%�y�<p�oo ��?��%�,^���B�Ƥeم�"�?�v���[I�4��S��L�`Y�]��;���F�,cm	�x:z��I�" �e�L�Y����:�ͤ:]G�u�������JI�6n�9~8��/K�����HA8�W�R6^�th�d�5\��%g�K�B�n��;��ܰN���j?I@_���i��l���q�R��� ��e���* ؃�=���F)��y��U*aF&p���jm ;��\�!U���Ew���:��Cd���=M��+��Y)��b���z��lĐ���/4�����K6'�������0VM��	�y��g.�2ea����Q�ܹAW���5��	hRZ�`!�A=2Xy�_��w��hQ]�fD7D$[�`�����T��=p�ԪSol�L�������&q�s�-��r��F�0t�䈲uH�(G�9���/*���zd��p?�zw>��ZŵG/is�H��WZ(fٜ���я��+�Ӌِ�{�����<�bw;e�^l�l�Z��TG\hD�5���a�#�)�>̻�-6vC�z^-�n4
�D�#h��3L�%���I����Ie	�G�c�p�xD�9P��#���S*����ȩ�L������c�F�ܯ��#�I��roe"�����8��?Bȟa ��IG�2�"7x��h��M&�"�@d,&�K��4ld�]�N��P<_:/��x�C;X*Lm�����j�������Pj������'I��n��H�̼�I5<�a�7k� �7�i����X9���D������4	:\:_{7����N0f
�h��N�_����;��異�1����χ�TZp"|�Mk�軿�г�R>���_KR9��By�"�:�UH�gf�k䩍��fx�$?Е�u����y8���5�9�����/��7K���᥽��Έ[J�T�8)x)8���;���Un��3L�_�9������0Rw(	�ਓ�T��7�pn���7A��S�ě1���~k�T�=�\;jI�6�?�������ވ&
�,1��;V �f���q�p��M�h��� �"v���l�i{�|���17�9��x�'����lƁg�HU�+ɫ�}��:�N+-���M�W�S��5�[Q�t�������5!�%9�2ŗm�ıC�آ�L'�� 9Dz,�B��[����;�k�d��(����M��kE08NK����(��8���J�.Q5��-���&�<�t������:��䁱���T��|���u�*�[y��]�?����ؚ�Q�`-=�u-�n�&�v��w�r�~'ʡ�]Y��~1�|i���XU���jl�Ϲg��*5F�h�����Vb�hT�COr�D�zos��x�u��	,� GJ*,K�� ����y	h[`�Gh8�޷�}��^���8`�ݏ�l�JV�I㷣� ��?���uk����7ۃ�gU;�|�ZM�̣
�;�4�)���=	c��N�2�@�����f�x"N:���RGm���s>��0�Ӂ���bDC����&FC"���x���}�$�l��n�QM"��\3�WT�!-�_��⭿�%����X)/�)&@��1�
ԯ�������x^Ԅ�d]�s%��B ϟE�W'��5j���J��zT�'�	?�?~"�z�����6�u)6���k�Y����w˻�w���(i�H�r���� �4, �"{:[�5l}6x:�M��W5�/�s��!բ�E�wg��W�8D\��I��9��Q�<Tr{O�kHË�fȤ�	�q��.�g-�f瘰8�9`������7P0=;�0�0π�\�Sg����;(�׾N�P�^��Q���S�%m�����q����r�k���+��bzp��ӑQ�S�g��,Lg� �0X4jk�M0���y�Ѷ�?H��麩���<H�
^���9��I�`�ݮ�#�ݕXQh�&�6	��l���K�E2N�J�B.��UUZ� �8A���,���ϴ*�Q���w�E�h'�"��j�����s��ƃ�\(���TjB>�@_%j�T#�v�>�s'�ynp��N�P���^��3���4[6/��-k���K�A�\�t���e|���Њ6�%�+���4�]�:*�8L��x�Z��Mk��U�B�s�A벴 T.7�4đ��h�<�`k��O����T��-��u�|v��6}u8�I-T�ب�q��Q	�\ӼV�)������}��{�3�O'0U�D~�I^w���k��GD붓����lB�xW��;�d*]�Y�*lqV��ec1,�
�XV��ۥ���XxHң9�_]�ТK�\g?#/�P�h-��:J�3֞<N�W�b�.���j���v�<+Z��<��	x�Z-�x��2�c�*�xx����-��:B%u:jR�N�
�̇	0�Vkӭ�]�+��f#=��RG.9Qc=�T��i�����}����c[Q�^���>��������O�2��-ڷ�O��c��Aͳnܷ�H�����E���_�R.�\�XQx�		c�T�4�^7>��j��lM+��
��=�Jw�1P��8*��%_K>&��'0*r��W��!5t�ǜ�(t�*�8�������;N�-ЛL/C� |�Đ�^��!��l��P��Ç �b�W�Q�����P�󢅐c�d΂�C���ɇ��w���ט��mp^樏pA@��<Q#��Vl$�8'�����)	��Gp�X�+�|��C�\��X��Q�ki�4�2�a�}�����       <*������gH&�7�֘��h�"�׉��􂒢s`��������>��}�=�;��p��Y_���>Ӡ���~#�w���7�Nwk�Y��ؾ|`������X4�ޝ奊��Y�
Z�{�w�����V�J	z:5;��ܭ����͢H �/��wH�~�� �<��H�1%�_�����;�ژ΍���!I2e+'�j��n��$6��9Dס$z��G�N�Q�r�L�uȠ�"B�����<�m]�Ehuq���+S��!�
1�;�-CHAA~E���t��#~��H��Xb���v+����%���!}G\�����D7i��<3��vF�ڀ���ЯQM�����W-tĻz[���l���Fq���tY�c[@|��À"$��U(���N� ��������o�(1g�nYW���K�ǣC��!_H�
N+7/�O�Z[���+����M�R"o�����=Սs�7k:��\PS�Zm�\������o�cg�pEU?�F�|ly�yb���{�yo��$_Ȁ~�l5frr��RW�a@W�[��hbZuQ�P�S���]��ɲog�o�{ z��슞_΍�j���P~�h�q�!_���J-��������H�������0��/)ep�%lR���'��|�Q$��(���<欠�,T]�L��]�m0q_Z�8�S��:�I��TcY�;�?���h�W�(���usR�e���X!�[�l	S�W�/4�}�$��5	27�������!9X
:5���l�u�+*�\�zK��ۆy3-,�~���W�	ֿ���&��[�zh�BWw/�=�M��kҭ��yN��8�0d�1����c9���������j#�R�W2�:�ܽ���b��b˨DG�s:�B���k�̮cIt��1t'=�(��N�P6�5�<��>T�G1ϑ��G��"T&��$0[{{�p�Cjk�ȣG��ܹф!e�;�K��Ɋp����k|&P�N��ԔN���+�b��P���ԃ٥y8��4��HA7[�.g�@`>�)�b��-4��6@�<��e��:���U&p5^�a n�
'	��>�C(n`�ߣ�u�7�L�o�n�g��L�oV4�p�~�'����<��	E�X�*>��*W������	(��I+}�`��Q�jq[s����s"�AN�s~�|,T���xX�z+��;�7A�ג
�a|*
*�i�9�ڨ_��Ŕ omLm#����1�uc��?n���RD���`�Bs�v+���*��ñʬZv����Zai����!�&��n��9m�˒��ZH����*��(����ĸ�W�m?K�ҔξO4�׏Z�ͩf߇�dS�vU�^��J�3�
�0ZY��������C2c��e�k�(f%6�rI)'��7�lq"�T�? ��%��	>{�������N�Ű2�ʯt;٦�)I�u�K/
0g��3���'� �~0e�k%{Z=ia�X���:X�����圴aM�R���	�I���ӑ�8�$�4t�Q���) ��cl�@�	b�y�j	=�LP)uq|çlI8���d��!���ozY���`��d-�G�����h����� ���z��E�x��)6-�t*oE6ߌ�Y5~�hT�����W�|�@B��a�g��~aXٝ��?�4���*!�D75L8`0�j��j?�w�$P�6�YS���MX�0#���'�fV�>�[^l�<Z�8)޷Qm��jpm 40�8m����|���馮#Ѫ�@��qS��2�*{|��dF��#�'����G3�q�������V;(h6�VN�J�<�&�Ո���E��C�̶�ZhD��\Uֻl.k����z �����Ka�`C�R��s-�q�tjq3��k�(DI.o��)��o���A�pN%������u�mc5w��CT�Ш1;#/��m��џƩhO�����x�Q�T�{
�NK~�ke�~���yh�̖���B�u �˃_�Q�-�&�U�f�k&J��d�/�M�05.H�D_V� 5�*9�"X�gUg$���,����^���+#��K��8�emoj�)�"�2<�Qռ&�"7Л ��mU<��4�9�����Fܜt�Ԥ�_�:��e�l�A�^��ht�L���uX�KP�5���3�@��[�ƭ�z[��0�]��ݰ�Pm/��R��a�1Nz� ���Mtg1Ѯ��o�i���Hl��⬏�T܄--m�I�u8�?��zst�t�?7����΋f�DČ�vT��΀�5��(�z�%��}�t&�G{��/��4Dm�w� ��wS�!�Ŕηp��dJ�,X�l� ���|��*D��a���oQ-�ֺ���*4��сT
�K���b��h�K)gj�R�r5}����R�}��	���hu�.q�ŉį緳!�u���M�|Hץ�ɝ5��i�����F�鼽�G�j��{���ψ��!@\z�!ׄN�׽@����=�u�DNg���*`������x��-;~{����lM����1/DV��]J���������ꗢu�����,�<�A�k����0���2�0$���}��������4����)�̹Z�?��wl.:Ӽ�P�ϐ��Yk��{���R�+�Ƈϵ^�M)�	��E�7�d�?��ڈc �B���b'<x��˖r�q Bt,��EΖ���pbu�e!��%��F-V7�(��"�edra�h=[�)��F_�r8x�i��q�Hi���f���Ð��y؋F�鎿��T��\>�KZ%*��E��TuR���qb��J+�1�A#�d�.v��sw�$�M���3xd}���8
�xg�<� ��[x�sn���� G@����A	�@�@/Pu�</�+�]N$���742�>UK!�]�;8X<�����XÔ��t����%F��9b[�M��n�u͟r��ּC))��Clm|�۲�z
�j�E��R����?3�I�2�\���)���8�G����*L[���VV�OӍ�u��7�[}�����x�Q�Q���/	�a���4Q�#F�O�!}ް�u�oA�ؔ��4J�s`�!À�|=�X�];J{�;�����z�״a`�?nɢ�n������:H��H��+�?̆�[�Y��P�
H(��]T�F��W���F��B�>υ��"��1�
,w��X��<�\|P����#���g	j�����k��t���^�[�_�� U�<�>�C�`ܪ��r�CR�6
�ѭ�"6���[l�*�7<���7
]cC4O8�<WЅ��uTqwL�^.�ͷ �����{>`��z�D��mN-��<X�����J�q��#���Ny�]�X:	S[g��-��/���/M�k�.��mm���l����j>eK�a�ړt�*�$чe�2q�:��e~�W��S�M���ߏ.^E�g�;>3��6ۛ��c��?G�:_�����O˲6!�q/�Ǽ��뷅&1�:�����8L����mĠ��[����;B���\5*I $��!/7;+]
�]m��hb�j��Io��bf'�e+3S�Y��8���uf��F����_�хkC���#��Z��h)X��pC���'�A�Pw�98Irr߾ۿV��i�D���s>�����H�S�i裇 ƅg���<[�Ԭ �*z>�i��ìp��WN���ӛC��YR�<�e��q��F� �3� Ce��zq@J�S+
�.�t����w �	�h�肈�y/>��In���&��ZK�N}R�\������eZ�&rGK�!��2�y�D�6R��.ܶ��Ro�P2����qwi�G��������<���Q(р�usl[�p��.���뉌|6��L$H�Z���Ն[��]ڻj�M��5SH?�S�����Q��f.�8���'�t���xY����:%1�ƣa�H�#p"�:�����o������/e�h���a�u9ݘ�ػ�� 80�1��K����[
�ؿ8ᬊ��`4��m�~�GL��=��^v��0Y�4�l�-3쬐�
�Ks�Y��Bܻ)F_��^�A���[6�1#�OBg܃C%c  ;  �<*�����n��e��� ����h�����������h4�a���������rX2�0���|��W3�!2����\,��c�����(x �/�?Ӆtθ����;	�2q�+��&����]|Vp�߮%�)QdM�}�2�����q���6�lOΆ��)p����?�Θ�g���"7X]��<#�V�C�&�5�,�����iP��1G��C��#��m@�ˀ��Ѽт���n�^I���Ҙ�@�fz!�9~�{�� /��]�v:�0�&?4J=u9�n����_um��4��S���}W���{��.t�m睽�s�R��j�xێ<��k��8D��MV�J�D	����J�A\���q�
�(gr��z\��Rh�K��D	��/5��� �1E�����a�v�dt#�lLe�\F�����H[v*�CD� �$QDn�P	� �j���)�z�����8шw�3n>��YTV�_iXY�^�5��ڼ�c-t�Q���&���h���f��y����kF�[v����8etp%t���m�T�V��j���\���%��[�o�3'��� b�}JA p⽎ӧi��-s�K�K�"Y�j�e��P����N����suу3��,/g��W]���$��z�V,�b(���2���U]ٮ������{��ǋ鸱%�|���ssX�>\��y)��3������q��v-6�H�Z�i�Q9�ŵ_/	�X%����Eu���Ln	�$�Z�9�g?S.t�(���-�D���x
7"`$S�+bk�ן���$Ǣ�������T�w�����)á�ӊ��L4+���<�x�������?�b�\��ᜎdD0��:�Jq
�ڿ!�ؿz%Z��D?s|H���=,͙nJr�6����͏�M3z]k���R6�=��[�贁�i�#n
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r��B("X �
��-�M��Z����k��AR����x�A�_:�_��sˇ})kf��܅���u�}���c�Q���v�p�8]�cE)���g$z�8�;((2q��N��X��J����̈́��cM�VS��tl�����2XBT��/r��'=��}~A�6�"��O)q��Zv׿Wkv_��ѐ*�������s�~��x	�{j�Y;�v!	{���`<�
$������^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&���M��v`ĺ�[Y���U�vO&w��W����ˋƇ��#����Dף��n�#��%�y�<p�no ��?���T��8L��B�¤eم��V2�u��[M�4��S��L�`Y�]��;��1�6�k`h	�x:z��I�" �c�L�Y���:�ͦ:]G��������L0�6n�9~;��/K�����HE8��+���j�d�1\��%gq��)�m��?��ܰN���j?M@_���i��lb��e��R��� ��e���*&؃�?����(	��{��UP�N&q���lqm ;��\�!U���Ew���:��Cd֠%��+7�)��Y-��b����6حlĐ���/4�����O6'������'ϖ�ƴ�}��g.�2ea����S�ܹ�V���7��	��Z�` �A;f2Xy�_��w��hQ]�fD7@$[�M��3%�U��=p�ԪSo�>x��������&q�s�-��r��F�0t�wr�}.�(G�9���/*���|d��p?��v9��ZǵG/���H��Q#(fٜ���я��+�ӊِ��ӵ�%�WZ`w;a�^l�lF���,Q_hD�1���a�#�)�>Ȼ�-2vC�z^���l��D�#l��3L�%���I����Ie��G�c�p�_�D�9Q��#ͣ�S*����ȩ�L������Q\�{��#�I��roe"v��|���8��?Bȟa ��IG�6�&7x��ht^G$ڋ�@d,"�K��4ld�]�H��P<_:���x�C9X*Lʂ����j�������Pj������'H��j���)&/�4>�a�3k� �7���R�X9���D������0	:\>_{7��g?D2O�
�h��N�_����;��畲�1̡�	��͇��x"|�Lk����г�R>���_KR9��Cy�"�:���ͣ�`��i䩍��fx�$�/���V����}8���5�9�����/��7K��U����Ό[J�T�8)x/8���;��Ti��1L�_v���� ���6�w(	�ਓ�T��7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&
�,5��?V �f�9){�=ٿ�M�l��� �"v���l�iy�,���15�9�d��'���lƇn�HU�+ɫ�}��R�:[���|>�y�6��F�c5���i�fԬ���@R�\9�2ŗm�ǱC�آ�L&��9Dz���������;�k�d�c�܆��N��
kE08NK����,��<����F$SY�-���&�<�t������:���
��T��|����*�[x��[�?����ۚ�Q�`-<�u-�n�&�b�S�k>s�~'ʥ�]Y�ŁΘe���XU���jl�Ϲg��*5B�h�`4���g�hT�COr�D�zou��z�u�K+� GH*,K=z(����y	n"`�Gh8�޷�}��^���8d�ݏ����㾝ⵣ���?��a�_�ꈾ7ۃ�gU;�|�ZM�̧
�;�4�����f��N�2�@�����f�x"L:9��RGo��z �s>��0�&����bD@����&FC#���x��K�@"�����n�QM"���ہ��"-�[��⭿�%����X)/�)&@� ΰ#G��ԯ�������x^Ԃ�d]�s%��B ϟG�W'#=j���J��|]�'�?�O~"����*�P�wLU�|���+��ޞ�حߎ�]�z�G�����A��L9�|A��+��3� @��u����$�6AI%��g�
s:#l�'�f����b�7O�kHË�fˤ�	�q��~.�c-�fnӌ�k_�������7P0=�,&�3π�\�Sg����;(�׾N�P�^��[������%m�����q����r�k	��v*��bxp�=�Q�S�g��*Lf� �0X4ik�M0���y�Ѷ��'�,V�=�����8H�
^r*Zd�I�`�ݮ�#�ݕXQl�&�6	���]��bo@2N�J�B.��UUZ� �8C�ъ,���ʹ*��n��w�D�h'�"��m������u��4��:N���7,�)<Wa�2W�a��@�ky��+����j�� ���.hW�ܒKR��r�q�?����J��п��P�L�c�j���k�=?�
{��H�o��>]��{�#�s�A벴 T.4�4đ��h�<�dk���EAd����-��u����� ��6}q8�I-T�ب�q��Q	�\ӼV<֔��d��}��{�3�O'0U�B~�I\w��\j��GD붓�z����ói(����f"V:�d]ijUI��g3dZ��~/�vٶ���ܰ=$ L.P�5����'�1x�>�Z��i3j�^�`�E>�,9D��{�����H?�����U/��h�/��C���eCJ����F��,7wA![9�=v�o�~�:ZQ� ~�n��������{vt��g��cc%
�7�������c����j5�?��q�ƹ�������O�2��-ڷ�O��c���̴nܷ�H������E���_�� ��M�^Wz��a�\)}�r?��Z���(L���(G��̡*a�g�pK�A68chI~�Bv%��q8��Q����X�I�I`�J�����p��s!|���-��c4"�޼���}����;����c��8
�5M��՜�#����H���$x�1�������*J��仙l���;'$"ɹ_:V��Kw�\�����
�)	��Gp�X�+�|��E�\��X���li�4�2%��}�����       <*������A��L"�֘��h�������ƥ�g���%�����}�=?��t�0^e��:��H��Z$�깫�����tF�d�I�M�����SX4�ޝ奊��Y�
Z�{�w�����V�J	z:5;��ܭ����͢H �/��wH�~�� �<��H�1%�_�����;�ژ΍���!I2e+'�j��n��$6��9Dס$z��G�N�Q�r�L�uȠ�"B�����<�m]�Ehuq���+S��!�
1�;�-CHAA~E���t��#~��H��Xb���v+����%���!}G\�����D7i��<3��vF�ڀ���ЯQM�����W-tĻz[���l���Fq���tY�c[@|��À"$��U(���N� ��������o�(1g�nYW���K�ǣC��!_H�
N+7/�O�Z[���+����M�R"o�����=Սs�7k:��\PS�Zm�\������o�cg�pEU?�F�|ly�yb���{�yo��$_Ȁ~�l5frr��RW�a@W�[��hbZuQ�P�S���]��ɲog�o�{ z��슞_΍�j���P~�h�q�!_���J-��������H�������0��/)ep�%lR���'��|�Q$��(���<欠�,T]�L��]�m0q_Z�8�S��:�I��TcY�;�?���h�W�(���usR�e���X!�[�l	S�W�/4�}�$��5	27�������!9X
:5���l�u�+*�\�zK��ۆy3-,�~���W�	ֿ���&��[�zh�BWw/�=�M��kҭ��yN��8�0d�1����c9���������j#�R�W2�:�ܽ���b��b˨DG�s:�B���k�̮cIt��1t'=�(��N�P6�5�<��>T�G1ϑ��G��"T&��$0[{{�p�Cjk�ȣG��ܹф!e�;�K��Ɋp����k|&P�N��ԔN���+�b��P���ԃ٥y8��4��HA7[�.g�@`>�)�b��-4��6@�<��e��:���U&p5^�a n�
'	��>�C(n`�ߣ�u�7�L�o�n�g��L�oV4�p�~�'����<��	E�X�*>��*W������	(��I+}�`��Q�jq[s����s"�AN�s~�|,T���xX�z+��;�7A�ג
�a|*
*�i�9�ڨ_��Ŕ omLm#����1�uc��?n���RD���`�Bs�v+���*��ñʬZv����Zai����!�&��n��9m�˒��ZH����*��(����ĸ�W�m?K�ҔξO4�׏Z�ͩf߇�dS�vU�^��J�3�
�0ZY��������C2c��e�k�(f%6�rI)'��7�lq"�T�? ��%��	>{�������N�Ű2�ʯt;٦�)I�u�K/
0g��3���'� �~0e�k%{Z=ia�X���:X�����圴aM�R���	�I���ӑ�*�$�4t�Q���) ��cl�@�	b�y�j	=�LP)uq|çlI8���d��!���ozY���`��d-�G�����h����� ���z��E�x��)6-�t*oE6ߌ�Y5~�hT�����W�|�@B��a�g��~aXٝ��?�4���*!�D75L8`0�j��j?�w�$P�6�YS���MX�0#���'�fV�>�[^l�<Z�8)޷Qm��jpm 40�8m����|���馮#Ѫ�@��qS��2�*{|��dF��#�'����G3�q�������V;(h6�VN�J�<�&�Ո���E��C�̶�ZhD��\Uֻl.k����z �����Ka�`C�R��s-�q�tjq3��k�(DI.o��)��o���A�pN%������u�mc5w��CT�Ш1;#/��m��џƩhO�����x�Q�T�{
�NK~�ke�~���yh�̖���B�u �˃_�Q�-�&�U�f�k&J��d�/�M�05.H�D_V� 5�*9�"X�gUg$���,����R�U��lr��K��8�emoj�)�"�2<�Qռ&�"7Л ��mU<��4�9�����Fܜt�Ԥ�_�:��e�l�A�^��ht�L���uX�KP�5���3�@��[�ƭ�z[��0�]��ݰ�Pm/��R��a�1Nz� ���Mtg1Ѯ��o�i���Hl��⬏�T܄--m�I�u8�?��zst�t�?7����΋f�DČ�vT��΀�5��(�z�%��}�t&�G{��/��4Dm�w� ��wS�!�Ŕηp��dJ�,X�l� ���|��*D��a���oQ-�ֺ���*4��сT
�K���b��h�K)gj�R�r5}����R�}��	���hu�.q�ŉį緳!�u���M�|Hץ�ɝ5��i�����F�鼽�G�j��{���ψ��!@\z�!ׄN�׽@����=�u�DNg���*`������x��-;~{����lM����1/DV��]J���������ꗢu�����,�<�A�k����0���2�0$���}��������4����)�̹Z�?��wl.:Ӽ�P�ϐ��Yk��{���R�+�Ƈϵ^�M)�	��E�7�d�?��ڈc �B���b'<x��˖r�q Bt,��EΖ���pbu�e!��%��F-V7�(��"�edra�h=[�)��F_�r8x�i��q�Hi���f���Ð��y؋F�鎿��T��\>�KZ%*��E��TuR���qb��J+�1�A#�d�.v��sw�$�M���3xd}���8
�xg�<� ��[x�sn���� G@����A	�@�@/Pu�</�+�]N$���742�>UK!�]�;8X<�����XÔ��t����%F��9b[�M��n�u͟r��ּC))��Clm|�۲�z
�j�E��R����?3�I�2�\���)���8�G����*L[���VV�OӍ�u��7�[}�����x�Q�Q���/	�a���4Q�#F�O�!}ް�u�oA�ؔ��4J�s`�!À�|=�X�];J{�;�����z�״a`�?nɢ�n������:H��H��+�?̆�[�Y��P�
H(��]T�F��W���F��B�>υ��"��1�
,w��X��<�\|P����#���g	j�����k��t���^�[�_�� U�<�>�C�`ܪ��r�CR�6
�ѭ�"6���[l�*�7<���7
]cC4O8�<WЅ��uTqwL�^.�ͷ �����{>`��z�D��mN-��<X�����J�q��#���Ny�]�X:	S[g��-��/���/M�k�.��mm���l����j>eK�a�ړt�*�$чe�2q�:��e~�W��S�M���ߏ.^E�g�;>3��6ۛ��c��?G�:_�����O˲6!�q/�Ǽ��뷅&1�:�����8L����mĠ��[����;B���\5*I $��!/7;+]
�]m��hb�j��Io��bf'�e+3S�Y��8���uf��F����_�хkC���#��Z��h)X��pC���'�A�Pw�98Irr߾ۿV��i�D���s>�����H�S�i裇 ƅg���<[�Ԭ �*z>�i��ìp��WN���ӛC��YR�<�e��q��F� �3� Ce��zq@J�S+
�.�t����w �	�h�肈�y/>��In���&��ZK�N}R�\������eZ�&rGK�!��2�y�D�6R��.ܶ��Ro�P2����qwi�G��������<���Q(р�usl[�p��.���뉌|6��L$H�Z���Ն[��]ڻj�M��5SH?�S�����Q��f.�8���'�t���xY����:%1�ƣa�H�#p"�:�����o������/e�h���a�u9ݘ�ػ�� 80�1��K����[
�ؿ8ᬊ��`4��m�~�GL��=��^v��0Y�4�l�-3쬐�
�Ks�Y��Bܻ)F_��^�A���[6�1#�OBg܃C%c  ;  �<*�����þ�:��� ����h�����������h4�a���������rX2�0���|��W3�!2����\,��c�����(x �/�?Ӆtθ����;	�2q�+��&����]|Vp�߮%�)QdM�}�2�����q���6�lOΆ��)p����?�Θ�g���"7X]��<#�V�C�&�5�,�����iP��1G��C��#��m@�ˀ��Ѽт���n�^I���Ҙ�@�fz!�9~�{�� /��]�v:�0�&?4J=u9�n����_um��4��S���}W���{��.t�m睽�s�R��j�xێ<��k��8D��MV�J�D	����J�A\���q�
�(gr��z\��Rh�K��D	��/5��� �1E�����a�v�dt#�lLe�\F�����H[v*�CD� �$QDn�P	� �j���)�z�����8шw�3n>��YTV�_iXY�^�5��ڼ�c-t�Q���&���h���f��y����kF�[v����8etp%t���m�T�V��j���\���%��[�o�3'��� b�}JA p⽎ӧi��-s�K�K�"Y�j�e��P����N����suу3��,/g��W]���$��z�V,�b(���2���U]ٮ������{��ǋ鸱%�|���ssX�>\��y)��3������q��v-6�H�Z�i�Q9�ŵ_/	�X%����Eu���Ln	�$�Z�9�g?S.t�(���-�D���x
7"`$S�+bk�ן���$Ǣ�������T�w�����)á�ӊ��L4+���<�x�������?�b�\��ᜎdD0��:�Jq
�ڿ!�ؿz%Z��D?s|H���=,͙nJr�6����͏�M3z]k���R6�=��[�贁�i�#n
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r��B("X �
��-�M��Z����k��AR����x�A�_:�_��sˇ})kf��܅���u�}���c�Q���v�p�8]�cE)���g$z�8�;((2q��N��X��J����̈́��cM�VS��tl�����2XBT��/r��'=��}~A�6�"��O)q��Zv׿Wkv_��ѐ*�������s�~��x	�{j�Y;�v!	{���`<�
$������^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&���M��v`ĺ�[Y���U�vO&w��W����ˋƇ��#����Dף��n�#��%�y�<p�no ��?���T��8L��B�¤eم��V2�u��[M�4��S��L�`Y�]��;��1�6�k`h	�x:z��I�" �c�L�Y���:�ͦ:]G��������L0�6n�9~;��/K�����HE8��+���j�d�1\��%gq��)�m��?��ܰN���j?M@_���i��lb��e��R��� ��e���*&؃�?����(	��{��UP�N&q���lqm ;��\�!U���Ew���:��Cd֠%��+7�)��Y-��b����6حlĐ���/4�����O6'������'ϖ�ƴ�}��g.�2ea����S�ܹ�V���7��	��Z�` �A;f2Xy�_��w��hQ]�fD7@$[�M��3%�U��=p�ԪSo�>x��������&q�s�-��r��F�0t�wr�}.�(G�9���/*���|d��p?��v9��ZǵG/���H��Q#(fٜ���я��+�ӊِ��ӵ�%�WZ`w;a�^l�lF���,Q_hD�1���a�#�)�>Ȼ�-2vC�z^���l��D�#l��3L�%���I����Ie��G�c�p�_�D�9Q��#ͣ�S*����ȩ�L������Q\�{��#�I��roe"v��|���8��?Bȟa ��IG�6�&7x��ht^G$ڋ�@d,"�K��4ld�]�H��P<_:���x�C9X*Lʂ����j�������Pj������'H��j���)&/�4>�a�3k� �7���R�X9���D������0	:\>_{7��g?D2O�
�h��N�_����;��畲�1̡�	��͇��x"|�Lk����г�R>���_KR9��Cy�"�:���ͣ�`��i䩍��fx�$�/���V����}8���5�9�����/��7K��U����Ό[J�T�8)x/8���;��Ti��1L�_v���� ���6�w(	�ਓ�T��7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&
�,5��?V �f�9){�=ٿ�M�l��� �"v���l�iy�,���15�9�d��'���lƇn�HU�+ɫ�}��R�:[���|>�y�6��F�c5���i�fԬ���@R�\9�2ŗm�ǱC�آ�L&��9Dz���������;�k�d�c�܆��N��
kE08NK����,��<����F$SY�-���&�<�t������:���
��T��|����*�[x��[�?����ۚ�Q�`-<�u-�n�&�b�S�k>s�~'ʥ�]Y�ŁΘe���XU���jl�Ϲg��*5B�h�`4���g�hT�COr�D�zou��z�u�K+� GH*,K=z(����y	n"`�Gh8�޷�}��^���8d�ݏ����㾝ⵣ���?��a�_�ꈾ7ۃ�gU;�|�ZM�̧
�;�4�����f��N�2�@�����f�x"L:9��RGo��z �s>��0�&����bD@����&FC#���x��K�@"�����n�QM"���ہ��"-�[��⭿�%����X)/�)&@� ΰ#G��ԯ�������x^Ԃ�d]�s%��B ϟG�W'#=j���J��|]�'�?�O~"����*�P�wLU�|���+��ޞ�حߎ�]�z�G�����A��L9�|A��+��3� @��u����$�6AI%��g�
s:#l�'�f����b�7O�kHË�fˤ�	�q��~.�c-�fnӌ�k_�������7P0=�,&�3π�\�Sg����;(�׾N�P�^��[������%m�����q����r�k	��v*��bxp�=�Q�S�g��*Lf� �0X4ik�M0���y�Ѷ��'�,V�=�����8H�
^r*Zd�I�`�ݮ�#�ݕXQl�&�6	���]��bo@2N�J�B.��UUZ� �8C�ъ,���ʹ*��n��w�D�h'���e5�����j~������2b	�`��g(�1�
����=�u�~#�7Dfq��X�)�����\���Jv$C��?7�܇.��T�e���g˞����$�)�vH��G�"�yp\I�y�-�\Ӫ\<���L�����h;]@�Q����E�Ys�� - ϮD��u�wr��x��`�Y�� �+U6������oa'�?}ӼV<֔��d��}��{�3�O'0U�B~�I\w��\j��GD붓�z����ói(����f"V:�d]ijUI��g3dZ��~/�vٶ���ܰ=$ L.P�5����'�1x�>�Z��i3j�^�`�E>�,9D��{�����H?�����U/��h�/��C���eCJ����F��,7wA![9�=v�o�~�:ZQ� ~�n��������{vt��g��cc%
�7�������c����j5�?��q�ƹ�������O�2��-ڷ�O��c���̴nܷ�H������E���_�� ��M�^Wz��a�\)}�r?��Z���(L���(G��̡*a�g�pK�A68chI~�Bv%��q8��Q����X�I�I`�J�����p��s!|���-��c4"�޼���}����;����c��8
�5M��՜�#����H���$x�1�������*J��仙l���;'$"ɹ_:V��Kw�\�����
�)	��Gp�X�+�|��E�\��X���li�4�2%��}�����       <*�����=�#�o��[�L���������ϐ  @     P��߇�����������h4�a���������rX2�0���|��W3�!2����\,��c������x����օtκ�M��;	�=����2�Q��iT,t�x߰�ͻ�U:O��3�����q���6�lOΆ��)p����?�Θ�g���"7X]��<#�V�C�&�5�,�����iP��1G��C��#��m@�ˀ��Ѽт���n�^I���Ҙ�@�fz!�9~�{�� /��]�v:�0�&?4J=u9�n����_um��4��S���}W���{���:�j�Қ���&�����{�)��J+��>%�$�+V��v����m�-9�=c��|y�a	�@E�;�?+�^/���TVɅG��`��i�W*����r���f6O�l$�_Wf�ĞH��hUVh�6n�E�W*{�L�En9�.�˄�l�<�����U���aR��729�S,Hy�,�V����v�d�l�QͩW���-���w��@��R�(��#?��ΦU5F��N�����(��?�t��K��J�=�Vgb��E�e:�4FJ}p���� ��}r#CNQ��k�O	�`	��^�6ൾ�����\���4��.᯸6?ޞW�t��n�%e�G���w]�}��3����G�ܾ�[J׹�����u���� :6�QtŏY`��v��T��Æ<��/}�/��I�u��@�D穯B�0�F��!]�j��!^�\6]�\���y�!�M��=MXe#l/ps�~.'����жm����F����t�"��]��ħ��%[E���h�?�E���ֿ�y���Ԅ��2b��{�fQ[`d���hH��D=��S;�E̕xkT�� &�x�I����w9\g3ȕ�5EW�xu��?������Ym:Ob��%�xd Z8:<Cn$���)�������C��x��s@��S�E�*�N�2MQxi�^�h�m��z����G1��,?���K�� �8I�J	@��;��Q	#�}Ͳ���c��/��=�m���M<�Mǝ�(fV�~�ZO[8ݩ��
�+�h+ޔ������#UQ���{k��.��m\<',��w-U��NM��b�P�}Xd;*Z��<��>6�f���O����ܸ�6�\;�B�<LD�M?2�V�tR[���Z|�ZVv�����t�1���l�8q�xAaɝI��rA��-ZNM-�o��ӭ���u�Z�~�ҍ�/��4���(�Ƶ�;�t4"n������p���Ė�b�]�����<��&�S��P��R�F&d��qΤC���j��-��<�ˣ�v|�9P�5�)"�Q����#�yE�	H�����e�x�',D%[e�{`��'�kN�&�X�J�?��v��h(+c�cӼʙ���C�Ce�7uD��j��tP��n\X���8Z�D�B��@��M�^#Vһz�����@��&	���=��E���b�*�S�+�~ڼX�=��lO���K����XH�l��=�'|ju෱,CO�2l�b��� W�����
 ��g��B[�z��+Y����������L��Ǐ�Jga���a�,BN�u����B�����m�e1���`9�d�^���ޏ�>��՟�}��Bʳj��c4�"MH�-^5F`*�/�x��]�#c��g�R<�n�q��#.�J���ƐG"Ա�c6�!�}��?Z��f�uKT�VOW<�1b��{\H��{�NY���20Cޔ"��:u՝��"�žh���Rq(��Ψ7�����E����6�O��w�TYLWk}�;�?2���o>*0�x���R$��|�r��I@0/�-�8X��m9��l@�zq�v��=����;�l��7�7M��H�q��B�֑��!~uɵ�����p>����[�A�Z�,�7��;��
k���ڰul��Z#��C��-�R�Dcp=��&!�Ǩ0DP�*a��W��h��[{h���4�j�b!K������������5��􎋔��At��kv���k�FH\�Yx��z�o�eC�W���~�[H��M��Ʀ��U@T:Q ?R��ZQ;�wd�H�Y����ߓ�~�冀��w�ͥ@�`���R�[�X8� .ӭ������S���:
1M��-
�u���܄�ϴ)܀�ͭ��)4�
�k�������j��\�]��Q���m�^'v��3���{�T����8q�u^Fslj���~��@,�>�u���i��n���r�xhۈ��O;�9��[�8q�śX2�v9����|~b��<��FU�{������윗�@e�bZ�{�S?Z��X]�P�֍(��̂cB�c"�-��(�1Y�YEsG���]P�W���Hù����
�)!����?��;�_y�Ԋ2Q��Z��#�3T�x��䝤��?�Aֆ(X�^�� ׮�&����K�|�ap*��L�Q�ۛ�Ĺy�.�&�9
����!��O*"d}	��A��a��D��I�I�I�ќ�T�]q��J���^�I�Э�^Զ���8��E��B�:�Q�Kx����B���2�U_O�C�'�r�%��%q'�0r��}����xÃ�9!�֧�B8/���G��G
{�ZD�JG�m��G�>� D 0��Z!D9�9�-��KD�t"0^	q5j����lJLv�3HUt����4��,���]ǉ���̳��٧��E�P�����'����V��Gr�%�Pm���K�w�_7m����!�*�J���������Ve#fIB��7u	�L�8m�R�Un�Y�H�˻�'@�ڥccj��M��'�A��הm�@�4L˂��Ν�`xk�7����ַK���a�=`An�`Biȷ`�5��ٲ���}��j���3�1H̱.pA��.�9T$Bo/��j��>��cy}�*?{V�a���c���#S�\��K�j����F���U���R�V������'�Le4�UR~�.%QU;Bטg��gk�E!�G��|Պ��xb��K�KNJ��(ːJ��K�0.`��n!�����V���^�lN�����ߕr����T5CN�Bb �a���\|�#���Bcw���%� �,��>ݠ�؜��j#�R����m!�h���o�2�S���fg9�<��%�1���I'�P�_;QA�+_���w�	�����T�e8�R�蔬�l��K>\
c�<�0����P�[��7W(�o��s\LѸ���#=��j��Ub`Φyz�e�
�K��i��K������'�0�Hi�Wp׬,޼���N�UA��G���[<���ZkY�}y@3�|�YM�^�o4�8B'0[��l�-඙&ܔ ��At	#vЎ�&=Î�"��\�7���F����l�t�	���)�s^f�z��;�!��l��)�m�?�m���S OZ�Q����;�]l�D"¬�RD��I�߬� �>��o�E��r%.y�=_=ۭ��!��t8z�s��3D������	��k�V�{=TY&��=9��8��;����"ʰ�T2�pG��d^�X-�*�i"v̰]RwcE��*]���>�����С+;Y[�(���� �%N�R-�G�S,Z�N+{�r��N�F(�.N��v�����D_��f^��l(��_���_���|s;��ީh�� U`U{&�,w�o�~��~z@�9x�w�������P%fo��E��[Pgu���&����1)����C���E������T���|�.�Z��{���M�T6I�6Ź���ϳ?������@�	�7��1�a Z�5�=q1�]L$��z�{�b2~��@��*��iW����	D�+|+��xJ�'Rs��Bg{>��p��o����y�\p3�Ei�T�����P��N�a����Tk9ԫ����h��%������R�/�&�\ə�p������?a�c�������W9l��ߓ�APD+���{aǕpb�#p!�m�&����y�|y��3�x�+�i��E�O�       <*��������ˤ9�� ���%h������e�M�ng�A��f�L�C�[��}�=?�4sh�iS�s1լ��Lv-j�u�v�x��q��Ȋ?�3eξ���{*�:�����s=�)�W�~:��08	��Q����`�࠱`� :��Ҷy�,��0�Ȟ�����Pq6��#'1j�ȗ����o3_p��҈li�!1ӎU.՜vʶNі1���������Ϫ�l������4"���9~�{�� /��]�v:�0�&?4J=u9�n����_um��49�Oǌ�t"���{պ&X�a\q[jCSg��
�`�:�/��Q'��7�.�-z��u��w�#I#�%3�R��|g�;�`�	(�z��h�K��D	��/5��� �1E�����a�v�dt#�lLe�\F���)�STp,�J1�(�}LBmG(�7y���Ly��|����Q��+�RZ��*? �3nk�7�Q����K�Y�b�B�Ev�ڶF���eb�y����kF�[v����8etp%t���m�T�V��j���\���%��[���<'��	�U`�uf9I<~�����It��
�w��T0�j6��s�9����"����/"f��\��p|��20��I�R��b	�xI�+��2���U]ٮ������{��ǋ鸱%�|���ssX�>\��y)��3�I����w��/>�H#�R��pQ�#:_.݁��J8��1�u��)2A�V�>�~RX�J>KG����B��}��g._QG;�XEկ��K�$Ǣ�������T�w�����)á�ӊ��L4+���<�x�������?9e�S��瞇F8���B][rVf�3�ٱ��A�/8-�e�adp��
#�`�i�����w#Wz.7݀�&Se�'a��8������u �
%���70 mvpoNim���F�����Ю$��=��0�ӝ�7�O|r�~E?-X&���/�E��R�R�wn�9!Sl�B�M�D`~�r6b��;��M�X�����)��c����n�g�E�,�N>�6]ɺ�'��8�;((2q��N��X��J����̈́��cM�VS��tl�����2�EB��)t��%5��;Qvm�׿�0\��N�1:���372:�|���b�������%�d�n�$^u�`�h�nDdH���_{�yP7����4�^��ƒ"�z[�k�� ��i��B9z=>d�	뼷�W��r�s�&�x�X��pfƳ�YQ���]��o�y�A��#�)�o}���g�n�����n���H��I��,�D��LƲ}� ̡~��4�e������U��u��[M�4��S��L�`Y�]��;��1�6�k`h	�x:z��I��l�J�,����Ŋ�H�M�W������312�\Rs��g*��Ũ�#WpگN)E���|��b%��@
B��Z�tm�룹�����j?M@_���i��lb��e��R��� ��e���*&؃�?����(���{��WY�L&]���o@~0y��%G�ZX,�rӛ�_�� ��D��O^�B��5X���>���ʘA��?�����I����~�;B��~���'ϖ�ƴ�}��g.�2ea����S�ܹ�V���7��	��Z��'�A=`0-{�W�� ��뮈��roA�=X�xץ!zH��T�_��!�W��邬ng���O��^��h��+�N�zs�SK�MD$9���/*���|d��p?��v9��ZǵG/���H��Q#(fٜ��������-���ۘ��7ۙ�zA��v�i�d���h��ƞZ8<�P���|�u�E�S���z['�-���iƲw� ��\"S�&@�a�KI����Ie��G�c�p�_�D�9Q��#ͣ�S*����ȩ�L��䕓���WU�y��#�e���ov����1�b���f˓F|�%E��*"�~z�tB^�+*A�׹)
HM�8I��G�n���=0Y'_�%�x�C9X*Lʂ����j�������Pj������'H��j���)&��;1�g�1b��?�/��l��� S��y@Co��@U~9H6R��M V&�s\��r���6����H�����\���o�h���a�}��)hX���г�R>���_KR9��Cy�"�:���ͣ�`��i䩍��fx�$�/��_���{:�`��4���4��~mؒ����
][��0���-)�\�i\M\S���N͝=>�p�^;�%��q��3�â_�Gn����y~�(�7�qn���7AHl�q�����~k�T�=���}f#-�?�������ވ&��!:��=_U�n�9W��x��5��J��v�����0��~xpM���U\�R���J������;	�R�߽���L8���R[��麲F�c5���i�fԬ���@R�\9�2ŗm�ǱC�آ�L&��9Dz��������9�c�H�wO���c��
���F��� M��Z��x`��b�c�5Os5�@��q�Ry��I���s�	����i��� ��ӛʷ�*�[x��[�?����ۚ�Q�`-<�u-�n�&�b�S�k>s�~'ʥ��Q�ʁȞl���PU���ˌW�b3��L��ȱ���<p�z��� 5�'+l{�+�tF�B���8Tx�s3-Gya-}����k'Z9Gh8�޷�}��^���8d�ݏ����㾝ⵣ���?��a�_�ꈾ7ۃKl_4�z�S0�į
��S�75���t H%%�Ԑ�ځ�:�Ph(aIvo��'*~\�R�d�M�Gt�D�K��o0$����U2mF��"x��K�@"�����n�QM"���ہ��"-�[��⭿�%����X)/u./O�&ȲVE��ԃ����q�x�4e��V㯋��9�dQe��$�oBsY�֧��	0��YVz� 	k~�w���Gҁ�u$:���z�(�ޞ�حߎ�]�z�G�����A��L9�|A��+��3šG��s����,�6me-�_�nd��&4��I�y���VRep,�B#)����Ͽf���Mr�wI����+���5����_?CI�I|%^3π�\�Sg����;(�׾N�P�^��[������%m�����q���et�d	��[��bT\�����_��E�BL����3ph-~�.U���n�͢���K�A3�a�¨��O;��s-|Gi08�:�����F��"�XQl�&�6	���]��bo@2N�J�B.��UUZ� �8C�ъ,�焸˻*��l��u�L�D�:fq�I���֍���W�ꌠ~+���RRdq�MX>v
�];�x�L�~.�{t%F��_��ʐ��v��]7/z��KR��r�q�?����J��п��P�L�c�j���k�=?�
{��Hxi��>[��r�!�r�mǺ�����v����2N�B�il�� a(��M��I�jb�#�|��c��w��RK�T'ͽŷC��t#z�ry��R<)���d��}��{�3�O'0U�B~�I\w��\j��GD붓�z���mB�x�)��=�f#+�Q�*@]^?x 8�A�f��ztC�N~���Х1-qU�8�����0RF��I��IJ�iG���x*�%"E��1���v�<+Z��<��	x��,�x��0�c��px����-��CB%u:jR�N|�́=�Tc�#�ᓾ絍�o�<e��xo�3b�1͙
���aɘ��7$�;ާ@ך����(���m�"� ��^���+*��c����̴nܷ�H������E���_�R.�\�XQx�		c�T�4�^7>k#Ha��N ����>�f8m#�
p�9�� ����?�NڝQ]I«t%��d��(��@�M&(�Ce�W�󻣟b��V}=�^�� !�E��Đ����!��l��V��Ç �b�W�Q�����P��-��Ű�ʁ��_��)�{�;7��^�b�Xgހ�p^����U:"�Rl$%$u�j��V�
�)�G1X�+����E�P�X���'i���2%������æ       <*�����dz����~��֘��h�������ƥ�g��������>��}�=�;���u곯[��:�\��� ��(�2����p����s��|��ѳ��SX4�ޝ奊��Y�
Z�{�w�����V�J	z:5;��ܭ����͢H �/��wH�~�� �<��H�1%�_�����;�ژ΍���!I2e+'�j��n��$6��9Dס$z��G�N�Q�r�L�uȠ�"B�����<�m]�Ehuq���+S��!�
1�;�-CHAA~E���t��#~��H��Xb���v+����%���!}G\�����D7i��<3��vF�ڀ���ЯQM�����W-tĻz[���l���Fq���tY�c[@|��À"$��U(���N� ��������o�(1g�nYW���K�ǣC��!_H�
N+7/�O�Z[���+����M�R"o�����=Սs�7k:��\PS�Zm�\������o�cg�pEU?�F�|ly�yb���{�yo��$_Ȁ~�l5frr��RW�a@W�[��hbZuQ�P�S���]��ɲog�o�{ z��슞_΍�j���P~�h�q�!_���J-��������H�������0��/)ep�%lR���'��|�Q$��(���<欠�,T]�L��]�m0q_Z�8�S��:�I��TcY�;�?���h�W�(���usR�e���X!�[�l	S�W�/4�}�$��5	27�������!9X
:5���l�u�+*�\�zK��ۆy3-,�~���W�	ֿ���&��[�zh�BWw/�=�M��kҭ��yN��8�0d�1����c9���������j#�R�W2�:�ܽ���b��b˨DG�s:�B���k�̮cIt��1t'=�(��N�P6�5�<��>T�G1ϑ��G��"T&��$0[{{�p�Cjk�ȣG��ܹф!e�;�K��Ɋp����k|&P�N��ԔN���+�b��P���ԃ٥y8��4��HA7[�.g�@`>�)�b��-4��6@�<��e��:���U&p5^�a n�
'	��>�C(n`�ߣ�u�7�L�o�n�g��L�oV4�p�~�'����<��	E�X�*>��*W������	(��I+}�`��Q�jq[s����s"�AN�s~�|,T���xX�z+��;�7A�ג
�a|*
*�i�9�ڨ_��Ŕ omLm#����1�uc��?n���RD���`�Bs�v+���*��ñʬZv����Zai����!�&��n��9m�˒��ZH����*��(����ĸ�W�m?K�ҔξO4�׏Z�ͩf߇�dS�vU�^��J�3�
�0ZY��������C2c��e�k�(f%6�rI)'��7�lq"�T�? ��%��	>{�������N�Ű2�ʯt;٦�)I�u�K/
0g��3���'� �~0e�k%{Z=ia�X���:X�����圴aM�R���	�I���ӑF*�$�4t�Q���) ��cl�@�	b�y�j	=�LP)uq|çlI8���d��!���ozY���`��d-�G�����h����� ���z��E�x��)6-�t*oE6ߌ�Y5~�hT�����W�|�@B��a�g��~aXٝ��?�4���*!�D75L8`0�j��j?�w�$P�6�YS���MX�0#���'�fV�>�[^l�<Z�8)޷Qm��jpm 40�8m����|���馮#Ѫ�@��qS��2�*{|��dF��#�'����G3�q�������V;(h6�VN�J�<�&�Ո���E��C�̶�ZhD��\Uֻl.k����z �����Ka�`C�R��s-�q�tjq3��k�(DI.o��)��o���A�pN%������u�mc5w��CT�Ш1;#/��m��џƩhO�����x�Q�T�{
�NK~�ke�~���yh�̖���B�u �˃_�Q�-�&�U�f�k&J��d�/�M�05.H�D_V� 5�*9�"X�gUg$���,����R�U��lr��K��8�emoj�)�'�4"�Qռ&�"7ОO��mU<��4�9�����Fܜt�Ԥ�_�:��e�l�A�^��htO����uX�KP�5��w�@��[�ƨ��@[��0�]����xm/��R��a�1K��>���Mtg1Ѯ�Zi�i���Hl��⬏�T܄--m�I�u8�?��zst�t�?7����΋f�DČ�vT��΀�5��(�z�%��}�t&�G{��/��4Dm�w� ��wS�!�Ŕηp��dJ�,X�l� ���|��*D��a���oQ-�ֺ���*4��сT
�K���b��h�K)gj�R�r5}����R�}��	���hu�.q�ŉį緳!�u���M�|Hץ�ɝ5��i�����F�鼽�G�j��{���ψ��!@\z�!ׄN�׽@����=�u�DNg���*`������x��-;~{����lM����1/DV��]J���������ꗢu�����,�<�A�k����0���2�0$���}��������4����)�̹Z�?��wl.:Ӽ�P�ϐ��Yk��{���R�+�Ƈϵ^�M)�	��E�7�d�?��ڈc �B���b'<x��˖r�q Bt,��EΖ���pbu�e!��%��F-V7�(��"�edra�h=[�)��F_�r8x�i��q�Hi���f���Ð��y؋F�鎿��T��\>�KZ%*��E��TuR���qb��J+�1�A#�d�.v��sw�$�M���3xd}���8
�xg�<� ��[x�sn���� G@����A	�@�@/Pu�</�+�]N$���742�>UK!�]�;8X<�����XÔ��t����%F��9b[�M��n�u͟r��ּC))��Clm|�۲�z
�j�E��R����?3�I�2�\���)���8�G����*L[���VV�OӍ�u��7�[}�����x�Q�Q���/	�a���4Q�#F�O�!}ް�u�oA�ؔ��4J�s`�!À�|=�X�];J{�;�����z�״a`�?nɢ�n������:H��H��+�?̆�[�Y��P�
H(��]T�F��W���F��B�>υ��"��1�
,w��X��<�\|P����#���g	j�����k��t���^�[�_�� U�<�>�C�`ܪ��r�CR�6
�ѭ�"6���[l�*�7<���7
]cC4O8�<WЅ��uTqwL�^.�ͷ �����{>`��z�D��mN-��<X�����J�q��#���Ny�]�X:	S[g��-��/���/M�k�.��mm���l����j>eK�a�ړt�*�$чe�2q�:��e~�W��S�M���ߏ.^E�g�;>3��6ۛ��c��?G�:_�����O˲6!�q/�Ǽ��뷅&1�:�����8L����mĠ��[����;B���\5*I $��!/7;+]
�]m��hb�j��Io��bf'�e+3S�Y��8���uf��F����_�хkC���#��Z��h)X��pC���'�A�Pw�98Irr߾ۿV��i�D���s>�����H�S�i裇 ƅg���<[�Ԭ �*z>�i��ìp��WN���ӛC��YR�<�e��q��F� �3� Ce��zq@J�S+
�.�t����w �	�h�肈�y/>��In���&��ZK�N}R�\������eZ�&rGK�!��2�y�D�6R��.ܶ��Ro�P2����qwi�G��������<���Q(р�usl[�p��.���뉌|6��L$H�Z���Ն[��]ڻj�M��5SH?�S�����Q��f.�8���'�t���xY����:%1�ƣa�H�#p"�:�����o������/e�h���