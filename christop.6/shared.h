#pragma once
#ifndef O2_THOMPSON_3_SHARED_H
#define O2_THOMPSON_3_SHARED_H
#define SHARED_MEM_CLOCK_KEY 1775
#define SHARED_MEM_STATS_KEY 72912
#define QUEUE_KEY 1535
#define MASTER_ID 369


#define DEFAULT_NUM_PROCESSES 18

#include <stdlib.h>

typedef struct SharedClock {
    unsigned int seconds;
    unsigned int nanoSeconds;
} SharedMemClock;

typedef struct UserProcess {
    int index;
    pid_t actualPid;
    int priority;
    int duration;
    int progress;
    int burstTime;
    int waitTime;
} UserProcess;

typedef struct ProcessControlBlock {
    int pidIndex;
    pid_t actualPid;
    int priority;
    int isScheduled;
    int isBlocked;
    int burstTime;
    int resumeTime;
    int duration;
    int progress;
    int waitTime;
} PCB;

typedef struct Queue {
    UserProcess* array;
    int front;
    int rear;
    int size;
    unsigned int queueCapacity;
} Queue;

typedef struct Pages {
    int occupied;
    int dirty;
    int location;
} Pages;

typedef struct PageTable {
    Pages pages[32];
} PageTable;

typedef struct Reference {
    int pageNumber;
    int offset;
} Reference;

typedef struct Message {
    long type;
    long pid;
    int index;
    int dirty;
    int terminate;
    Reference ref;
} Message;

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
}ProcessStats;

#endif //O2_THOMPSON_4_SHARED_H