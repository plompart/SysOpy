#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define MAX_MESSAGE 256
#define MAX_USERNAME 20

struct Data{
	char username[MAX_USERNAME + 1];
	char message[MAX_MESSAGE + 1];
};

int sock = -1;
pthread_t threadIO = -1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canReadData = PTHREAD_COND_INITIALIZER;
struct Data data;
int isDataReadyToSend = 0;
struct sockaddr_un su;
struct sockaddr_in sa;
struct sockaddr *serverAddress;
socklen_t addressLength;
char *path;
char *socketPathUnix;
char *serverType;

void* run(void* parameters);
void usr1Handler(int signum);
void cleanup(int signum);

int main(int argc, char* argv[])
{
	if(argc<4){
		printf("Podales zle argumenty!\n");
		exit(-1);
	}

	char *username=argv[1];
	char *addressIP;
	int port;
	if (strlen(username)>20)
	{
		printf("Podales za dluga nazwe!\n");
		exit(-7);
	}
	strcpy(data.username,username);
	serverType=argv[2];
	if(strcmp(serverType,"remote")==0){
		if(argc<5){
			printf("Podales zle argumenty!\n");
			exit(-2);
		}else{
			addressIP=argv[3];
			port=atoi(argv[4]);
		}
	}else if(strcmp(serverType,"local")==0){
		path=argv[3];
	}else{
		printf("Podales zle argumenty!\n");
		exit(-3);
	}

	struct sigaction action;
	action.sa_handler = cleanup;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTSTP, &action, NULL);
	action.sa_handler = usr1Handler;
	sigaction(SIGUSR1, &action, NULL);

	if(0==strcmp(serverType,"local")){
		socketPathUnix=malloc((strlen(path) + 10)*sizeof(char));
		sprintf(socketPathUnix, "%s-%d", path, getpid());
		if(-1==(sock=socket(AF_UNIX, SOCK_DGRAM, 0))){
			printf("Problem z socket()!\n");
			exit(-3);
		}

		unlink(socketPathUnix);
		su.sun_family = AF_UNIX;
		strcpy(su.sun_path, socketPathUnix);
		
		if(bind(sock, (struct sockaddr*)&su, sizeof(su)) == -1) {
			printf("Nie moglem polaczyc sie z serwerem!\n");
			exit(-4);
		}
		strcpy(su.sun_path,path);
		serverAddress = (struct sockaddr *)&su;
		addressLength = sizeof(su);
	}else{
		if(-1==(sock=socket(AF_INET, SOCK_DGRAM, 0))){
			printf("Problem z socket()!\n");
			exit(-5);
		}

		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = inet_addr(addressIP);
		serverAddress = (struct sockaddr *)&sa;
		addressLength = sizeof(sa);
	}

	if (pthread_create(&threadIO, NULL, run, NULL) != 0) {
		printf("Nie moglem utworzyc watku!\n");
		exit(-6);
	}

	struct Data dataServer;

	int receivedData = 0;
	do {
		errno = 0;
		receivedData = recvfrom(sock, &dataServer, MAX_MESSAGE+MAX_USERNAME+2, 0, NULL, NULL);
		if (receivedData > 0)printf("Klient %s > %s", dataServer.username, dataServer.message);
		fflush(stdout);
	} while (receivedData > 0 || (receivedData == -1 && errno == EINTR));

	return 0;
}

void* run(void* parameters){
	int readData=0;
	while(1){
		pthread_mutex_lock(&mutex);
		while (isDataReadyToSend) {
			pthread_cond_wait(&canReadData, &mutex);
		}
		readData = read(STDIN_FILENO, data.message, MAX_MESSAGE);
		if (readData == 0) {
			pthread_mutex_unlock(&mutex);
			continue;
		}
		if (data.message[readData - 1] != '\n') {
			printf("Too long message!\n");
			while ((readData = read(STDIN_FILENO, data.message, MAX_MESSAGE)) > 0);
			pthread_mutex_unlock(&mutex);
			continue;
		}
		data.message[readData] = '\0';
		isDataReadyToSend = 1;
		pthread_cond_signal(&canReadData);
		pthread_mutex_unlock(&mutex);
		kill(getpid(), SIGUSR1);
	}
}

void usr1Handler(int signum) {
	pthread_mutex_lock(&mutex);
	
	if(sendto(sock, &data, sizeof(data), 0, serverAddress, addressLength) <= 0) {
		printf("Problem przy otrzymywaniu wiadomosci!\n");
		pthread_mutex_unlock(&mutex);
		
		//cleanup
		if (sock != -1)close(sock);
		if (threadIO != -1) {
			pthread_cancel(threadIO);
			pthread_join(threadIO, NULL);
		}
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&canReadData);
		if (strcmp(serverType,"local"))unlink(socketPathUnix);
	}

	isDataReadyToSend = 0;
	pthread_cond_signal(&canReadData);
	pthread_mutex_unlock(&mutex);
}

void cleanup(int signum){
	if (sock != -1)
		close(sock);
	if (threadIO != -1) {
		pthread_cancel(threadIO);
		pthread_join(threadIO, NULL);
	}
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&canReadData);
	if (strcmp(serverType,"local"))
		unlink(path);

	exit(-7);
}