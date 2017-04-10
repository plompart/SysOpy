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
		
	}

	unlink(path);
	unixSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	inetSocket = socket(AF_INET, SOCK_STREAM, 0);

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
    //listen() marks the socket referred to by sockfd as a passive socket, 
    //that is, as a socket that will be used to accept incoming connection requests using accept
    //SOMAXCONN maximum number allowed to pass to listen()
	if (listen(inetSocket, SOMAXCONN) == -1) {
        close(inetSocket);
        printf("Problem z inet listen()!\n");
        exit(-6);
    }


	struct sockaddr_un serverUnix;
	serverUnix.sun_family = AF_UNIX;
	strcpy(serverUnix.sun_path, path);
	if (bind(unixSocket, (struct sockaddr*)&serverUnix, sizeof(serverUnix)) == -1) {
		printf("Nie moglem zwiazac adresu unix z socketem!\n");
		exit(-5);
	}
	if (listen(unixSocket, SOMAXCONN) == -1) {
        close(unixSocket);
		printf("Problem z unix listen()!\n");
        exit(-7);
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

		int unixSocketClient = -1;
		int inetSocketClient = -1;

		if (clientsCounter < MAX_CLIENTS) {
            if (FD_ISSET(unixSocket, &readSocketsSet)) {
                if ((unixSocketClient = accept(unixSocket, NULL, NULL)) == -1)
                    printf("Nie moge zaakceptowac nowego polaczenia!\n");
            }
            //The accept() function shall extract the first connection on the queue of pending connections, 
            //create a new socket with the same socket type protocol and address family as the specified socket,
            //and allocate a new file descriptor for that socket
            if (FD_ISSET(inetSocket, &readSocketsSet) && !(clientsCounter + 1 == MAX_CLIENTS && unixSocketClient != -1)){
            	if ((inetSocketClient = accept(inetSocket, NULL, NULL)) == -1)
                    printf("Nie moge zaakceptowac nowego polaczenia!\n");
            }

            int i;
            //adding new client to list
            for (i = 0; i < MAX_CLIENTS && (unixSocketClient != -1 || inetSocketClient != -1); i++) {
                if (clients[i].sock == -1) {
                    int clientSocket;
                    if (unixSocketClient != -1) {
                        clientSocket = unixSocketClient;
                        unixSocketClient = -1;
                    } else {
                        clientSocket = inetSocketClient;
                        inetSocketClient = -1;
                    }
                    FD_SET(clientSocket, &socketsSet);
                    if (clientSocket > higherFD) {
                        higherFD = clientSocket;
                    }
                    clients[i].sock = clientSocket;
                    clients[i].lastSentData = timeOfReceivedData;
                    clientsCounter++;
                }
            }
        }
        else {
            if (FD_ISSET(unixSocket, &readSocketsSet)) {
                unixSocketClient = accept(unixSocket, NULL, NULL);
                close(unixSocketClient);
            }
            if (FD_ISSET(inetSocket, &readSocketsSet)) {
                inetSocketClient = accept(inetSocket, NULL, NULL);
                close(inetSocketClient);
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
        	//checking which client was added
            if (clients[i].sock == -1 || !FD_ISSET(clients[i].sock, &readSocketsSet)) {
                continue;
            }
            struct Data data;
            
            if(recv(clients[i].sock, &data, sizeof(data), 0) <= 0) {
            	printf("Usunalem klienta %s\n",clients[i].username);
                FD_CLR(clients[i].sock, &socketsSet);
    			close(clients[i].sock);
				clients[i].sock = -1;
				clients[i].username[0] = '\0';
				
				clientsCounter--;
                continue;
            }
            clients[i].lastSentData = timeOfReceivedData;
            strcpy(clients[i].username, data.username);
            sendDataToClients(data);
		}
	}
}

void sendDataToClients(struct Data data) {
	int j;
	for (j = 0; j < MAX_CLIENTS; j++) {
		if (clients[j].sock == -1 || strcmp(clients[j].username,data.username)==0)
			continue;
		if(send(clients[j].sock, &data, sizeof(data), 0) <= 0) {
			printf("Problem przy rozsylaniu wiadomosci zwrotnej!\n");
		}
	}
}

void checkingExpiratingClientsHandler(int signum) {
	struct timeval t;
	gettimeofday(&t, NULL);
	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].sock != -1 && clients[i].lastSentData.tv_sec + CONNECTION_TIMEOUT < t.tv_sec) {
			printf("Usunalem klienta %s\n",clients[i].username);
			FD_CLR(clients[i].sock, &socketsSet);
			close(clients[i].sock);
			
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
