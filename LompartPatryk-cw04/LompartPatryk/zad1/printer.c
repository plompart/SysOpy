#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

void handlerSIGINT(int signal);
void handlerSIGSTP(int signal);
void printText();
void loop(char* text);

int MAX_NUM;
int counter=0;
bool incrementing=true;
char *text;
char *textBackwords;

int main(int argc,char *argv[])
{
    if(argc<3)
    {
        printf("Generowanie pliku nie powiodlo sie\n");
        printf("Podales za malo lub za duzo argumentow\n");
        exit(-1);
    }

    char *tmp=malloc(sizeof(argv[1]));
    int i,j=0;

    MAX_NUM=2*atoi(argv[2]);
    text=argv[1];
    struct sigaction action;

    for(i=strlen(text)-1;i>=0;i--)
    {
        tmp[j]=text[i];
        j++;
    }

    textBackwords=tmp;

    action.sa_handler=handlerSIGSTP;
    action.sa_flags=SA_SIGINFO;

    if(sigaction(SIGTSTP,&action,NULL)==-1)
    {
        perror("Nie moge zlapac SIGSTP!\n");
        exit(-2);
    }

    if(signal(SIGINT,handlerSIGINT)==SIG_ERR) 
    {
        perror("Nie moge zlapac SIGINT!\n");
        exit(-3);
    }

    for(;;)pause();

    free(tmp);
    return 0;
}

void handlerSIGINT(int signal)
{
    printf("\nOdebrano sygnal SIGINT.\n\n");
    exit(-4);
}

void handlerSIGSTP(int signal)
{
    if(incrementing)
    {
        if(counter==MAX_NUM)
        {
            incrementing=false;
            counter--;
        } else 
        {
            counter++;
        }
    } else 
    {
        if(counter==1)
        {
            incrementing=true;
            counter++;
        } else 
        {
            counter--;
        }
    }
    printText();
}

void printText()
{
    if(counter%2==0)loop(text);
    else loop(textBackwords);
}

void loop(char *text)
{
    int i;
    for(i=counter/2 + counter%2;i>0;i--)
    {
        printf("\n%s\t",text);
    }
    printf("\n");
}