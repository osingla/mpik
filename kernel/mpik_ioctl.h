/*  
 *
 *  
 *  
 *  
 */

#ifndef __MPIK_IOCTL_H__
#define __MPIK_IOCTL_H__

#define MPIK_CHANNEL_MAXLEN 23

/* mpik_channel_create */
typedef struct {
	char name[MPIK_CHANNEL_MAXLEN+1];
	int msec_timeout;
	int maxnb_msg_buffered;
} mpik_ioctl_channel_create_t;
#define IOCTL_MPIK_CHANNEL_CREATE _IOR('X', 0, mpik_ioctl_channel_create_t *)

/* mpik_channel_delete */
typedef struct {
	int chid;
	int msec_timeout;
} mpik_ioctl_channel_delete_t;
#define IOCTL_MPIK_CHANNEL_DELETE _IOR('X', 1, mpik_ioctl_channel_delete_t *)

/* mpik_channel_connect */
typedef struct {
	char name[MPIK_CHANNEL_MAXLEN+1];
	int msec_timeout;
} mpik_ioctl_channel_connect_t;
#define IOCTL_MPIK_CHANNEL_CONNECT _IOR('X', 2, mpik_ioctl_channel_connect_t *)

/* mpik_channel_disconnect */
typedef struct {
	int chid;
	int msec_timeout;
} mpik_ioctl_channel_disconnect_t;
#define IOCTL_MPIK_CHANNEL_DISCONNECT _IOR('X', 3, mpik_ioctl_channel_disconnect_t *)

/* mpik_receive */
typedef struct {
	int chid;
	void *recv_buffer;
	int recv_buffer_sz;
	int msec_timeout;
} mpik_ioctl_receive_t;
#define IOCTL_MPIK_RECEIVE _IOR('X', 4, mpik_ioctl_receive_t *)

/* mpik_reply */
typedef struct {
	int chid;
	int index;
	void const *reply_buffer;
	int reply_len;
	int msec_timeout;
} mpik_ioctl_reply_t;
#define IOCTL_MPIK_REPLY _IOR('X', 5, mpik_ioctl_reply_t *)

/* mpik_send */
typedef struct {
	int chid;
	void const *send_buffer;
	int send_len;
	void *reply_buffer;
	int reply_maxlen;
	int msec_timeout;
} mpik_ioctl_send_t;
#define IOCTL_MPIK_SEND _IOR('X', 6, mpik_ioctl_send_t *)

/* mpik_channel_ping */
typedef struct {
	int chid;
	int msec_timeout;
} mpik_ioctl_ping_t;
#define IOCTL_MPIK_PING _IOR('X', 7, mpik_ioctl_ping_t *)


#endif /* __MPIK_IOCTL_H__ */
