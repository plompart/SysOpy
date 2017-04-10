#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

void* run(void* index);

sem_t forks[5];
int indexes[]={0,1,2,3,4};

int main()
{
	pthread_t philosophers[5];

	int i;
	for(i=0;i<5;i++)
	{
		//second parameter is describing whether semaphore is shared 
		//between threads of process or between processes
		sem_init(&forks[i],0,1);
	}

	for(i=0;i<5;i++)pthread_create(&philosophers[i],NULL,run,indexes+i);

	for(i=0;i<5;i++)pthread_join(philosophers[i],NULL);

	for(i=0;i<5;i++)sem_destroy(&forks[i]);
	
	return 0;
}

void* run(void* parameters)
{
	int index=*((int*)parameters);
	int priorityFork,secondFork;

	if(index==4)
	{
		priorityFork=index;
		secondFork=0;
	}
	else
	{
		priorityFork=index+1;
		secondFork=index;
	}

	while(1)
	{
		printf("Philosopher %d is waiting.\n",index);
		sem_wait(&forks[priorityFork]);
		sem_wait(&forks[secondFork]);
		printf("Philosopher %d is eating.\n",index);
		sleep(1);
		sem_post(&forks[priorityFork]);
		sem_post(&forks[secondFork]);
		printf("Philosopher %d is thinking.\n",index);
		sleep(1);
	}
}