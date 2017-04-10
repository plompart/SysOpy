#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>

static void handler(int signal);

int counter=0;

int main()
{
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

	for (;;)pause();
}


static void handler(int signal)
{
	int i;

	if(signal==SIGUSR1)
	{
		counter++;
	}
	else if(signal==SIGUSR2)
	{
		char line[PATH_MAX];

		pid_t pid;
		FILE *cmd = popen("pidof sender","r");

		fgets(line,PATH_MAX,cmd);

		if(!(pid=strtoul(line,NULL,10)))
		{
			printf("Nie ma programu sender!\n");
			pclose(cmd);
			exit(-41);
		}
		pclose(cmd);

		for(i=0;i<counter;i++)kill(pid,SIGUSR1);

		kill(pid,SIGUSR2);

		exit(-3);
	}

	return;
}