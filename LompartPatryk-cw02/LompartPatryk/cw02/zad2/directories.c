#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <stdint.h>

char *rights;

void relativeDir(char *path,char *rights);
void absoluteDir(char *path);
void error(char *message,int errorNum);
int printPath(char *path,struct stat *my_stat,int flag);

int main(int argc,char *argv[])
{
	if(argc<3)
	{
		printf("Generowanie pliku nie powiodlo sie\n");
		printf("Podales za malo lub za duzo argumentow\n");
		exit(-1);
	}

	char *path=argv[1];
	rights=argv[2];

	printf("Relative paths: \n");
	relativeDir(path,rights);

	printf("Absolute paths: \n");
	absoluteDir(path);

	return 0;
}

void relativeDir(char *path,char *rights)
{
	DIR *directory;
	struct dirent *dirent;
	

	char *wholePath=malloc(4096);
	char *fileRights=malloc(3);

	if(!(directory=opendir(path)))error(path,-2);

	while((dirent=readdir(directory)))
	{
		struct stat myStat;

		if((sprintf(wholePath,"%s/%s",path,dirent->d_name))<0)error("wholePath",-3);

		stat(wholePath,&myStat);

		if(sprintf(fileRights,"%o",myStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO))<0)error("fileRights",-4);

		if((S_ISDIR(myStat.st_mode)))
		{
			if(strcmp(dirent->d_name,".")!=0 && strcmp(dirent->d_name,"..")!=0)relativeDir(wholePath,rights);
		}else 
		if(S_ISREG(myStat.st_mode) && strcmp(rights,fileRights)==0 && strcmp(dirent->d_name,".")!=0 && strcmp(dirent->d_name,"..")!=0)
		{
			char charTime[128];
			struct tm tmp;
			localtime_r(&myStat.st_atime, &tmp);
			strftime(charTime, sizeof(charTime), "%c", &tmp);

			printf("%s ",wholePath);
			printf(" size: %lld last modified: %s\n",(long long)myStat.st_size,charTime);
		}
	}

	free(wholePath);
	free(fileRights);
	closedir(directory);
}

void absoluteDir(char *path)
{
	nftw(path,( __nftw_func_t)printPath,64,FTW_F);
}

int printPath(char *path,struct stat *myStat,int flag)
{
	char *absoluteDir=malloc(4096);
	char *fileRights=malloc(3);
	if (sprintf(fileRights, "%o", myStat->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) < 0)error("",-6);

	if (flag == FTW_F && strcmp(fileRights, rights)==0)
	{
		if (!realpath(path, absoluteDir))error("",-7);

		char charTime[128];
		struct tm tmp;
		localtime_r(&myStat->st_atime, &tmp);
		strftime(charTime, sizeof(charTime), "%c", &tmp);

		printf("%s ",absoluteDir);
		printf(" size: %lld last modified: %s\n",(long long)myStat->st_size,charTime);
	}

	free(fileRights);
	free(absoluteDir);
	return 0;
}

void error(char *message,int errorNum)
{
	perror(message);
	exit(errorNum);
}