#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define MAX_LENGTH 100

void sigintHandler(int signal);

char *fifo_path;
int fifo;

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        printf("Podales za malo lub za duzo argumentow\n");
        exit(-1);
    }

    char *fifoPath=argv[1];
    char *buffer=malloc(MAX_LENGTH*sizeof(buffer));
    char *finalBuffer=malloc(MAX_LENGTH*sizeof(finalBuffer));
    char bufferDay[20];
    char bufferTime[20];
    struct sigaction action;
    time_t writtenTime;

    action.sa_handler = sigintHandler;
    sigaction(SIGINT, &action, NULL);

    if((fifo=open(fifoPath, O_WRONLY))==-1)
    {
        printf("Nie mogłem otworzyć FIFO!");
        exit(-3);
    }

    while(1)
    {
        fgets(buffer,MAX_LENGTH,stdin);
        time(&writtenTime);
        strftime(bufferDay,20,"%F",localtime(&writtenTime));
        strftime(bufferTime,20,"%T",localtime(&writtenTime));
        sprintf(finalBuffer,"%d : %s %s : %s ",getpid(),bufferDay,bufferTime,buffer);

        if(finalBuffer[MAX_LENGTH-1]!='\0')finalBuffer[MAX_LENGTH-1]='\n';

        if((write(fifo,finalBuffer,strlen(finalBuffer)))!=strlen(finalBuffer))
        {
            printf("Nie moglem wyslac komunikatu!\n");
            close(fifo);
            exit(-5);
        }
    }
    free(buffer);
    free(bufferTime);
    close(fifo);

    return 0;
}

void sigintHandler(int signal)
{
    if (fifo>=0)close(fifo);
    exit(-4);
}
