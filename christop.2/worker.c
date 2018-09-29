#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <signal.h>
#include "shared.h"

/*prototypes*/
void signalHandlerChild(int);
int detachAndRemove(int, void*);

/* GLOBALS */
int workerIndexNumber;
int sharedMemId;
SharedMemClock *shm;

int main(int argc, char* const argv[]) {
    long timesIncremented = 0;

    if (argc < 2) {
        fprintf(stderr, "Error: Missing index number\n");
        exit(1);
    }

    workerIndexNumber = atoi(argv[1]);

    /* register signal handler */
    if (signal(SIGINT, signalHandlerChild) == SIG_ERR) {
        perror("Error: Couldn't catch SIGTERM.\n");
        exit(errno);
    }

    /* access shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_KEY, sizeof(SharedMemClock), 0600)) < 0) {
        perror("Error: shmget");
        exit(errno);
    }

    char myString[1000];

    /* attach shared memory */
    shm = shmat(sharedMemId, NULL, 0);

    if(shm->doneFlag == 1) {
        shm->doneFlag = 0;
        snprintf(myString, sizeof myString, "\nWorker %d  My Parent is %d\nMy process number is: %d\n", getpid(), getppid(), workerIndexNumber);
        shm->message = myString;
        fprintf(stderr, "%s", shm->message);
        snprintf(myString, sizeof myString, "Worker %d beginning to increment the clock.\n", getpid());
        shm->message = myString;
        fprintf(stderr, "%s", shm->message);

        while(1) {
            if(timesIncremented == 3000000) {
                snprintf(myString, sizeof myString, "\nWorker %d has reached 3000000 increments and will be terminating.\n"
                                                    "Shared Memory Seconds: %d\n"
                                                    "Shared Memory Milliseconds: %d\n", getpid(), shm->seconds, shm->milliseconds);
                shm->message = myString;
                fprintf(stderr, "%s", shm->message);
                break;
            }

            if((shm->milliseconds >= 999) && (timesIncremented < 3000000)) {
                shm->seconds += 1;
                shm->milliseconds = 000;
                timesIncremented += 1;
            } else if((shm->milliseconds < 999) && (timesIncremented < 3000000)) {
                shm->milliseconds += 1;
                timesIncremented += 1;
            }
        }
    }

    shm->turn += 1;
    shm->doneFlag = 1;
    shmdt(shm);
    return 0;
}

/*******************************************************!
* @function    signalHandlerChild
* @abstract    signal handler for child processes;
*              really just an empty handler to exit to
*              exit successfully
* @param       signo signal number received
*******************************************************/
void signalHandlerChild(int signal) {
    exit(0);
}

/*************************************************!
* @function    detachAndRemove
* @abstract    detach and remove any shared memory
* @param       pidIndex index of the pidIndex
* @returns     actual index from the pid array
* @cite        Unix Systems Programming Pg. 528
**************************************************/
int detachAndRemove(int shmid, void *shmaddr) {
    int error = 0;

    if (shmdt(shmaddr) == -1)
        error = errno;
    if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error)
        error = errno;
    if (!error)
        return 0;
    errno = error;
    return -1;
}