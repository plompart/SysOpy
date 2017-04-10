# the compiler
CC = gcc

# compiler flags
CFLAGS  = -Wall -o

all:	clean
	make progs
	
progs: 
	$(CC) $(CFLAGS) consumer consumer.c 
	$(CC) $(CFLAGS) host host.c 
	$(CC) $(CFLAGS) producer producer.c 
	
.PHONY: clean
clean:
	rm -f consumer host producer
