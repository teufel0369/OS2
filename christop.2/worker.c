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

int workerIndexNumber;
int sharedMemId;
SharedMemClock *shm;

int main(int argc, char* const argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Error: Missing process number\n");
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

    /* attach shared memory */
    shm = shmat(sharedMemId, NULL, 0);

    whoAmI(workerIndexNumber);
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