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

#define MAX_MSG_SIZE 256
#define QUEUE_NAME 25
#define MESSAGE_SIZE sizeof(messageStruct)

typedef struct
{
	char type;
	char prime;
	char id;
	int32_t number;
	char queue[QUEUE_NAME];
} messageStruct;

typedef union
{
	messageStruct message;
	char buff[MESSAGE_SIZE+50];
} messageBuffer;

enum
{
	INITIAL = 1,
	READY,
	RESULT,
	END
} clientMessageEnum;

enum
{
	SEND_ID = 1,
	RANDOM_NUMBER,
} serverMessageEnum;

static mqd_t queueDescription;
static char * queueName;
static mqd_t serverQDescription;
static int ID;
static messageBuffer * message;

int isPrime(int n);
void createClientQueue();
void openServerQueue(char* name);
void signalHandler(int signo);
void closeQueue(void);
void handleMessage();

int main(int argc, char* argv[])
{

	if(argc != 2)
	{
		printf("Usage <name>\n");
		return -1;
	}

	srand(time(NULL));
	message = (messageBuffer *) calloc (1,sizeof(messageBuffer));

	atexit(closeQueue);
	signal(SIGINT, signalHandler);

	openServerQueue(argv[1]);
	createClientQueue();

	message->message.type = (char) INITIAL;
	strcpy(message->message.queue, queueName);

	if (mq_send(serverQDescription, message->buff, MESSAGE_SIZE, 0) < 0)
	{
		printf("Error sending initial message\n");
		exit(-1);
	}

	while(1)
	{
		if (mq_receive(queueDescription, message->buff, MESSAGE_SIZE, NULL) < 0)
		{
			printf( "Error receiving message %d\n",errno);
			exit(-1);
		}
		handleMessage();
	}
	return 0;
}

int isPrime(int n)
{
	if(n <= 1 || (n % 2==0)) return 0;
	int i;
	for(i = 3; i * i <= n; i+=2)
	{
		if(n % i == 0) return 0;
	}
	return 1;
}

void createClientQueue()
{
	char * charset= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	size_t charLenght = strlen(charset);

	queueName = malloc(sizeof(char) * QUEUE_NAME);
	int i;
	queueName[0] = '/';
	for ( i = 1; i < QUEUE_NAME - 1; i++)
	{
		queueName[i]= charset[rand() % charLenght];
	}
	queueName[QUEUE_NAME]= '\0';

	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MESSAGE_SIZE;
	attr.mq_curmsgs = 0;

	if((queueDescription = mq_open(queueName, O_RDONLY | O_CREAT, 0666, &attr )) == -1)
	{
		printf("Error while creating client queue%d\n",errno);
		exit(-1);
	}
	printf("desc %d",queueDescription);
}

void openServerQueue(char* name)
{

	if ((serverQDescription = mq_open(name, O_WRONLY)) < 0)
	{
		printf("Cannot open server queue\n");
		exit(-1);
	}
}

void signalHandler(int signo)
{
	exit(0);
}

void closeQueue(void)
{
	message->message.type = (char) END;
	if (mq_send(serverQDescription, message->buff, MESSAGE_SIZE, 0) < 0)
	{
		printf("Error sending last message\n");
		exit(-1);
	}
	if(mq_close(queueDescription) < 0)
	{
		printf("Error while closing queue\n");
		exit(-1);
	}

	if(mq_close(serverQDescription) < 0)
	{
		printf("Error while closing server queue\n");
		exit(-1);
	}

	if(mq_unlink(queueName) < 0)
	{
		printf("Error while removing queue\n");
		exit(-1);
	}

	free(message);
	free(queueName);
	exit(0);
}

void handleMessage()
{

	switch(message->message.type)
	{

		case (char) SEND_ID:

			ID = message->message.id;
			sleep(1);
			message->message.type = (char) READY;
			if (mq_send(serverQDescription, message->buff, MESSAGE_SIZE, 0) < 0)
			{
				printf("Error sending ready message \n");
				exit(-1);
			}

			break;

		case (char) RANDOM_NUMBER:

			message->message.prime = isPrime(message->message.number);
			message->message.type = RESULT;
			if (mq_send(serverQDescription, message->buff, MESSAGE_SIZE, 0) < 0)
			{
				printf("Error sending result message %d \n",errno);
				exit(-1);
			}

			break;
	}

}