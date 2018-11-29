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
SharedMemClock* sharedMemClock;
UserProcess* userProcess;
Queue* queue;

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
PageTable initializePageTable();
void sharedMemoryClockSetup();
UserProcess* initializeUserProcess(int);
int randomNumberGenerator(int, int);


int main(int argc, char **argv) {
    srand(time(NULL));
    int c, i, wait_status;
    char childId[10];
    char* nvalue = NULL;
    int numProcesses;

    while ((c = getopt (argc, argv, "h n::")) != -1) {
        switch (c) {
            case 'h':
                displayHelp(1);
                break;
            case 'n':
                nvalue = optarg;
                break;
            case '?':
                if (optopt == 'n')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character '%c'.\n", optopt);
                return 1;
            default:
                abort();
        }
    }

    /* check the passed in values or assign defaults */
    numProcesses = (nvalue == NULL) ? DEFAULT_NUM_PROCESSES : atoi(nvalue);

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

    /* initialize the page table */
    PageTable pageTable = initializePageTable();

    /* set up the shared memory clock */
    sharedMemoryClockSetup();

    /* initialize the user processes */
    userProcess = initializeUserProcess(numProcesses);

    int totalExecutedProcesses = numProcesses;
    int currentNumProcesses, randomMillis = 0;
    pid_t childPid;

    do {
        randomMillis = randomNumberGenerator(500, 1);
        if(currentNumProcesses <= 18) {
            childPid = fork();

            /* if this is the child process */
            if(childPid == 0) {

                fprintf(stderr, "\nMaster: Generating process with PID %d at time %d.%d\n", getpid(), sharedMemClock->seconds, sharedMemClock->nanoSeconds);

            } else if (childPid < 0) {
                perror("[-]ERROR: Failed to fork CHILD process.\n");
                exit(errno);
            }
        }

    } while(totalExecutedProcesses <= numProcesses);
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
            kill(userProcess[i].actualPid, SIGINT);
        }

        while (wait(&wait_status) > 0) { ; }

        free(userProcess);
        detachAndRemove(sharedMemClockId, sharedMemClock);
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
        printf("\t-n numProcesses : Set the number of processes'. Defaults to '%d'.\n", DEFAULT_NUM_PROCESSES);
        exit(0);
    }
}

/*************************************************!
* @function    initializePageTable
* @abstract    initalizes the page table
* @return      pageTable
**************************************************/
PageTable initializePageTable() {
    PageTable* pageTable = (struct PageTable*) malloc(sizeof(struct PageTable));
    return *pageTable;
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

/*************************************************!
* @function    initializeUserProcess
* @abstract    sets up the user processes
* @param       index
* @return      userProcess
**************************************************/
UserProcess* initializeUserProcess(int numProcesses) {
    /* Allocate memory for user processes, but will not be in shared memory. Seems easier to manage this way */
    UserProcess* userProcess = (struct UserProcess*) malloc(sizeof(struct UserProcess) * numProcesses);
    return userProcess;
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