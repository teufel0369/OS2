CC=gcc
CFLAGS = -g -Wall

WORKER_SOURCE=worker.c
WORKER_OBJECTS=$(WORKER_SOURCE:.c=.o)
WORKER_EXECUTABLE=worker

MASTER_SOURCE=master.c
MASTER_OBJECTS=$(MASTER_SOURCE:.c=.o)
MASTER_EXECUTABLE=master

HEADER=shared.h

all: $(WORKER_EXECUTABLE) $(MASTER_EXECUTABLE)

$(WORKER_EXECUTABLE): $(WORKER_OBJECTS)
	$(CC) $(WORKER_OBJECTS) -o $(WORKER_EXECUTABLE)

$(MASTER_EXECUTABLE): $(MASTER_OBJECTS)
	$(CC) $(MASTER_OBJECTS) -o $(MASTER_EXECUTABLE)

%.o: %.c $(HEADER)
	$(CC) -c $(CFLAGS) $*.c -o $*.o

.PHONY: clean

clean:
	rm *.o $(WORKER_EXECUTABLE) $(MASTER_EXECUTABLE)