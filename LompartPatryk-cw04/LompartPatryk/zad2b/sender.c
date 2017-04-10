#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>


static void handler(int signal);

int counter=0;
int signalNum=0;

int main(int argc,char *argv[])
{
	if(argc<2)
	{
		printf("Generowanie pliku nie powiodlo sie\n");
		printf("Podales za malo lub za duzo argumentow\n");
		exit(-1);
	}

	if (signal(SIGUSR1,handler)==SIG_ERR)
	{
		perror("Nie moge zlapac SIGUSR1!\n");
		exit(-1);
	}
	if (signal(SIGUSR2,handler)==SIG_ERR)
	{
		perror("Nie moge zlapac SIGUSR2!\n");
		exit(-2);
	}

	signalNum=atoi(argv[1]);


	char line[PATH_MAX];

	pid_t pid;
	FILE *cmd=popen("pidof catcher","r");

	fgets(line,PATH_MAX,cmd);

	if(!(pid=strtoul(line,NULL,10)))
	{
		printf("Nie ma programu catcher!\n");
		pclose(cmd);
		exit(-3);
	}
	pclose(cmd);

	kill(pid,SIGUSR1);

	for (;;)pause();
}

static void handler(int signal)
{
	char line[PATH_MAX];
	
	FILE *cmd=popen("pidof catcher","r");
	pid_t pid;

	fgets(line,PATH_MAX,cmd);

	if(!(pid=strtoul(line,NULL,10)) && signalNum!=-1)
	{
		printf("Nie ma programu catcher!\n");
		pclose(cmd);
		exit(-4);
	}
	pclose(cmd);

	if(signal==SIGUSR1)
	{
		signalNum--;
		if(signalNum>0)
		{

			kill(pid,SIGUSR1);
		}
		else if(signalNum==0)
		{		
			signalNum=-1;
			kill(pid,SIGUSR2);
		}
		else
		{
						printf("a\n");
			counter++;
		}

	}
	else if(signal==SIGUSR2)
	{
		printf("Sender:%d sygnalow SIGUSR1\n",counter);
		exit(-5);
	}
	
	return;
}
