/*  
 *
 *  
 *  
 *  
 */

#ifndef __MPIK_H__
#define __MPIK_H__


/* The name of the device file */
#define DEVICE_FILE_NAME "mpik"

#define MAXNB_CHANNELS 128

#define MAXNB_CHANNELS_ATTACHED	32

#define MAXNB_SEND_WAITING	32

typedef struct {
	pid_t tid;								// Task id of task waiting on the mpik_send()
	wait_queue_head_t queue;				// To awake the task waiting on mpik_send() [reply]
	void *reply_buffer;						// Reply buffer address in user-space
	int reply_maxlen;						// Maximum size of the reply message
} send_waiting_t;

typedef struct {
	pid_t tid;								// TID of the task attached to the channel
} channel_cnx_t;

typedef struct {
	char name[MPIK_CHANNEL_MAXLEN+1];					// Channel name
	pid_t tid_owner;									// Task ID of owner - (-1) means not in use
	pid_t pid_owner;									// Provcess ID of owner
	int maxnb_msg_buffered;								// Maximum number of messages that can be buffered
	void *recv_buffer;									// Where to store the received messages in receiver user-space
	int recv_buffer_sz;									// Maximum size of the message that can be received
	int nb_cnx;											// Current number of tasks attached to this channel
	channel_cnx_t channel_cnx[MAXNB_CHANNELS_ATTACHED];	// Information for the tasks attached to this channel - See nb_cnx
	wait_queue_head_t queue;							// To awake the task waiting on mpik_receive()
	int nb_sent_waiting;								// Number of sent waiting for the reply
	send_waiting_t send_waiting[MAXNB_SEND_WAITING];	// Tasks waiting on mpik_send()
	int index;											// 
} channel_t;


#endif /* __MPIK_H__ */

