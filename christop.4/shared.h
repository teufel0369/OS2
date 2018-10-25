#pragma once
#ifndef O2_THOMPSON_3_SHARED_H
#define O2_THOMPSON_3_SHARED_H
#define SHARED_MEM_KEY 1775
#define QUEUE_KEY 4604
#define MASTER_ID 369

#include <stdlib.h>

typedef struct SharedClock {
    int seconds;
    int nanoSeconds;
} SharedMemClock;

typedef struct Message {
    long messageType;
    int childId;
    int doneFlag;
    int seconds;
    int nanoSeconds;
} Envelope;

typedef struct ProcessControlBlock {
    int pidIndex;
    pid_t actualPid;
    int seconds;
    int nanoSeconds;
} PCB;

#endif //O2_THOMPSON_4_SHARED_H