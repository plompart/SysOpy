#include<stdio.h>
#include<stdlib.h>
#include<mqueue.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<time.h>
#include <unistd.h>
#include<errno.h>
#include <signal.h>

typedef struct
{
	long type;
	int number;
	int id;
	char prime;
} messageStruct;


enum
{
	INITIAL = 1,
	READY,
	RESULT
} clientMessageEnum;

enum
{
	SEND_ID = 1,
	RANDOM_NUMBER
} serverMessageEnum;

int messageSize = sizeof(messageStruct)-sizeof(long);

static int clients[256];
static int ID = 1;
static int queueID;

messageStruct * message;

void closeQueue();
int createQueue(char* path, char * number);
void handleMessage(messageStruct *message);

int main(int argc,char* argv[])
{

	srand(time(NULL));

	signal(SIGINT,closeQueue);

	if(argc != 3)
	{
		printf("\nUsage <name> <id>\n");
		return -1;
	}

	int messageid = createQueue(argv[1],argv[2]);
	if( messageid < 0 ){
		return -1;
	}

	printf("SERVERID: %d\n",queueID);
	fflush(stdout);

	message = (messageStruct * )calloc(1,messageSize);

	while(1)
	{

		if (msgrcv(queueID,message,messageSize,0,0)==-1)
		{
			printf("Error while receiving message");
			exit(-1);
		}
		handleMessage(message);

	}
	return 0;
}

void closeQueue()
{
	if(msgctl(queueID,IPC_RMID,0)==-1)
	{
		printf("Error while removing queue");
		exit(EXIT_FAILURE);
	}
	free(message);
	exit(EXIT_SUCCESS);
}

int createQueue(char* path, char * number)
{
	key_t key = ftok(path,atoi(number));

	if (key==-1)
	{
		printf("Cannot create key number");
		return -1;
	}
	if((queueID = msgget(key,0666|IPC_CREAT))==-1)
	{
		printf("Cannot create server queue");
		return -1;
	}
	return queueID;
}

void handleMessage(messageStruct *message)
{

	if(message->type==(long)INITIAL)
	{
		clients[ID] = message->number;
		message->id= ID;
		message->type = SEND_ID;
		ID++;
		msgsnd(message->number,message,messageSize,0);
	} else if(message->type==(long)READY)
	{
		message->number = rand()%10+1;
		message->type = (long)RANDOM_NUMBER;
		msgsnd(clients[message->id],message,messageSize,0);
	} else if(message->type==(long)RESULT)
	{
		if(message->prime)
		{
			printf("Liczba pierwsza: %d (klient: %d)\n",message->number,message->id);
		}
		message->number = rand()%10 +1;
		message->type = (long)SEND_ID;
		msgsnd(clients[message->id],message,messageSize,0);

	} else
	{
		printf("co to za wiadomosc");
	}
}