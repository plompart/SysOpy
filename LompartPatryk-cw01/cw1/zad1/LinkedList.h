#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>
 
typedef struct Person
{
    char *name;
    char *surname;
    char *dateBirth;
    char *email;
    char *phone;
    char *address;
}Person;
 
typedef struct Member
{
    Person *person;
    struct Member *previous;
    struct Member *next;
}Member;
 
typedef struct List
{
    Member *first;
    Member *last;
}List;
 
extern List *createList();
extern void deleteList(List *head);
extern void addElement(List *head, Person *person);
extern void deleteElement(List *head, Person *person);
extern Person *searchElement(List *head, char *name, char *surname);
extern void sortList(List *head);
extern void quickSort(Member *first,Member *last);
extern Member *partition(Member *first,Member *last);
extern void swap(Member *a,Member *b);
extern void printPerson(Person *person);
extern void printList(List *head);
extern Person *createPerson(char *name, char *surname, char *dateBirth, char *email, char *phone, char *address);
extern char *allocate(char *a);
extern void setControlPoint();
extern void printControlPoints(double real, double user, double system);
 
#endif