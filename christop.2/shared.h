#pragma once

#ifndef CHRISTOP_2_SHARED_H
#define CHRISTOP_2_SHARED_H
#define SHARED_MEM_KEY 1775

#include <stdlib.h>

typedef struct SharedClock {
    int seconds;
    int milliseconds;
    int doneFlag;
    int turn;
    char* message;
} SharedMemClock;

typedef struct ProcessContainer {
    int pidIndex;
    pid_t actualPid;
} Process;

#endif //CHRISTOP_2_SHARED_H
