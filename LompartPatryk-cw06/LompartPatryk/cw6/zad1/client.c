#include<stdio.h>
#include<stdlib.h>
#include<mqueue.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<time.h>
#include<errno.h>
#include <signal.h>
#include <unistd.h>

typedef struct
{
	long type;
	int number;
	int id;
	char prime;
} messageStruct;

 enum
{
	INITIAL=1,
	READY,
	RESULT
} clientMessageEnum;

enum
{
	SEND_ID=1,
	RANDOM_NUMBER,
} serverMessageEnum;

int msgSize=sizeof(messageStruct)-sizeof(long);
static int queueID;
static int serverID;
static int ID;
messageStruct * msg;

void sigHandler(int signo);
int isPrime(int n);
void closeQueue(void);
void openServerQueue(char* path, char * number);
void createClientQueue();
void handleMessage(messageStruct * msg);

int main(int argc, char* argv[])
{
	
	srand(time(NULL));
	
	if(argc != 3)
	{
		printf("\nUsage <name> <id>\n");
		return -1;
	}
	
	atexit(closeQueue);
	signal(SIGINT,sigHandler);
	openServerQueue(argv[1], argv[2]);		
	createClientQueue();

	msg=(messageStruct*) calloc(1,msgSize);
	msg -> number=queueID;
	msg -> type=INITIAL;
	
	if(msgsnd(serverID, msg, msgSize, 0) == -1)
	{
		printf("Error while sending initial message to server");
	}
	
	while(1)
	{	
		if (msgrcv(queueID, msg, msgSize,0,0) == -1)
		{
			printf("Error while receiving message");
			exit(-1);
		}
		handleMessage(msg);
	}
	
	return 0;
}

int isPrime(int n)
{
	if (n <= 1 || (n % 2 == 0)) return 0;
	int i;
	for (i = 3; i * i <= n; i += 2)
	{
		if (n % i == 0) return 0;
	}
	return 1;
}

void closeQueue(void)
{
	free(msg);

	if(msgctl(queueID, IPC_RMID, 0) == -1)
	{
		printf("Error while removing queue");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

void openServerQueue(char* path, char * number)
{
	key_t key=ftok(path,atoi(number));

	if (key <0)
	{
		printf("Cannot create key number");
		exit(-1);
	}

	if((serverID=msgget(key, 0666)) == -1)
	{
		printf("Cannot open server queue");
		exit(-1);
	}
}

void createClientQueue()
{
	if((queueID=msgget(IPC_PRIVATE, 0666)) == -1)
	{
		printf("Error while creating client queue");
		exit(-1);
	}
}

void handleMessage(messageStruct * msg)
{
	if( msg -> type == (long) SEND_ID)
	{
		ID=msg -> id;
		sleep(1);
		msg -> type=(long) READY;
		if(msgsnd(serverID, msg, msgSize,0) == -1)
		{
			printf("Error sending READY message");
			exit(-1);
		}

	} else if(msg-> type == (long) RANDOM_NUMBER )
	{
		int number=msg -> number;
		ID=msg -> id;
		msg -> type=(long) RESULT;
		msg -> prime=isPrime(number);
		if(msgsnd(serverID, msg, msgSize, 0) == -1)
		{
			printf("Error while ");
		}
	}
}

void sigHandler(int signo)
{
	exit(0);
}