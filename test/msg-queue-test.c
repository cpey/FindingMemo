#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define NUM_MSQIDS 1

struct msgbuf {
	long mtype;
	char mtext[80];
};

int send_msg(int qid, int msgtype) 
{
	struct msgbuf msg;
	time_t t;

	msg.mtype = msgtype;
	snprintf(msg.mtext, sizeof(msg.mtext), "a message at %s", ctime(&t));
	printf("msg send: %s\n", msg.mtext);

	if (msgsnd(qid, (void *) &msg, sizeof(msg.mtext), IPC_NOWAIT) < 0) {
		perror("msgsnd error");
		exit(EXIT_FAILURE);
	}
}

int get_msg(int qid, int msgtype) 
{
	struct msgbuf msg;

	if (msgrcv(qid, (void *) &msg, sizeof(msg.mtext), msgtype,
				MSG_NOERROR | IPC_NOWAIT) < 0) {
		if (errno != ENOMSG) {
			perror("msgrcv error");
			exit(EXIT_FAILURE);
		}
		printf("No message available for msgrcv()\n");
	} else {
		printf("Message received: %s\n", msg.mtext);
	}
}

int main(int argc, char** argv) 
{
	int qid[NUM_MSQIDS];
	int msgtype = 1;

	printf("+ Message queue creation\n");
	for (int i = 0; i < NUM_MSQIDS; i++) {
		if ((qid[i] = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) < 0) {
			perror("msgget error");
			exit(EXIT_FAILURE);
		}
	}

	printf("+ Send message\n");
	send_msg(qid[0], msgtype);

	printf("+ Get message\n");
	get_msg(qid[0], msgtype);

	exit(EXIT_SUCCESS);
}


