#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        printf("Podales za malo lub za duzo argumentow\n");
        exit(-1);
    }

    char* width=argv[1];
    int pipefd[2];
    pid_t pidTr;
    pid_t pidFold;

    if(pipe(pipefd)==-1)
    {
        printf("Nie mogłem utworzyć pipe'ow!");
        exit(-2);
    }

    if((pidTr=fork())==-1)
    {
        printf("Nie mogłem utworzyć procesu!");
        exit(-3);
    }
    if(pidTr==0) //tr process
    {
        if((pidFold=fork())==-1)
        {
            kill(pidTr,SIGINT);
            printf("Nie mogłem utworzyć procesu!");
            exit(-4); 
        }
        if(pidFold==0) //fold process
        {
            dup2(pipefd[0],0);
            close(pipefd[0]);
            close(pipefd[1]);
            execlp("fold","fold","-w",width,NULL);
        }
        dup2(pipefd[1],1);
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("tr","tr,","'[:lower:]'","'[:upper:]'",NULL);
    }
    else
    {
        close(pipefd[0]);
        close(pipefd[1]);
        wait(NULL);
        wait(NULL);
    }

    return 0;
}