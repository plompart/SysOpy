#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]){
	if(argc!=2){
		printf("Podales zle argumeny!\n");
		exit(-1);
	}
	int recordLength=1024;
	int numRecords=atoi(argv[1]);
	int idSpot;
	int i,j;
	char buffer[recordLength];
	FILE* file;

	file=fopen("records.txt","w");
	
	for(i=1;i<=numRecords;i++){
		memset(buffer,'\0',recordLength);
		
		idSpot=snprintf(buffer,recordLength,"%d ",i);
		for(j=idSpot;j<recordLength-1;j++){
			buffer[j]='a'+rand()%26;
		}
		fprintf(file,buffer);
		fprintf(file,"\n");
	}

	fclose(file);

	return 0;
}