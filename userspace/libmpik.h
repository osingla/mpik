
int mpik_channel_create(const char *name, int msec_timeout, int maxnb_msg_buffered);

int mpik_channel_delete(int chid, int msec_timeout);

int mpik_channel_connect(const char *name, int msec_timeout);

int mpik_channel_disconnect(int chid, int msec_timeout);

int mpik_receive(int chid, void *recv_buffer, int recv_buffer_sz, int msec_timeout);

int mpik_reply(int chid, int index, void const *reply_buffer, int reply_len, int msec_timeout);

int mpik_send(int chid, void const *send_buffer, int send_len, void *reply_buffer, int reply_maxlen, int msec_timeout);

int mpik_ping(int chid, int msec_timeout);

