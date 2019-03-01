#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <syslog.h>

#include "../kernel/mpik_ioctl.h"

#include "mpik.h"

#include "log.h"

static int mpik_fd = -1;

static void constructor() __attribute__((constructor));
static void destructor() __attribute__((destructor));

void constructor() {
    mpik_fd = open("/dev/mpik", O_RDWR);
    DEBUG("constructor: mpik_fd=%d - %m", mpik_fd);
}

void destructor() {
    DEBUG("destructor: mpik=%d", mpik_fd);
    if (mpik_fd != -1)
		close(mpik_fd);
}

/**
 * mpik_channel_create
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_channel_create(const char *name, int msec_timeout, int maxnb_msg_buffered) {
	mpik_ioctl_channel_create_t s;
	
	memset(&s, 0, sizeof(s));
	strncpy(s.name, name, MPIK_CHANNEL_MAXLEN);
	s.msec_timeout = msec_timeout;
	s.maxnb_msg_buffered = maxnb_msg_buffered;
	int chid = ioctl(mpik_fd, IOCTL_MPIK_CHANNEL_CREATE, &s);
	return chid;
}

/**
 * mpik_channel_delete
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_channel_delete(int chid, int msec_timeout) {
	mpik_ioctl_channel_delete_t s;
	
	memset(&s, 0, sizeof(s));
	s.chid = chid;
	s.msec_timeout = msec_timeout;
	int status = ioctl(mpik_fd, IOCTL_MPIK_CHANNEL_DELETE, &s);
	return status;
}


/**
 * mpik_channel_connect
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_channel_connect(const char *name, int msec_timeout) {
	mpik_ioctl_channel_connect_t s;
	
	memset(&s, 0, sizeof(s));
	strncpy(s.name, name, MPIK_CHANNEL_MAXLEN);
	s.msec_timeout = msec_timeout;
	int status = ioctl(mpik_fd, IOCTL_MPIK_CHANNEL_CONNECT, &s);
	return status;
}


/**
 * mpik_channel_disconnect
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_channel_disconnect(int chid, int msec_timeout) {
	mpik_ioctl_channel_disconnect_t s;
	
	memset(&s, 0, sizeof(s));
	s.chid = chid;
	s.msec_timeout = msec_timeout;
	int status = ioctl(mpik_fd, IOCTL_MPIK_CHANNEL_DISCONNECT, &s);
	return status;
}


/**
 * mpik_receive
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_receive(int chid, void *recv_buffer, int recv_buffer_sz, int msec_timeout) {
	mpik_ioctl_receive_t s;
	
	memset(&s, 0, sizeof(s));
	s.chid = chid;
	s.recv_buffer = recv_buffer;
	s.recv_buffer_sz = recv_buffer_sz;
	s.msec_timeout = msec_timeout;
	int status = ioctl(mpik_fd, IOCTL_MPIK_RECEIVE, &s);
	return status;
}

/**
 * mpik_reply
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_reply(int chid, int index, void const *reply_buffer, int reply_len, int msec_timeout) {
	mpik_ioctl_reply_t s;
	
	memset(&s, 0, sizeof(s));
	s.chid = chid;
	s.index = index;
	s.reply_buffer = reply_buffer;
	s.reply_len = reply_len;
	s.msec_timeout = msec_timeout;
	int status = ioctl(mpik_fd, IOCTL_MPIK_REPLY, &s);
	return status;
}

/**
 * mpik_send
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_send(int chid, void const *send_buffer, int send_len, void *reply_buffer, int reply_maxlen, int msec_timeout) {
	mpik_ioctl_send_t s;
	
	memset(&s, 0, sizeof(s));
	s.chid = chid;
	s.send_buffer = send_buffer;
	s.send_len = send_len;
	s.reply_buffer = reply_buffer;
	s.reply_maxlen = reply_maxlen;
	s.msec_timeout = msec_timeout;
	int status = ioctl(mpik_fd, IOCTL_MPIK_SEND, &s);
	return status;
}

/**
 * mpik_ping
 * 
 * 
 * @note This function must be called only once per process, namely from the main thread.
 * 
 * @see messip_disconnect(), messip_channel_create(), messip_channel_disconnect()
 */
int
mpik_ping(int chid, int msec_timeout) {
	mpik_ioctl_ping_t s;
	
	memset(&s, 0, sizeof(s));
	s.chid = chid;
	s.msec_timeout = msec_timeout;
	int status = ioctl(mpik_fd, IOCTL_MPIK_PING, &s);
	return status;
}

