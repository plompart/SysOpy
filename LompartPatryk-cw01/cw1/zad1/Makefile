static:
	gcc -Wall -Wextra -c LinkedList.c -o LinkedList.o
	ar rs libList.a LinkedList.o
	
shared:
	gcc -Wall -Wextra -c -fpic LinkedList.c
	gcc -shared -o libList.so LinkedList.o

clean:
	rm *.a *.so *.o
