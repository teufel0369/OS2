#ifndef CHRISTOP_2_SHARED_H
#define CHRISTOP_2_SHARED_H

#include <ntsid.h>

#define SHARED_MEM_KEY 1775

#define TOTAL_PROCESSES 20



typedef struct SharedClock {
    int seconds;
    int milliseconds;
} SharedMemClock;

typedef struct ProcessContainer {
    int pidIndex;
    pid_t actualPid;
} Process;

#endif //CHRISTOP_2_SHARED_H
