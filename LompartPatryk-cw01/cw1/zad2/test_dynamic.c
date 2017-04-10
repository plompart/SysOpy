#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/resource.h>
#include <dlfcn.h>
#include <time.h>
#include "LinkedList.h"

int main()
{
	void *handler;
	handler=dlopen("libList.so",RTLD_LAZY);
	if(!handler)exit(1);

	List *(*createList)(void);
	createList= dlsym(handler,"createList");
	void (*deleteList)(List *head);
	deleteList=dlsym(handler, "deleteList");
	void (*addElement)(List *head,Person *person);
	addElement=dlsym(handler,"addElement");
	void (*deleteElement)(List *head,Person *person);
	deleteElement=dlsym(handler,"deleteElement");
	Person *(*searchElement)(List *head,char *name,char *surname);
	searchElement=dlsym(handler,"searchElement");
	void(*sortList)(List *head);
	sortList=dlsym(handler,"sortList");
	Person *(*createPerson)(char *name, char *surname, 
		char *dateBirth, char *email, char *phone, char *address);
	createPerson=dlsym(handler,"createPerson");
	char *(*allocate)(char *a);
	allocate=dlsym(handler,"allocate");
	void (*setControlPoint)(void);
	setControlPoint=dlsym(handler,"setControlPoint");

	printf("Dynamicznie ladowana biblioteka poczatek programu");
	(*setControlPoint)();

	srand(time(NULL));
	char* names[]={"Adam","Bartosz","Cyprian","Danuta","Eustachy","Frania","Gabriel","Halina"};
	char* surnames[]={"Kowalski","Nowak","Job","Kolasa","Perdek","Slowik","Wisniewski","Rinner"};
	char* birthDates[]={"01-01-1991","02-02-1992","03-03-1993","04-04-1994",
                    "05-05-1995","06-06-1996","07-07-1997","08-08-1998"};
	char* emails[]={"adam.kowalski@gmail.com","bartosz.nowak@gmail.com","cyprian.job@gmail.com","danuta.kolasa@gmail.com",
                "eustachy.perdek@gmail.com","frania.slowik@gmail.com","gabriel.wisniewski@gmail.com","halina.rinner@gmail.com"};
	char* numbers[]={"111111111","222222222","333333333","444444444","555555555","666666666","777777777","888888888"};
	char* adresses[]={"Krakow","Warszawa","Plock","Gdynia","Rzeszow","Katowice","Wroclaw","Poznan"};

	int N=10000;

	List* head=(*createList)();
	int i=0;
	for(i=0;i<N;i++)
	{
		Person *person=(*createPerson)((*allocate)(names[rand()%8]),
					(*allocate)(surnames[rand()%8]),
					(*allocate)(birthDates[rand()%8]),
					(*allocate)(emails[rand()%8]),
					(*allocate)(numbers[rand()%8]),
					(*allocate)(adresses[rand()%8]));
		(*addElement)(head,person);
	}
	printf("Utworzona lista\n");
	(*setControlPoint)();

	(*sortList)(head);
	printf("Posortowane\n");
	(*setControlPoint)();

	for(i=0;i<1000;i++)
	{
		Person *searched=(*searchElement)(head,names[rand()%8],surnames[rand()%8]);
		(*deleteElement)(head,searched);
	}

	(*deleteList)(head);
	printf("Usunieta lista\n");
	(*setControlPoint)();

	dlclose(handler);
	return 0;
}
