/*********************************************************************************
* Author: Chris Thompson
* Project Name: o2-thompson.3
* Project Desc: This project will demonstrate interprocess communication using
*		message queues to pass messages between processes and shared
*		memory to
* Initial Commit: 22 October 2018
* Github Repo: https://github.com/teufel0369/OS2.git
*********************************************************************************/

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

#define DEFAULT_TIMER 20
#define DEFAULT_FILENAME "log.out"
#define TOTAL_PROCESSES 100

/* GLOBALS */
PCB pcb[18];
int numChildren;
int sharedMemId;
int queueId;
SharedMemClock *shm;
int indexCounter;
int numTerminated = 0;
char* fileName;
int timerAmount;

/* PROTOTYPES */
void displayHelp(int);
int detachAndRemove(int, void*);
long checkLogFile(FILE*);
int randomNumberGenerator(int, int);

int main(int argc, char **argv) {
    int c;
    char* lvalue = NULL;
    char* tvalue = NULL;

    while ((c = getopt (argc, argv, "h l:t:")) != -1) {
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
                if (optopt == 's' || optopt == 'l' || optopt == 't')
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

    /* create message queue */
    queueId = msgget(QUEUE_KEY, IPC_CREAT | 0600);

    /* create shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_KEY, sizeof(SharedMemClock), IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment.");
        exit(errno);
    }

    /* create shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_KEY, sizeof(pcb), IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment.");
        exit(errno);
    }

    /* detach and remove the message queue, shared memory, and any allocated memory */
    free(pcb);
    detachAndRemove(sharedMemId, shm);
    msgctl(queueId, IPC_RMID, NULL);
    return 0;
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

/*************************************************!
* @function    checkLogFile
* @abstract    checks the log file for the length
* @param       fp
* @returns     number of lines in log file
**************************************************/
long checkLogFile(FILE* fp) {
    long numLines = 0;
    char c;

    fp = fopen(fileName, "r");
    for (c = getc(fp); c != EOF; c = getc(fp))
        if (c == '\n')
            numLines += 1;

    fclose(fp);
    return numLines;
}

/*************************************************!
* @function    randomNumberGenerator
* @abstract    generates a random number between
*              MAX and MIN
* @param       MAX
* @param       MIN
* @returns     random number
**************************************************/
int randomNumberGenerator(int MAX, int MIN) {
    return rand()%(MAX - MIN) + MIN;
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