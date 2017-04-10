#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

static void handlerRT(int signal,siginfo_t *info,void* __);

int counter=0;

int main()
{
	struct sigaction action;

	action.sa_sigaction=handlerRT;
	sigfillset(&action.sa_mask);
	action.sa_flags=SA_SIGINFO;
	if((sigaction(SIGRTMIN+1,&action,NULL)) < 0)
	{
		perror("Nie moge zlapac \"SIGUSR1\"\n");
		exit(-1);
	}

	if((sigaction(SIGRTMIN+2,& action,NULL)) < 0)
	{
		perror("Nie moge zlapac \"SIGUSR2\"\n");
		exit(-2);   
	}

	for ( ; ; )pause();
}

static void handlerRT(int signal,siginfo_t *info,void* __)
{
	if(signal==SIGRTMIN+1)counter++;

	else if(signal==SIGRTMIN+2)
	{
		int i;

		for(i=0;i<counter;i++)sigqueue(info->si_pid,SIGRTMIN+1,(union sigval)NULL);

		sigqueue(info->si_pid,SIGRTMIN+2,(union sigval)NULL);

		exit(-3);
	}
}