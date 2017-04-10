#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <wait.h>
#include <stdint.h>
#include <time.h>
#include <ftw.h>

int string_eq(char* s1, char*s2);

int main(int argc, char** argv)
{
	char *dirName,w,v,dirName2[PATH_MAX],*EXT;
	int i,sum,selfSum,childCounter,status;
	DIR *dir;
	struct dirent *dirent;
	struct stat myStat;

	dirName=NULL;
	w=v=0;
	for(i=1;i<argc;i++)
	{
		if(string_eq(argv[i],"-w")) w=1;
		else if(string_eq(argv[i],"-v")) v=1;
		else if(!dirName) dirName=argv[i];
  	}
	if(!dirName) dirName=".";

	dir=opendir(dirName);
	if(!dir)
	{
		_exit(0);
	}

	childCounter=0;
	sum=0;
	selfSum=0;

	for(dirent=readdir(dir);dirent;dirent=readdir(dir))
	{
		stat(dirName,&myStat);

		if(!string_eq(dirent->d_name,".") && !string_eq(dirent->d_name,".."))
		{
			if(S_ISDIR(myStat.st_mode))
			{
				sprintf(dirName2,"%s/%s",dirName,dirent->d_name);
				childCounter++;
				if(fork()==0)
				{
					if(w && v) execl(*argv,*argv,dirName2,"-w","-v",NULL);		//w execl wykonujemy program z podanymi parametrami w wywolaniu funkcji
					else if(w) execl(*argv,*argv,dirName2,"-w",NULL);
					else if(v) execl(*argv,*argv,dirName2,"-v",NULL);
					else execl(*argv,*argv,dirName2,NULL);
				}

				waitpid(-1,&status,0);
				sum+=WEXITSTATUS(status);
			}

			selfSum++;		
		}
		

	}

	if(w) sleep(15);

	sum+=selfSum;

	if(v)printf("Jestem w katalogu %s, jest tu %d plikow, razem z podkatalogami: %d\n",dirName,selfSum,sum);

	_exit(sum);
}

int string_eq(char* s1, char* s2)
{
	return strcmp(s1,s2)==0;
}

