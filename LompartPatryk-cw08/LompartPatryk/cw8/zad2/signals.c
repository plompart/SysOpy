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
#include <stdbool.h>
#include <signal.h>

typedef struct parameters{
	int version;
	int sig;
}parameters;

void handler(int sig);
void* run(void*);

int main(int argc, char* argv[]){
	if(argc!=3){
		printf("Podales zle parametry!\n");
		exit(-1);
	}

	int version=atoi(argv[1]);
	int sig=atoi(argv[2]);
	pthread_t thread;

	switch(sig){
		case 1:
			sig=SIGUSR1;
			break;
		case 2:
			sig=SIGTERM;
			break;
		case 3:
			sig=SIGKILL;
			break;
		case 4:
			sig=SIGSTOP;
			break;
		default:
			printf("Podales zle parametry!\n");
			break;
	}

	if(2==version){
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, sig);
		if(0!=sigprocmask(SIG_SETMASK, &set, NULL)){
			printf("Nie moglem ustawic maski watku glownego!\n");
			exit(-2);
		}
	}

	parameters* params=malloc(sizeof(parameters));
	params->version=version;
	params->sig=sig;

	if(0!=pthread_create(&thread,NULL, &run, (void*)params)){
		printf("Nie moglem utworzyc watku\n");
		exit(-3);
	}

	sleep(1);

	if(3==version){
		signal(sig,handler);
	}

	if(4==version || 5==version){
		pthread_kill(thread, sig);
	}

	if (3==version || 2==version || 1==version)raise(sig);
	
	pthread_join(thread, NULL);

	return 0;
}

void handler(int sig) {
	printf("PID = %d TID = %d\n", getpid(), (int)pthread_self());
}

void* run(void* args){
	parameters* params=(parameters*)args;

	if(params->version==3 || params->version==5)signal(params->sig,handler);
	if(params->version==4){
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, params->sig);
		sigprocmask(SIG_SETMASK, &set, NULL);
	}

	for(;;);
	return NULL;
}