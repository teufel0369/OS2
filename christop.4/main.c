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


/* GLOBAL STRUCTS */
PCB* pcb;
SharedMemClock* sharedMemClock;
UserProcess* userProcess;

/* GLOBAL MISC */
char* fileName;
char* pcbAddress;
int indexCounter;
int numChildren;
int numTerminated;
int pcbSharedMemId;
int queueSharedMemId;
int sharedMemClockId;
int timerAmount;


/* PROTOTYPES */
void displayHelp(int);
int detachAndRemove(int, void*);
long checkLogFile(FILE*);
int randomNumberGenerator(int, int);
void signalHandlerMaster(int);
void processControlBlocksSetup();
void sharedMemoryClockSetup();
void advanceSharedMemoryClock();

/*************************************************!
* @function    main
* @abstract    orchestrates the madness
* @param       argc
* @param       argv
* @returns     0... hopefully
**************************************************/
int main(int argc, char **argv) {
    int c, wait_status;
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

    /* set up the timer */
    alarm(timerAmount);

    /* create message queue */
    queueSharedMemId = msgget(QUEUE_KEY, IPC_CREAT | 0600);

    /* set up the shared memory clock */
    sharedMemoryClockSetup();

    /* set up the process control blocks */
    processControlBlocksSetup();

    /*  */

    /* wait for any remaining child processes to finish */
    while (wait(&wait_status) > 0) { ; }

    /* detach and remove the message queue, shared memory, and any allocated memory */
    free(pcb);
    detachAndRemove(sharedMemClockId, sharedMemClock);
    msgctl(queueSharedMemId, IPC_RMID, NULL);
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

//    fp = fopen(fileName, "r");
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

/*******************************************************!
* @function    processControlBlocksSetup
* @abstract    sets up the process control blocks
*******************************************************/
void processControlBlocksSetup() {
    /* Allocate memory for user processes, but will not be in shared memory. Seems easier to manage this way */
    userProcess = (struct UserProcess*) malloc(sizeof(struct UserProcess) * 18);

    /* allocate some memory for the array of processes */
    pcb = (struct ProcessControlBlock*) malloc(sizeof(struct ProcessControlBlock) * 18);

    /* create Process Control Block shared memory segment */
    if ((pcbSharedMemId = shmget(SHARED_MEM_PCB_KEY, sizeof(pcb), IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment.");
        exit(errno);
    }

    /* attach the shared memory process control blocks */
    pcbAddress = shmat(pcbSharedMemId, NULL, 0);

    //initialize pcb with slaveID = -1;
    pcb = (PCB*) ((void*)pcbAddress+sizeof(int));

    int i;
    for(i = 0; i < 18; i++) {

    }
}

/*******************************************************!
* @function    sharedMemoryClockSetup
* @abstract    sets up the shared memory clock
*******************************************************/
void sharedMemoryClockSetup() {
    /* create Shared Memory Clock shared memory segment */
    if ((sharedMemClockId = shmget(SHARED_MEM_CLOCK_KEY, sizeof(SharedMemClock), IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment.");
        exit(errno);
    }

    /* attach the shared memory clock */
    sharedMemClock = shmat(sharedMemClockId, NULL, 0);

    /* initialize the shared memory clock */
    sharedMemClock->seconds = 0;
    sharedMemClock->nanoSeconds = 0;
}

/*******************************************************!
* @function    advanceClock
* @abstract    advances the shared memory clock by 1.xx
*              where xx is random number in range
*              [0, 1000]
*******************************************************/
void advanceSharedMemoryClock() {
    unsigned int tempVar = 0;
    int nanoRandom = randomNumberGenerator(1000, 0);
    sharedMemClock->nanoSeconds += nanoRandom;
    sharedMemClock->seconds += 1;
    if(sharedMemClock->nanoSeconds >= 999999999) {
        tempVar = sharedMemClock->nanoSeconds - 999999999;
        sharedMemClock->seconds += 1;
        sharedMemClock->nanoSeconds = tempVar;
    }
}

/*******************************************************!
* @function    generateQueue
* @abstract    generates the Queue
* @param       capacity
* @returns     queue
*******************************************************/
struct Queue* generateQueue(unsigned int capacity) {
    /* allocate space for the Queue and initialize the queue */
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->queueCapacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(queue->queueCapacity * sizeof(int));
    return queue;
}