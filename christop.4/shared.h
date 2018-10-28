#pragma once
#ifndef O2_THOMPSON_3_SHARED_H
#define O2_THOMPSON_3_SHARED_H
#define SHARED_MEM_CLOCK_KEY 1775
#define SHARED_MEM_PCB_KEY 8541
#define QUEUE_KEY 4604
#define MASTER_ID 369

#define DEFAULT_TIMER 20
#define DEFAULT_FILENAME "log.out"
#define TIME_QUANTUM

#include <stdlib.h>

typedef struct SharedClock {
    unsigned int seconds;
    unsigned int nanoSeconds;
} SharedMemClock;

typedef struct Message {
    long messageType;
    int childId;
    int doneFlag;
    int seconds;
    int nanoSeconds;
} Message;

typedef struct ProcessControlBlock {
    int pidIndex;
    pid_t actualPid;
} PCB;

typedef struct UserProcess {
    int index;
    pid_t actualPid;
    int priority;
    int duration;
    int progress;
    int burstTime;

} UserProcess;

typedef struct Queue {
    int* array;
    int front;
    int rear;
    int size;
    unsigned int queueCapacity;
} Queue;

#endif //O2_THOMPSON_4_SHARED_H