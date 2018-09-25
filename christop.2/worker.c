#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <signal.h>
#include "shared.h"

/*prototypes*/
void signalHandlerChild(int);
int whoAmI(int);
int detachAndRemove(int, void*);

/* GLOBALS */
int workerIndexNumber;
int sharedMemId;
SharedMemClock *shm;

int main(int argc, char* const argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Error: Missing process number\n");
        exit(1);

    } else {
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
            snprintf(myString, sizeof myString, "Child %d  My Parent is %d\nMy process number is: %d\n\n", getpid(), getppid(), workerIndexNumber);
            shm->message = myString;
            fprintf(stderr, "%s", shm->message);
            shm->message = "";
        }

        shm->doneFlag = 1;
        shmdt(shm);
    }

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
 * @function    whoAmI
 * @abstract    used for testing
 * @param       stat_loc    return status
 **************************************************/
int whoAmI(int processNum){
    printf("Child %d  My Parent is %d\nMy process number is: %d\n\n", getpid(), getppid(), processNum);
    return 1;
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