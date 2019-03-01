#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#include "mpik_ioctl.h"

#include "mpik.h"

#include "../userspace/log.h"

#define MSG_SENT_1		"Hello world!\n"
#define MSG_SENT_2 		"How are you going?\n"
#define MSG_REPLIED_1	"Bonjour le monde\n"
#define MSG_REPLIED_2	"Tres bien, et vous?"

static void *thread1(void *arg) {
	char recv_buffer[100];
	
	int chid1 = mpik_channel_create("channel_1", 1000, 0);
	DEBUG("chid1=%d", chid1);
	assert(chid1 >= 0);

	int chid1b = mpik_channel_create("channel_1", 1000, 0);
	assert((chid1b == -1) && (errno == EEXIST));

	memset(recv_buffer, 0, sizeof(recv_buffer));
	int s = mpik_receive(chid1, recv_buffer, sizeof(recv_buffer), 1000);
	DEBUG("@@@ s=%d %d [%s]", s, strlen(recv_buffer), recv_buffer);
	assert(!strcmp(recv_buffer, MSG_SENT_1));

	int s2 = mpik_reply(chid1, s, MSG_REPLIED_1, sizeof(MSG_REPLIED_1), 1000);
	DEBUG("@@@ s2=%d", s2);

	s = mpik_receive(chid1, recv_buffer, sizeof(recv_buffer), 1000);
	DEBUG("@@@ s=%d %d [%s]", s, strlen(recv_buffer), recv_buffer);
	assert(!strcmp(recv_buffer, MSG_SENT_2));

	s2 = mpik_reply(chid1, s, MSG_REPLIED_2, sizeof(MSG_REPLIED_2), 1000);
	DEBUG("@@@ s2=%d", s2);

	return NULL;
}

static void *thread2(void *arg) {
	char reply_buffer[50];

	int chid1;
	for (;;) {
		chid1 = mpik_channel_connect("channel_1", 1000);
		if (chid1 != -1)
			break;
		sleep(1);
	}
	DEBUG("chid1=%d", chid1);

	sleep(10);
	int s = mpik_send(chid1, MSG_SENT_1, sizeof(MSG_SENT_1), reply_buffer, sizeof(reply_buffer), 0);
	DEBUG("s=%d", s);
	DEBUG("reply_buffer=[%s] %d\n", reply_buffer, strlen(reply_buffer));
	assert(!strcmp(reply_buffer, MSG_REPLIED_1));

	s = mpik_send(chid1, MSG_SENT_2, sizeof(MSG_SENT_2), reply_buffer, sizeof(reply_buffer), 0);
	DEBUG("s=%d", s);
	DEBUG("reply_buffer=[%s] %d\n", reply_buffer, strlen(reply_buffer));
	assert(!strcmp(reply_buffer, MSG_REPLIED_2));

	return NULL;
}

int main(int argc, char *argv[]) {

	pthread_t tid1, tid2;
	pthread_attr_t tattr1, tattr2;

	pthread_attr_init(&tattr1);
	pthread_create(&tid1, &tattr1, &thread1, NULL);
	
	pthread_attr_init(&tattr2);
	pthread_create(&tid2, &tattr2, &thread2, NULL);
	
	printf("Press <CR> to stop");
	getchar();

	return 0;
}
