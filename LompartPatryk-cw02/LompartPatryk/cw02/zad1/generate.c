#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void clear(char *bufor);

int main(int argc, char *argv[])
{
	srand(time(NULL));

	if(argc<4)
	{
		printf("Generowanie pliku nie powiodlo sie\n");
		printf("Podales za malo lub za duzo argumentow\n");
		exit(-1);
	}
	
	char *path=argv[1];
	char *sizeRecord=argv[2];
	char *sizeFile=argv[3]; 

	int sizeR=atoi(sizeRecord);
	int sizeF=atoi(sizeFile);
	int i,j;
	
	FILE *file;
	
	if(!(file=fopen(path,"w+")))
	{
		perror(path);
		exit(-2);
	}
	
	for(i=0;i<sizeF;i++)
	{
		char *bufor=malloc(sizeR);

		for(j=0;j<sizeR;j++)
		{
			bufor[j]=(char)(rand()%74+48);
		}
		fwrite(bufor,1,sizeR,file);
		fwrite("\n",1,1,file);
		free(bufor);
	}
	
	fclose(file);
	return 0;
}

void clear(char *bufor)
{
	if(bufor==NULL)return;
	
	int size=strlen(bufor+1);
	int i;
	for(i=0;i<size;i++)
	{
		*(bufor+i)='\0';
	}
}

