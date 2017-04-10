# the compiler
CC = gcc

# compiler flags
CFLAGS  = -Wall -o

all:	clean
	make progs
	
progs: 
	$(CC) $(CFLAGS) reader reader.c 
	$(CC) $(CFLAGS) host host.c 
	$(CC) $(CFLAGS) writer writer.c 
	
.PHONY: clean
clean:
	rm -f reader host writer
