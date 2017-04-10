#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_OF_PLANES 20
#define PRIORITY 5
#define CAPACITY 10

void* runInAir(void* parameters);
void* run(void* parameters);
void start(int id);
void fly();
void land(int id);
void stop();
void goSleep();

int planesOnAircraft=0;
int waitingForStart=0;
int waitingForLand=0;
int indexes[NUM_OF_PLANES];
pthread_mutex_t runway=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canStart=PTHREAD_COND_INITIALIZER;
pthread_cond_t canLand=PTHREAD_COND_INITIALIZER;

int main()
{
	int i;
	pthread_t planes[NUM_OF_PLANES];

	for(i=0;i<NUM_OF_PLANES;i++)
	{
		indexes[i]=i;
	}

	pthread_mutex_lock(&runway);
	if (CAPACITY<NUM_OF_PLANES)
	{
		for(i=0;i<CAPACITY;i++)
		{
			pthread_create(&planes[i],NULL,run,&i);
			planesOnAircraft++;
		}
		for(i=CAPACITY;i<NUM_OF_PLANES;i++)pthread_create(&planes[i],NULL,runInAir,indexes+i);
	}
	else
	{
		for(i=0;i<NUM_OF_PLANES;i++)
		{
			pthread_create(&planes[i],NULL,run,&i);
			planesOnAircraft++;
		}
	}
	pthread_mutex_unlock(&runway);

	for(i=0;i<NUM_OF_PLANES;i++)pthread_join(planes[i],NULL);

	pthread_cond_destroy(&canStart);
	pthread_cond_destroy(&canLand);
	pthread_mutex_destroy(&runway);
	
	return 0;
}

void* runInAir(void* parameters)
{
	int id=*((int*)parameters);

	fly();
	land(id);
	stop();

	run(&id);

	return NULL;
}

void* run(void* parameters)
{
	int id=*((int*)parameters);

	while(1)
	{
		start(id);
		fly();
		land(id);
		stop();
	}
}

void start(int id)
{
	printf("Plane %d wants to start!\n",id);
	pthread_mutex_lock(&runway);
	waitingForStart++;

	while(planesOnAircraft<PRIORITY && waitingForLand>0)pthread_cond_wait(&canStart,&runway);

	waitingForStart--;
	planesOnAircraft--;
	printf("Plane %d started. Planes on aircraft: %d.\n",id,planesOnAircraft);
	if(planesOnAircraft<PRIORITY)pthread_cond_signal(&canLand);
	else pthread_cond_signal(&canStart);
	pthread_mutex_unlock(&runway);
}

void fly()
{
	goSleep();
}

void land(int id)
{
	printf("Plane %d wants to land!\n",id);
	pthread_mutex_lock(&runway);
	waitingForLand++;

	while(planesOnAircraft==CAPACITY || (planesOnAircraft>=PRIORITY && waitingForStart>0))
	{
		pthread_cond_wait(&canLand,&runway);
	}
	waitingForLand--;
	planesOnAircraft++;
	printf("Plane %d landed. Planes on aircraft: %d.\n",id,planesOnAircraft);
	if(planesOnAircraft<PRIORITY)pthread_cond_signal(&canLand);
	else pthread_cond_signal(&canStart);
	pthread_mutex_unlock(&runway);
}

void stop()
{
	goSleep();
}

void goSleep()
{
	srand(time(NULL));
	sleep(rand()%5);
}