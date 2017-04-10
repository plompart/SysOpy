#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

static void handlerRT(int signal,siginfo_t *info,void* _);

int counter=0;

int main(int argc,char *argv[])
{
	if(argc<2)
	{
		printf("Generowanie pliku nie powiodlo sie\n");
		printf("Podales za malo lub za duzo argumentow\n");
		exit(-1);
	}

	int i;
	int signalNum=atoi(argv[1]);
	char line[PATH_MAX];

	pid_t pid;
	FILE *cmd=popen("pidof catcher","r");
	struct sigaction action;


	action.sa_sigaction=handlerRT;
	sigfillset(&action.sa_mask);
	action.sa_flags=SA_SIGINFO;

	if((sigaction(SIGRTMIN+1,&action,NULL))<0)
	{
		perror("Nie moge zlapac \"SIGUSR1\"\n");
		exit(-1);
	}

	if((sigaction(SIGRTMIN+2,&action,NULL))<0)
	{
		perror("Nie moge zlapac \"SIGUSR2\"\n");
		exit(-2);   
	}

	fgets(line,PATH_MAX,cmd);

	if(!(pid=strtoul(line,NULL,10)))
	{
		printf("Nie ma programu catcher!\n");
		pclose(cmd);
		exit(-23);
	}

	pclose(cmd);

	for(i=0;i<signalNum;i++)
	{
		sigqueue(pid,SIGRTMIN+1,(union sigval)NULL);
	}

	sigqueue(pid,SIGRTMIN+2,(union sigval)NULL);

	for(;;)pause();	
}

static void handlerRT(int signal,siginfo_t *info,void* _)
{
	if(signal==SIGRTMIN+1)
	{
		counter++;
	}
	else if(signal==SIGRTMIN+2)
	{
		printf("Sender:%d sygnalow SIGRTUSR+1\n",counter);
		exit(-1);
	}
}