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

#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

#include "mpik_ioctl.h"

#include "libmpik.h"

#include "../userspace/log.h"

static void *thread1(void *arg) {
	char recv_buffer[100];
	char reply_buffer[] = "Bonjour le monde\n";
	
	int chid1 = mpik_channel_create("channel_1", 1000, 0);
	DEBUG("chid1=%d", chid1);

	memset(recv_buffer, 0, sizeof(recv_buffer));
	int s = mpik_receive(chid1, recv_buffer, sizeof(recv_buffer), 1000);
	DEBUG("@@@ s=%d %d [%s]", s, strlen(recv_buffer), recv_buffer);

	int s2 = mpik_reply(chid1, s, reply_buffer, sizeof(reply_buffer), 1000);
	DEBUG("@@@ s2=%d", s2);

	return NULL;
}

static void *thread2(void *arg) {
	char send_buffer[] = "Hello world!\n";
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
	int s = mpik_send(chid1, send_buffer, sizeof(send_buffer), reply_buffer, sizeof(reply_buffer), 0);
	DEBUG("s=%d", s);
	DEBUG("reply_buffer=[%s]\n", reply_buffer);

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
