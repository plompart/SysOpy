#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

void sortSys(char *path,int sizeR,int sizeF);
void sortLib(char *path,int sizeR,int sizeF);
void error(char *message,int errorNum);
int compare(char *a,char* b,int length);
void setPoint(double *user, double *sys);

int main(int argc,char* argv[])
{
	if(argc<4)
	{
		printf("Generowanie pliku nie powiodlo sie\n");
		printf("Podales za malo lub za duzo argumentow\n");
		exit(-1);
	}

	struct stat myStat;

	char *path=argv[1];
	char *sizeRecord=argv[2];
	char *mode=argv[3];
	char *bufor=calloc(300,1);
	char *buforInt=calloc(10,1);
	int sizeF;
	int sizeR=atoi(sizeRecord);
	double userStart,sysStart,userEnd,sysEnd;

	stat(path,&myStat);
	sizeF=myStat.st_size/(sizeR+1);


	setPoint(&userStart,&sysStart);

	if(!strcmp(mode,"sys"))sortSys(path,sizeR+1,sizeF);
		else if(!strcmp(mode,"lib"))sortLib(path,sizeR+1,sizeF);
		else error("bad argument",-16);

	setPoint(&userEnd,&sysEnd);
	printf("user:%f\tsystem:%f\n",userEnd-userStart,sysEnd-sysStart);

	FILE *file;
	if(!(file=fopen("wyniki.txt", "a")))error("wyniki.txt",-17);

	fwrite("size of record: ",1,16,file);
	fwrite(sizeRecord,1,strlen(sizeRecord),file);
	fwrite(" size of file: ",1,15,file);
	sprintf(buforInt,"%d",sizeF);
	fwrite(buforInt,1,10,file);
	fwrite(" mode: ",1,7,file);
	fwrite(mode,1,3,file);
	fwrite(" user: ",1,7,file);
	sprintf(buforInt,"%f",userEnd-userStart);
	fwrite(buforInt,1,10,file);
	fwrite(" system: ",1,9,file);
	sprintf(buforInt,"%f",sysEnd-sysStart);
	fwrite(buforInt,1,10,file);
	fwrite("\n",1,1,file);

	free(bufor);
	free(buforInt);
	fclose(file);
	return 0;
}

void sortSys(char *path,int sizeR,int sizeF)
{
	int handler=open(path,O_RDWR);

	if(handler<0)error(path,-2);

	int i;
	char *bufor1=malloc(sizeR);
	char *bufor2=malloc(sizeR);

	if(!bufor1 || !bufor2)error("Did not allocate memory",-3);

    for(i=1;i<sizeF;i++)
    {
        lseek(handler,i*sizeR,SEEK_SET);
        read(handler,bufor1,sizeR);
        
        int j=i-1;
        
        lseek(handler,j*sizeR,SEEK_SET);
        read(handler,bufor2,sizeR);
        
        int test=compare(bufor2,bufor1,sizeR);
        while(j>=0 && test==0)
        {
            lseek(handler,(j+1)*sizeR,SEEK_SET);
            write(handler,bufor2,sizeR);
            
            j--;

            lseek(handler,j*sizeR,SEEK_SET);
            read(handler,bufor2,sizeR);

            test=compare(bufor2,bufor1,sizeR);
        }
        
        lseek(handler,(j+1)*sizeR,SEEK_SET);
        write(handler,bufor1,sizeR);
    }

    free(bufor1);
    free(bufor2);
    close(handler);
}

void sortLib(char *path,int sizeR,int sizeF)
{
	FILE *file;
	if(!(file=fopen(path,"r+")))error("path",-11);

	int i;
	char *bufor1=malloc(sizeR);
	char *bufor2=malloc(sizeR);

	if(!bufor1 || !bufor2)error("Did not allocate memory",-12);

	for(i=1;i<sizeF;i++)
    {
        int j=i-1;

        fseek(file,i*sizeR,0);
        fread(bufor1,1,sizeR,file);
        
        fseek(file,j*sizeR,0);
        fread(bufor2,1,sizeR,file);

        int test=compare(bufor2,bufor1,sizeR);
        while(j>=0 && test==0)
        {
            fseek(file,(j+1)*sizeR,0);
            fwrite(bufor2,1,sizeR,file);
            
            j--;

            fseek(file,j*sizeR,0);
            fread(bufor2,1,sizeR,file);
            test=compare(bufor2,bufor1,sizeR);
        }

        fseek(file,(j+1)*sizeR,0);
        fwrite(bufor1,1,sizeR,file);
    }

	free(bufor1);
	free(bufor2);
	fclose(file);
}

int compare(char *a,char* b,int length)
{
    int i;

    for(i=0;i<length;i++)
    {
        if(a[i]>b[i])return 0;
        else if(a[i]<b[i])return 1;
    }
    return 0;
}

void setPoint(double *user,double *sys)
{
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);

    *user=rusage.ru_utime.tv_sec+rusage.ru_utime.tv_usec/(double)10e6;
    *sys=rusage.ru_stime.tv_sec+rusage.ru_stime.tv_usec/(double)10e6;
}

void error(char *message,int errorNum)
{
	perror(message);
	exit(errorNum);
}
