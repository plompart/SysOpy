#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

void options();
int getOption();
int getByte(int sizeF);
void lockRead(int file,int byte);
void lockWrite(int file,int byte);
void printLocks(int file,int sizeF);
void removeLock(int file,int byte);
char readByte(int file,int byte);
void writeByte(int file,int byte,char *newByte);
void error(char *message,int errorNum);

int main(int argc,char *argv[])
{
	if(argc<2)
	{
		printf("Generowanie pliku nie powiodlo sie\n");
		printf("Podales za malo lub za duzo argumentow\n");
		exit(-1);
	}

	char newByte;
	char *path=argv[1];
	struct stat myStat;

	int file;
	int option;
	int sizeF;
	int byte;

	if((file=open(path,O_RDWR))<0)error(path,-2);

	stat(path,&myStat);
	sizeF=(int)myStat.st_size;

	options();

	while((option=getOption()))
	{
		switch(option)
		{
			case 1:
				byte=getByte(sizeF);
				lockRead(file,byte);
				break;
			case 2:
				byte=getByte(sizeF);
				lockWrite(file,byte);
				break;
			case 3:
				printLocks(file,sizeF);			
				break;
			case 4:
				byte=getByte(sizeF);
				removeLock(file,byte);
				break;
			case 5:
				byte=getByte(sizeF);
				printf("\nRead byte: %c",readByte(file,byte));		
				break;
			case 6:
				byte=getByte(sizeF);
				printf("\nChoose character you want to write: ");
				scanf("%c",&newByte);
				scanf("%c", &newByte);
				writeByte(file,byte,&newByte);
				printf("\nDone");
				break;
			case 0:
				break;
		}
	}
	return 0;
}

void options()
{
	printf("\n1:Lock byte for reading");
    printf("\n2:Lock byte for writing");
    printf("\n3:List of locked bytes");
    printf("\n4:Remove lock");
    printf("\n5:Read byte");
    printf("\n6:Write byte");
    printf("\n0:Exit");
}

int getOption()
{
	int option;

	printf("\nChoose option: ");
	scanf("%d",&option);
	if(option<0 || option>6)
	{
		printf("\nWrong argument!");
		return getOption();
	}

	return option;
}

int getByte(int sizeF)
{
	int byte;

	printf("\nChoose byte: ");
	scanf("%d",&byte);
	if(byte<0 || byte>sizeF)
	{
		printf("\nWrong argument!");
		return getByte(sizeF);
	}

	return byte;
}

void lockRead(int file,int byte)
{
    struct flock lock;

    lock.l_type=F_RDLCK;
    lock.l_start=byte;
    lock.l_whence=SEEK_SET;
    lock.l_len=1;

    if (fcntl(file,F_SETLK,&lock)==-1)error("readlock",-3);
    else printf("\nRead lock for byte %d was set",byte);
}

void lockWrite(int file,int byte)
{
	struct flock lock;

    lock.l_type=F_WRLCK;
    lock.l_start=byte;
    lock.l_whence=SEEK_SET;
    lock.l_len=1;

    if (fcntl(file,F_SETLK,&lock)==-1)error("writelock",-4);
    else printf("\nWrite lock for byte %d was set",byte);
}

void printLocks(int file,int sizeF)
{
    int i;

    printf("\nLocked bytes:");

    for (i=0;i<sizeF;i++)
    {
        struct flock lock;

        lock.l_type=F_WRLCK;
        lock.l_start=i;
        lock.l_whence=SEEK_SET;
        lock.l_len=1;
        fcntl(file,F_GETLK,&lock);

        if (lock.l_type!=F_UNLCK)printf("\nByte %d locked by process: %d",i,lock.l_pid);
    }
}

void removeLock(int file,int byte)
{
    struct flock lock;

    lock.l_type=F_UNLCK;
    lock.l_start=byte;
    lock.l_whence=SEEK_SET;
    lock.l_len=1;

    if (fcntl(file, F_SETLK, &lock)==-1)error("remove lock",-5);
	else printf("\nLock for byte %d was removed", byte);
}

char readByte(int file,int byte)
{
	char tmp;
    lseek(file,byte-1,SEEK_SET);
    read(file,&tmp,1);

    return tmp;
}

void writeByte(int file,int byte,char *newByte)
{
	lseek(file,byte-1,SEEK_SET);
    write(file,newByte,1);
}

void error(char *message,int errorNum)
{
	perror(message);
	exit(errorNum);
}