#define _GNU_SOURCE
#define _XOPEN_SOURCE 9000
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <dlfcn.h>
#include <sys/times.h>
#include <time.h>
#include <dlfcn.h>
#define CHILD_STACK_SIZE 0x800

void createChildsFork(int NUM_OF_CHILDS);
void createChildsvFork(int NUM_OF_CHILDS);
void createChildsClone(int NUM_OF_CHILDS);
void createChildsvClone(int NUM_OF_CHILDS);

int increment(void* _);
void printTimes(int NUM_OF_CHILDS,clock_t childrenRealTime,clock_t previousTime, 
	struct tms* tms0, clock_t currentTime,struct tms* tms1);

int Counter;

int main(int argc, char *argv[])
{
	if(argc<3)
	{
		printf("Generowanie pliku nie powiodlo sie\n");
		printf("Podales za malo lub za duzo argumentow\n");
		exit(-1);
	}


	int NUM_OF_CHILDS=atoi(argv[1]);

	switch(atoi(argv[2]))
	{
		case 1:
			createChildsFork(NUM_OF_CHILDS);
			break;
		case 2:
			createChildsvFork(NUM_OF_CHILDS);
			break;
		case 3:
			createChildsClone(NUM_OF_CHILDS);
			break;
		case 4:
			createChildsvClone(NUM_OF_CHILDS);
			break;
	}

	return 0;
}

void createChildsFork(int NUM_OF_CHILDS)
{
	struct tms tms0,tms1;
	clock_t previousTime,currentTime;
	clock_t childrenRealTime=0;
	int i,status;
	int Counter=0;
	
	previousTime=times(&tms0);
	for(i=0;i<NUM_OF_CHILDS;i++)
	{
		if(fork()==0)					//fork zwraca 0 do procesu potomnego, a do macierzystego zwraca numer pidu, -1 jesli fail
		{
			clock_t x=times(NULL);
			Counter++;
			_exit(times(NULL)-x);
		}

		waitpid(-1,&status,0);
		childrenRealTime+=WEXITSTATUS(status);
	}
	currentTime=times(&tms1);

	printf("\n%d",Counter);

	printTimes(NUM_OF_CHILDS,childrenRealTime,previousTime,&tms0,currentTime,&tms1);	
}

void createChildsvFork(int NUM_OF_CHILDS)
{
	struct tms tms0,tms1;
	clock_t previousTime,currentTime;
	clock_t childrenRealTime=0;
	int i,status;
	int Counter=0;
	
	previousTime=times(&tms0);
	for(i=0;i<NUM_OF_CHILDS;i++)
	{
		if(vfork()==0)
		{
			clock_t x=times(NULL);
			Counter++;
			_exit(times(NULL)-x);
		}

		waitpid(-1,&status,0);
		childrenRealTime+=WEXITSTATUS(status);
	}
	currentTime=times(&tms1);

	printf("\n%d",Counter);

	printTimes(NUM_OF_CHILDS,childrenRealTime,previousTime,&tms0,currentTime,&tms1);
}

void createChildsClone(int NUM_OF_CHILDS)
{
	struct tms tms0,tms1;
	clock_t previousTime,currentTime;
	clock_t childrenRealTime=0;
	void *stack;
	int i,status;
	int Counter=0;

	stack=malloc(CHILD_STACK_SIZE);
	previousTime=times(&tms0);
	for(i=0;i<NUM_OF_CHILDS;i++)
	{
		clone(increment,stack+CHILD_STACK_SIZE, SIGCHLD,NULL,NULL,NULL,NULL);

		waitpid(-1,&status,0);

		childrenRealTime+=WEXITSTATUS(status);
	}
	currentTime=times(&tms1);

	printf("\n%d",Counter);

	printTimes(NUM_OF_CHILDS,childrenRealTime,previousTime,&tms0,currentTime,&tms1);
}

void createChildsvClone(int NUM_OF_CHILDS)
{
	struct tms tms0,tms1;
	clock_t previousTime,currentTime;
	clock_t childrenRealTime=0;
	void *stack;
	int i,status;
	int Counter=0;

	stack=malloc(CHILD_STACK_SIZE);
	previousTime=times(&tms0);
	for(i=0;i<NUM_OF_CHILDS;i++)
	{
		clone(increment,stack+CHILD_STACK_SIZE,CLONE_VM | CLONE_VFORK | SIGCHLD,NULL,NULL,NULL,NULL);		//w clone 1szy arg to wskaznik na funkcje, potem stos dla dzieci,flagi CLONE_VM-macierzysty i potomny pracuja w tej samej przestrzeni pamieci,clonem_vfork- proces potomny sie nie skonczy dopoki nie zwolni wirtualnej pamieci

		waitpid(-1,&status,0);

		childrenRealTime+=WEXITSTATUS(status);
	}
	currentTime=times(&tms1);

	printf("\n%d",Counter);

	printTimes(NUM_OF_CHILDS,childrenRealTime,previousTime,&tms0,currentTime,&tms1);
}

int increment(void* _)
{
	clock_t x=times(NULL);
	Counter++;
	_exit(times(NULL)-x);

}

void printTimes(int NUM_OF_CHILDS,clock_t childrenRealTime,clock_t previousTime, 
	struct tms* tms0, clock_t currentTime,  struct tms* tms1)
{
  long ticksperse = sysconf(_SC_CLK_TCK);
  double pReal,pUser,pSystem,chReal,chUser,chSys;
  pReal=(double)(currentTime-previousTime)/ticksperse;
  pUser=(double)(tms1->tms_utime-tms0->tms_utime)/ticksperse;
  pSystem=(double)(tms1->tms_stime-tms0->tms_stime)/ticksperse;
  chReal=(double)(childrenRealTime)/ticksperse;
  chUser=(double)(tms1->tms_cutime-tms0->tms_cutime)/ticksperse;
  chSys=(double)(tms1->tms_cstime-tms0->tms_cstime)/ticksperse;

  printf("\n%d  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf  %lf %lf  \n",
         NUM_OF_CHILDS,
         
         pReal,
         pUser,
         pSystem,
         pUser+pSystem,
         
         chReal,
         chUser,
         chSys,
         chUser+chSys,

         pReal+chReal,
         pUser+chUser,
         pSystem+chSys,
         pUser+pSystem+chUser+chSys);
}