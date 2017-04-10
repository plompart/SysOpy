#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define MAX_LENGTH 100

void sigintHandler(int signal);

char* fifoPath;
int fifo;

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        printf("Podales za malo lub za duzo argumentow\n");
        exit(-1);
    }

    fifoPath=argv[1];
    char *buffer=malloc(MAX_LENGTH*sizeof(buffer));
    char bufferDay[20];
    char bufferTime[20];
    struct sigaction action;
    time_t readTime;

    action.sa_handler = sigintHandler;
    sigaction(SIGINT, &action, NULL);

    if((mkfifo(fifoPath,S_IRUSR | S_IWUSR))==-1)
    {
        printf("Nie moglem utworzyc FIFO!\n");
        exit(-2);
    }

    if((fifo=open(fifoPath, O_RDONLY))==-1)
    {
        printf("Nie mogłem otworzyć FIFO!\n");
        exit(-3);
    }

    while(1)
    {
        char c;
        int counter=0;        
        while (read(fifo, &c, 1) != 0 && counter<MAX_LENGTH-1)
        {
            if (c=='\n')break;
            buffer[counter++] = c;
        }
        buffer[counter]='\0';
        time(&readTime);
        if(strlen(buffer)>1)
        {
            strftime(bufferDay,20,"%F",localtime(&readTime));
            strftime(bufferTime,20,"%T",localtime(&readTime));
            printf("%s : %s %s\n",buffer,bufferDay,bufferTime);
        }
        sleep(1);
    }
}

void sigintHandler(int signal)
{
    if (fifo >= 0)close(fifo);
    unlink(fifoPath);
    exit(-4);
}