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

#define MAXNB_SEND_WAITING	32

typedef struct {
	wait_queue_head_t queue;				// To awake the task waiting on mpik_send() [reply]
	void *reply_buffer;						// Reply buffer address in user-space
	int reply_maxlen;						// Maximum size of the reply message
} send_waiting_t;

typedef struct {
	char name[MPIK_CHANNEL_MAXLEN+1];					// Channel name
	pid_t pid_owner;									// PID of owner - (-1) means not in use
	pid_t tgid_owner;									// Main PID of owner - (-1) means not in use
	int chid;											// Unique channel ID
	int maxnb_msg_buffered;								// Maximum number of messages that can be buffered
	void *recv_buffer;									// Where to store the received messages in receiver user-space
	int recv_buffer_sz;									// Maximum size of the message that can be received
	wait_queue_head_t queue;							// To awake the task waiting on mpik_receive()
	int nb_sent_waiting;								// Number of sent waiting for the reply
	int index;											// 
	send_waiting_t send_waiting[MAXNB_SEND_WAITING];	// Tasks waiting on mpik_send()
} channel_t;


#endif /* __MPIK_H__ */

