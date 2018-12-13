#pragma once
#ifndef O2_THOMPSON_3_SHARED_H
#define O2_THOMPSON_3_SHARED_H
#define SHARED_MEM_CLOCK_KEY 1775
#define SHARED_MEM_STATS_KEY 72912
#define USER_PROCESS_KEY 4604
#define QUEUE_KEY 1535
#define MASTER_ID 369


#define DEFAULT_NUM_PROCESSES 18

#include <stdlib.h>
#include <stdbool.h>

typedef struct SharedClock {
    unsigned int seconds;
    unsigned int nanoSeconds;
} SharedMemClock;

typedef struct Process {
    pid_t actualPid;
    int indexNumber;
} Process;

typedef struct Pages {
    int occupied;
    int dirty;
    int location;
} Pages;

typedef struct PageTable {
    Pages pages[18][32];
} PageTable;

typedef struct Reference {
    int pageNumber;
    int offset;
} Reference;

typedef struct Message {
    long type;
    pid_t pid;
    int index;
    int dirty;
    int terminateFromMaster;
    int childProcessTerminating;
    int messageTestMaster;
    int messageTestChild;
    int passedCommChecks;
    Reference ref;
    int isScheduled;
} Message;

typedef struct User {
    pid_t pid;
    int index;
    int scheduled;
    int isActive;
    int terminated;
} UserProcess;

typedef struct Queue {
    Message* array;
    int front;
    int rear;
    int size;
    unsigned int queueCapacity;
} Queue;

typedef struct Frames {
    int dirty;
    int index;
    int used;
    int pageNumber;
    int offset;
} Frames;

typedef struct TotalProcesses {
    int activeProcesses;
    int totalExecuted;
} ProcessStats;

#endif //O2_THOMPSON_4_SHARED_H