#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>



#define QUEUE_NAME 25
#define MESSAGE_SIZE sizeof(msg_t)
#define MAX_CLIENTS 25
#define MAX_MESSAGES 10

typedef struct  {
	char type;
	char prime;
	char id;
	int32_t number;
	char queue[QUEUE_NAME];
} msg_t;

typedef union {
	msg_t msg;
	char buff[MESSAGE_SIZE+50];
} msgBuff_t;

enum {
	INITIAL = 1,
	READY,
	RESULT,
	END
} client_msg_type_t;

enum {
	SEND_ID = 1,
	RANDOM_NUMBER,
} server_msg_type_t;

static mqd_t serverDescriptor;
static char * serverQueueName;
static int ID=1;
static msgBuff_t * msg;
static mqd_t clients[MAX_CLIENTS];

void openServerQueue(char* name);
void sigHandler(int signo);
void closeQueue(void);

int main(int argc, char* argv[]){

	if(argc != 2) {
		printf("Usage <name>\n");
		return -1;
	}

	srand(time(NULL));
	msg = (msgBuff_t * ) calloc (1, sizeof(msgBuff_t));
	serverQueueName = argv[1];
	atexit(closeQueue);
	signal(SIGINT, sigHandler);

	openServerQueue(serverQueueName);

	while(1) {
		if (mq_receive (serverDescriptor , msg -> buff, MESSAGE_SIZE, NULL) < 0) {
			printf( "Error receiving message%d\n",errno);
			exit(1);
		}
		handleMessage();
	}

	return 0;
}

void openServerQueue(char* name) {

	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MESSAGES;
	attr.mq_msgsize = MESSAGE_SIZE;
	attr.mq_curmsgs = 0;

	if ((serverDescriptor = mq_open(name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr)) < 0) {
		printf("Cannot open server queue\n");
		exit(1);
	}
}

void sigHandler(int signo) {
	exit(0);
}

void closeQueue(void) {
	if(mq_close(serverDescriptor)<0) {
		printf("Error while closig queue\n");
		exit(-1);
	}

	if(mq_unlink(serverQueueName) < 0){
		printf("Error while removing queue\n");
		exit(-1);
	}

	free(msg);

	exit(0);
}

void handleMessage() {

	switch(msg->msg.type) {

		case (char) INITIAL:
			if((clients[ID] = mq_open(msg -> msg.queue, O_WRONLY)) < 0){
				printf("Error opening the client queue \n");
				exit(-1);
			}

			msg -> msg.type = (char) SEND_ID;
			msg -> msg.id = (char) ID;
			if ( mq_send(clients[ID], msg -> buff, MESSAGE_SIZE, 0) < 0){
				printf("Error sending send_ID message \n");
				exit(-1);
			}
			ID++;
			break;

		case (char) READY:

			msg->msg.type = (char) RANDOM_NUMBER;
			msg->msg.number = rand() % 10 +1;

			if ( mq_send(clients[msg->msg.id], msg->buff, MESSAGE_SIZE, 0) < 0){
				printf("Error sending send_ID message \n");
				exit(-1);
			}

			break;

		case (char) RESULT:
			if(msg -> msg.prime) {
				printf("Liczba pierwsza %d, klient ID %d\n", msg -> msg.number, msg -> msg.id);
				fflush(stdout);
			}

			msg -> msg.type = (char) SEND_ID;

			if ( mq_send(clients[msg->msg.id], msg->buff, MESSAGE_SIZE, 0) < 0){
				printf("Error sending send_ID message \n");
				exit(-1);
			}

			break;


		case (char) END:

			if(mq_close(clients[msg -> msg.id])== -1) {
				printf("Error closing client descriptor\n");
				exit(-1);
			}

	}

}