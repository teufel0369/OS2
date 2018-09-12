CC=gcc
CFLAGS = -g -Wall

all: ass1

ass1: main.o
	$(CC) $(CFLAGS) -o ass1 main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

.PHONY: clean

clean:
	rm *.o ass1 
