#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define CONNECTION_TIMEOUT 10
#define MAX_MESSAGE 256
#define MAX_USERNAME 20

struct Client {
	int sock;
	struct sockaddr_storage address;
	socklen_t addressLength;
	char username[MAX_USERNAME + 1];
	struct timeval lastSentData;
};

struct Data{
	char username[MAX_USERNAME + 1];
	char message[MAX_MESSAGE + 1];
};

void checkingExpiratingClientsHandler(int signum);
void sendDataToClients(struct Data data);
void deleteClient(int index);
void cleanup(int signum);

char *path;
struct Client clients[MAX_CLIENTS];
int clientsCounter = 0;
fd_set socketsSet;
int unixSocket;
int inetSocket;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Podales zle argumenty!\n");
		exit(-1);
	}
	int port = atoi(argv[1]);
	if (port <= 0) {
		printf("Zly port!\n");
		exit(-2);
	}
	path = argv[2];

	struct sigaction act;
	act.sa_handler = cleanup;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTSTP, &act, NULL);
	act.sa_handler = checkingExpiratingClientsHandler;
	sigaction(SIGALRM, &act, NULL);

	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		clients[i].sock = -1;
		clients[i].username[0] = '\0';
		clients[i].addressLength = 0;
	}

	unlink(path);
	unixSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
	inetSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (unixSocket == -1 || inetSocket == -1) {
		printf("Wystapil problem przy tworzeniu socketow!\n");
		exit(-3);
	}

	struct sockaddr_in serverInet;
	serverInet.sin_family = AF_INET;
	serverInet.sin_port = htons(port);
	serverInet.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(inetSocket, (struct sockaddr*)&serverInet, sizeof(serverInet)) == -1) {
		printf("Nie moglem zwiazac adresu inet z socketem!\n");
		exit(-4);
	}


	struct sockaddr_un serverUnix;
	serverUnix.sun_family = AF_UNIX;
	strcpy(serverUnix.sun_path, path);
	if (bind(unixSocket, (struct sockaddr*)&serverUnix, sizeof(serverUnix)) == -1) {
		printf("Nie moglem zwiazac adresu unix z socketem!\n");
		exit(-5);
	}

	struct itimerval timerInterval;
	struct timeval interval;
	interval.tv_sec = 5;
	interval.tv_usec = 0;
	timerInterval.it_interval = interval;
	timerInterval.it_value = interval;

	setitimer(ITIMER_REAL, &timerInterval, NULL);

	fd_set readSocketsSet;
	FD_ZERO(&socketsSet);
	int higherFD = unixSocket > inetSocket ? unixSocket : inetSocket;
	FD_SET(unixSocket, &socketsSet);
	FD_SET(inetSocket, &socketsSet);

	struct timeval timeOfReceivedData;
	while(1) {
		gettimeofday(&timeOfReceivedData, NULL);
		readSocketsSet = socketsSet;
		if (select(higherFD + 1, &readSocketsSet, NULL , NULL , NULL) < 0) {
			if (errno == EINTR)
				continue;
			else {
				printf("Blad przy wywolaniu select()!\n");
				continue;
			}
		}
		struct Data data;
		struct sockaddr_storage address;
		int sock;
		socklen_t addressLength;
		if (FD_ISSET(unixSocket, &readSocketsSet)) {
			addressLength = sizeof(struct sockaddr_un);
			if (recvfrom(unixSocket, &data, MAX_MESSAGE, 0, (struct sockaddr *)&address, &addressLength) <= 0) {
				printf("Problem przy otrzymywaniu wiadomosci!\n");
				continue;
			}
			sock = unixSocket;
		}
		else if (FD_ISSET(inetSocket, &readSocketsSet)) {
			addressLength = sizeof(struct sockaddr_in);
			if (recvfrom(inetSocket, &data, MAX_MESSAGE, 0, (struct sockaddr *)&address, &addressLength) <= 0) {
				printf("Problem przy otrzymywaniu wiadomosci!\n");
				continue;
			}
			sock = inetSocket;
		}
		else
			continue;
		
		//checking if sender is on list of clients
		int index = -1;
		for (i = 0; i < MAX_CLIENTS; i++) {
			if (strcmp(clients[i].username, data.username) == 0) {
				index = i;
				break;
			}
		}

		//if client is new and if there is space for new client, add him
		if (index == -1) {
			if (clientsCounter == MAX_CLIENTS) {
				printf("Przyjalem maksimum klientow!\n");
				continue;
			}
			for (i = 0; i < MAX_CLIENTS; i++) {
				if (strlen(clients[i].username) == 0) {
					index = i;
					break;
				}
			}
			strcpy(clients[index].username, data.username);
			clientsCounter++;
		}
		clients[index].sock = sock;
		clients[index].lastSentData = timeOfReceivedData;
		clients[index].address = address;
		clients[index].addressLength = addressLength;

		sendDataToClients(data);
	}
}

void sendDataToClients(struct Data data) {
	int j;
	for (j = 0; j < MAX_CLIENTS; j++) {
		if (strlen(clients[j].username) == 0 || strcmp(clients[j].username, data.username) == 0)
			continue;
		if(sendto(clients[j].sock, &data, sizeof(data), 0, (struct sockaddr *)&clients[j].address, clients[j].addressLength) <= 0) {
			printf("Problem przy rozsylaniu wiadomosci zwrotnej!\n");
		}
	}
}

void checkingExpiratingClientsHandler(int signum) {
	struct timeval t;
	gettimeofday(&t, NULL);
	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].addressLength != 0 && clients[i].lastSentData.tv_sec + CONNECTION_TIMEOUT < t.tv_sec) {
			printf("Usunalem klienta %s\n",clients[i].username);
			clients[i].addressLength = 0;
			clients[i].sock = -1;
			clients[i].username[0] = '\0';
			clientsCounter--;
		}
	}
}

void cleanup(int signum){
	close(unixSocket);
	close(inetSocket);
	unlink(path);
	exit(-6);
}

