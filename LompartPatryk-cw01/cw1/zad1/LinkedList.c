#include "LinkedList.h"

List *createList()
{
    List *head = (List*)malloc(sizeof(List));
    head->first = NULL;
    head->last = NULL;
    return head;
}

void deleteList(List *head)
{
    Member *first = head->first;

    if (first == NULL)return;
    while (first != NULL)
    {
        Member *tmp = first;
        first = first->next;
        free(tmp->person->name);
        free(tmp->person->surname);
        free(tmp->person->dateBirth);
        free(tmp->person->email);
        free(tmp->person->phone);
        free(tmp->person->address);
        free(tmp->person);
        free(tmp);
    }
}

void addElement(List *head, Person *person)
{
    Member *last = head->last;
    Member *newMem = (Member*)malloc(sizeof(Member));
    newMem->person = person;
    newMem->previous = newMem->next = NULL;
    if (last == NULL)
    {
        head->first = head->last = newMem;
        return;
    }

    last->next = newMem;
    newMem->previous = last;
    head->last = newMem;
}

void deleteElement(List *head, Person *person)
{
    if(head==NULL || person==NULL)return;
    Member *member=head->first;

    while (member!=NULL && member->person!=person)member = member->next;

    if(member==NULL)return;

    if(member==head->first)
    {
        if (member==head->last)
        {
            head->first = head->last = NULL;
        }
        else
        {
            head->first = head->first->next;
            head->first->previous = NULL;
        }
    }
    else
    {
        if (member == head->last)
        {
            head->last = head->last->previous;
            head->last->next = NULL;
        }
        else
        {
            member->previous->next = member->next;
		member->next->previous=member->previous;
        }
    }
	free(member->person->name);
	free(member->person->surname);
	free(member->person->dateBirth);
	free(member->person->email);
	free(member->person->phone);
	free(member->person->address);
	free(member->person);
	free(member);
}

Person *searchElement(List *head, char *name, char *surname)
{
    Member *tmp = head->first;
    while (tmp != NULL)
    {
        if (strcmp(tmp->person->name, name) == 0 && strcmp(tmp->person->surname, surname) == 0)
            return tmp->person;
        tmp=tmp->next;
    }
    return NULL;
}

void sortList(List *head)
{
    quickSort(head->first,head->last);
}

void quickSort(Member *first,Member *last)
{
    if (first!= NULL && last!=NULL && first!=last && first!=last->next)
    {
        Member *p = partition(first,last);
        quickSort(first, p->previous);
        quickSort(p->next,last);
    }
}

Member *partition(Member *first,Member *last)
{
    Person *pivot=last->person;
    Member *i=first->previous;
    Member *j;
    for(j=first;j!=last;j=j->next)
    {
        if(strcmp(j->person->surname,pivot->surname)<0)
        {
            i=(i==NULL) ? first : i->next;
            if(i!=j)swap(i,j);
        }
    }
    i =(i==NULL)? first : i->next;
    swap(i,last);
    return i;
}

void swap(Member *a,Member *b)
{
    Person *tmp=a->person;
    a->person=b->person;
    b->person=tmp;
}

void printPerson(Person *person)
{
    if (person == NULL)return;
    printf("%s %s %s %s %s %s\n", person->name, person->surname, person->dateBirth, person->email, person->phone, person->address);
}

void printList(List *head)
{
    Member *tmp = head->first;
    while (tmp != NULL)
    {
        printPerson(tmp->person);
        tmp = tmp->next;
    }
}

Person *createPerson(char *name, char *surname, char *dateBirth, char *email, char *phone, char *address)
{
    Person *person = (Person*)malloc(sizeof(Person));

    person->name = name;
    person->surname = surname;
    person->dateBirth = dateBirth;
    person->email = email;
    person->phone = phone;
    person->address = address;

    return person;
}

char *allocate(char *a)
{

    char *b=(char*)malloc(strlen(a)+1);
    strcpy(b,a);
    return b;
}

void printControlPoints(double real,double user,double system)
{
	printf("real:%fs\t uzytkownik:%fs\t system:%fs\n",real, user, system);
}

void setControlPoint()
{
	static int counter=0;
	static double real,user,system;
	static double real_first,user_first,system_first;
	static double real_before,user_before,system_before;
	struct rusage rusage;
	clock_t my_clock;

	counter+=1;

	if (counter>1)
	{
		real_before=real;
		user_before=user;
		system_before=system;
	}

	getrusage(RUSAGE_SELF,&rusage);
	my_clock=clock();

	real=my_clock/(double)CLOCKS_PER_SEC;
	user=rusage.ru_utime.tv_sec+rusage.ru_utime.tv_usec/(double)10e6;
	system=rusage.ru_stime.tv_sec+rusage.ru_stime.tv_usec/(double)10e6;

	if (counter==1)
	{
		real_first=real;
		user_first=user;
		system_first=system;
	}

	printf("\nPunkt kontrolny %d\n",counter);
	printControlPoints(real,user,system);

	if (counter>1)
	{
		printf("Roznica pomiedzy 1 i %d punktem:\n",counter);
		printControlPoints(real-real_first,user-user_first,system-system_first);
	}

	if (counter > 2)
	{
		printf("Roznica pomiedzy %d i %d punktem:\n",counter-1,counter);
		printControlPoints(real-real_before,user-user_before,system-system_before);
	}

	printf("\n");
}
