#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>

#define RECORD_SIZE 1024

int currentRecord=0;
pthread_mutex_t mutex;

typedef struct parameters{
	int version;
	int file;
	int readRecords;
	int threadsNum;
	char* searchedWord;
	pthread_t* threads;
}parameters;

void* run(void*);

int main(int argc, char* argv[]){
	if(argc!=6){
		printf("Podales zle argumenty!\n");
		exit(-1);
	}

	int threadsNum=atoi(argv[1]);
	int readRecords=atoi(argv[3]);
	int version=atoi(argv[5]);
	char* filename=argv[2];
	char* searchedWord=argv[4];

	pthread_t* threads=(pthread_t*)malloc(threadsNum*sizeof(pthread_t));

	int file=open(filename,O_RDONLY);
	if(file==-1){
		printf("Nie moglem otworzyc pliku!\n");
		exit(-2);
	}

	if(pthread_mutex_init(&mutex, NULL)!=0){
		printf("Nie moglem zainicjalizowac mutexa!\n");
		exit(-3);
	}
	parameters* params=(parameters*)malloc(sizeof(parameters));
	params->version=version;
	params->file=file;
	params->readRecords=readRecords;
	params->threadsNum=threadsNum;
	params->searchedWord=searchedWord;
	params->threads=threads;

	int i;
	for(i=0;i<threadsNum;i++){
		if(0!=pthread_create(&threads[i],NULL,&run,(void*)params)){
			printf("Nie moglem utworzyc watku!\n");
			exit(i);
		}
	}

	for(i=0;i<threadsNum;i++){
		if(0!=pthread_join(threads[i],NULL)){
			printf("Wystapil blad przy oczekiwaniu na zakonczenie watku!\n");
			exit(i);
		}
	}

	close(file);

	return 0;
}

void* run(void* args){
	parameters* params=(parameters*)args;
	char buffer[RECORD_SIZE];
	char* recordID=calloc(4,sizeof(char));
	char keys[]=" ";
	int canRead=1;
	int found;
	int i;

	int temp;
	switch(params->version){
		case 1:
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &temp);
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &temp);
			break;
		case 2:
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &temp);
			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &temp);
			break;
		case 3:
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&temp);
			break;
		default:
			printf("Podales zla wersje programu!\n");
			exit(-4);
	}

	while(canRead){
		found=0;
		pthread_mutex_lock(&mutex);
		for(i=0;i<params->readRecords;i++){
			if(0==read(params->file,buffer,RECORD_SIZE)){
				canRead=0;
				break;
			}
			else{
				
				if(NULL!=strstr(buffer,params->searchedWord)){
					strncpy(recordID,buffer,strcspn(buffer,keys));
					printf("TID: %d  Record ID: %s\n",(int)pthread_self(),recordID);
					found=1;
					break;
				}
			}
		}
		pthread_mutex_unlock(&mutex);
		if(!canRead)break;

		if(params->version==2) pthread_testcancel();

		if(found && (params->version==1 || params->version==2)){
			for(i=0; i<params->threadsNum; i++){
				if(pthread_self() != params->threads[i]){
					pthread_cancel(params->threads[i]);
				}
			}
			canRead=0;
		}

		sleep(1);
	}

	return NULL;
}