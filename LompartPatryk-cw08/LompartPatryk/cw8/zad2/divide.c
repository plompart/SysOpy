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

void* run(void*);
void* dividedRun(void*);

int main(){
	pthread_t thread1,thread2,dividedThread;

	pthread_create(&thread1,NULL,run,NULL);
	pthread_create(&thread2,NULL,run,NULL);
	pthread_create(&dividedThread,NULL,dividedRun,NULL);

	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	pthread_join(dividedThread,NULL);

	return 0;
}

void* dividedRun(void* args){
	int c;
	while(1){
		puts("Dziele przez 0");
		c=1/0;
		printf("%d\n",c);
	}
}

void* run(void* args){
	while(1){  
		puts("d");
	}
}