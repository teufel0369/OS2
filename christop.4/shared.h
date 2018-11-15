#pragma once
#ifndef O2_THOMPSON_3_SHARED_H
#define O2_THOMPSON_3_SHARED_H
#define SHARED_MEM_CLOCK_KEY 1775
#define SHARED_MEM_PCB_KEY 72912
#define QUEUE_KEY 1535
#define MASTER_ID 369


#define DEFAULT_TIMER 20
#define DEFAULT_FILENAME "log.out"
#define QUANTUM_FULL 1000000
#define QUANTUM_HALF 500000

#include <stdlib.h>

typedef struct SharedClock {
    unsigned int seconds;
    unsigned int nanoSeconds;
} SharedMemClock;

typedef struct Message {
    long messageType;
    int childId;
    int doneFlag;
    int index;
    int resumeTime;
    int burstTime;
    int completeFlag;
    int blockFlag;
    int moveFlag;
    int terminateFlag;
    int priority;
    int duration;
    int progress;
    int seconds;
    int nanoSeconds;
    char* message;
} Message;

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

typedef struct UserProcess {
    int index;
    pid_t actualPid;
    int priority;
    int duration;
    int progress;
    int burstTime;
    int waitTime;
} UserProcess;

typedef struct Queue {
    PCB* array;
    int front;
    int rear;
    int size;
    unsigned int queueCapacity;
} Queue;

#endif //O2_THOMPSON_4_SHARED_H