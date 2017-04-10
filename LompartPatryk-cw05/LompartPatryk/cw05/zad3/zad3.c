#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        printf("Podales za malo lub za duzo argumentow\n");
        exit(-1);
    }

    char *filename=argv[1];
    int output=open(filename,O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    if(output==-1)
    {
        printf("Nie moglem otworzyc pliku!\n");
        exit(-2);
    }

    dup2(output,1);
    close(output);

    FILE *file=popen("ls -l | grep ^d", "w");
    if(file==NULL)
    {
        printf("Nie moglem utworzyc polecenia!\n");
    }
    pclose(file);
    close(1);

    return 0;
}
