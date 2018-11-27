#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include "shared.h"


/* GLOBAL STRUCTS */
PCB* pcb;
SharedMemClock* sharedMemClock;
Queue* lowPriorityQueue;
Queue* highPriorityQueue;

/* GLOBAL MISC */
char fileName[1000];
char* pcbAddress;
int indexCounter;
FILE* logfile;
int numChildren;
int numTerminated;
int pcbSharedMemId;
int queueSharedMemId;
int sharedMemClockId;
int timerAmount;

/* PROTOTYPES */
void displayHelp(int);
void signalHandlerMaster(int);
int detachAndRemove(int, void*);


int main(int argc, char **argv) {
    srand(time(NULL));
    int c, i, wait_status;
    char childId[10];
    char* lvalue = NULL;
    char* tvalue = NULL;


    while ((c = getopt (argc, argv, "h l::t::")) != -1) {
        switch (c) {
            case 'h':
                displayHelp(1);
                break;
            case 'l':
                lvalue = optarg;
                break;
            case 't':
                tvalue = optarg;
                break;
            case '?':
                if (optopt == 'l' || optopt == 't')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character '%c'.\n", optopt);
                return 1;
            default:
                abort();
        }
    }

    /* check the passed in values or assign defaults */
    timerAmount = (tvalue == NULL) ? DEFAULT_TIMER : atoi(tvalue);
    (lvalue == NULL) ? strcpy(fileName, DEFAULT_FILENAME) : strcpy(fileName, lvalue);

    /* register signal handler (SIGINT) */
    if (signal(SIGINT, signalHandlerMaster) == SIG_ERR) {
        perror("Error: Couldn't catch SIGINT\n");
        exit(errno);
    }

    /* register signal handler (SIGALRM) */
    if (signal(SIGALRM, signalHandlerMaster) == SIG_ERR) {
        perror("Error: Couldn't catch SIGALRM\n");
        exit(errno);
    }

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

/*******************************************************!
* @function    signalHandler
* @abstract    performs all necessary signal handling
*
* @param       pidIndex index of the pidIndex
* @returns     actual index from the pid array
*******************************************************/
void signalHandlerMaster(int signo) {
    int i, wait_status;

    if (signo == SIGINT || signo == SIGALRM) {
        if (signo == SIGINT)
            printf("MASTER: SIGNAL: SIGINT detected by MASTER\n");
        else
            printf("MASTER: SIGNAL: SIGALRM detected by MASTER\n");

        for (i = 0; i < numChildren; i++) {
            kill(pcb[i].actualPid, SIGINT);
        }

        while (wait(&wait_status) > 0) { ; }

        free(pcb);
        detachAndRemove(sharedMemClockId, sharedMemClock);
        detachAndRemove(pcbSharedMemId, pcb);
        msgctl(queueSharedMemId, IPC_RMID, NULL);
        exit(0);
    }
}

/*************************************************!
* @function    displayHelp
* @abstract    displays a help message if the
*              the flag is 1
* @param       flag
**************************************************/
void displayHelp(int flag) {
    if (flag == 1) {
        printf("Options\n");
        printf("\t-h          : Display this message.\n");
        printf("\t-l filename : Set file to 'filename'. Defaults to '%s'.\n", DEFAULT_FILENAME);
        printf("\t-t z        : Set time (in seconds) before master terminates. Defaults to '%d'.\n", DEFAULT_TIMER);
        exit(0);
    }
}