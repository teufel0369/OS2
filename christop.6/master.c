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
ProcessStats* processStats;
Queue* queue;
Queue* suspendedQ;
Frames frames[256];
Process* processQueue;

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
int processStatsId;
int timerAmount;
int readWriteConfig;
int readWriteConfig;

/* PROTOTYPES */
void displayHelp(int);
void signalHandlerMaster(int);
int detachAndRemove(int, void*);
PageTable initializePageTable();
void sharedMemoryClockSetup();
int randomNumberGenerator(int, int);
unsigned int getMillis();
void processStatsSetup();
int isQueueEmpty(Queue*);
int isQueueFull(Queue*);
void pushToQueue(struct Queue*, Message);
Message popFromQueue(Queue*);
void suspendedCheck(Queue*);
static void printFrames();
struct Queue* generateQueue(unsigned int);
void rollClock(int);


int main(int argc, char **argv) {
    srand(time(NULL));
    int c, i, wait_status;
    char childId[10];
    char readWriteRatio[10];
    char* nvalue, pvalue = NULL;

    while ((c = getopt (argc, argv, "h n::p::")) != -1) {
        switch (c) {
            case 'h':
                displayHelp(1);
                break;
            case 'n':
                nvalue = optarg;
                break;
            case 'p':
                pvalue = optarg;
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
    numChildren = (nvalue == NULL) ? DEFAULT_NUM_PROCESSES : atoi(nvalue);
    readWriteConfig = (pvalue == NULL || atoi(pvalue) > 99) ? 50 : atoi(pvalue);

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

    /* process stats setup */
    processStatsSetup();

    /* set up the shared memory clock */
    sharedMemoryClockSetup();

    /* set up the suspend queue */
    suspendedQ = generateQueue(18);

    processQueue = (struct Process*) malloc(sizeof(struct Process) * 18);

    int randomMillis = 0;
    pid_t childPid;
    int processIndex = 0;
    unsigned int spawnTime = 0;
    unsigned int currentTime = 0;
    int check = 0;
    int index = 0;

    while(1) {

        if(processStats->totalExecuted >= numChildren) {
            break;
        }

        check++; //suspend check goes here
        if(check % 30 == 0) {
            ;
        }

        spawnTime = randomNumberGenerator(500, 1);
        currentTime = getMillis();

        fprintf(stderr, "\nMaster: current time %u\n", currentTime);
        fprintf(stderr, "\nMaster: spawn time %u\n", spawnTime);

        if(processStats->activeProcesses < 18) {
            childPid = fork();

            /* if this is the child process */
            if(childPid == 0) {

                processStats->activeProcesses += 1;
                fprintf(stderr, "\nMaster: Generating process with PID %d and PPID %d at time %d.%d\n", getpid(), getppid(), sharedMemClock->seconds, sharedMemClock->nanoSeconds);
                snprintf(childId, 10,"%d", processIndex);
                snprintf(readWriteRatio, 10,"%d", processIndex);
                execl("./child", "./child", childId, readWriteRatio, NULL);

            } else if (childPid < 0) {
                perror("[-]ERROR: Failed to fork CHILD process.\n");
                exit(errno);
            }
        }

        sharedMemClock->nanoSeconds += 1000;
    }

    /* wait for any remaining child processes to finish */
    while (wait(&wait_status) > 0) { ; }

    /* detach and remove the message queue, shared memory, and any allocated memory */
    detachAndRemove(sharedMemClockId, sharedMemClock);
    detachAndRemove(processStatsId, processStats);
    return 0;
}

/*************************************************!
* @function    rollClock()
* @abstract    rolls the shared memory clock
* @param       seconds
* @param       nanos
**************************************************/
void rollClock(int nanos) {
    if(sharedMemClock->nanoSeconds >= 900000000) {
        sharedMemClock->seconds = sharedMemClock->seconds + 1;
        sharedMemClock->nanoSeconds = 0;
    } else {
        sharedMemClock->nanoSeconds = sharedMemClock->nanoSeconds + nanos;
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
            kill(processQueue[i].actualPid, SIGINT);
        }

        while (wait(&wait_status) > 0) { ; }

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

/*******************************************************!
* @function    processStatsSetup
* @abstract    sets up the shared process records
*******************************************************/
void processStatsSetup() {
    /* create Shared Memory Clock shared memory segment */
    if ((processStatsId = shmget(SHARED_MEM_STATS_KEY, sizeof(ProcessStats), IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment.");
        exit(errno);
    }

    /* attach the shared memory process record */
    processStats = shmat(processStatsId, NULL, 0);

    /* initialize the shared memory process record */
    processStats->activeProcesses = 0;
    processStats->totalExecuted = 0;
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
* @function    randomNumberGenerator
* @abstract    converts the shared memory clock to
*              milliseconds
* @returns     milliseconds of shared memory clock
**************************************************/
unsigned int getMillis() {
    return (sharedMemClock->seconds * 1000) + (sharedMemClock->nanoSeconds/1000000);
}

/*******************************************************!
* @function    isQueueEmpty
* @abstract    adds the item to the queue if there is
*              space available
* @param       queue
* @param       item
* @param       priority
*******************************************************/
void pushToQueue(struct Queue* queue, Message item) {
    if(isQueueFull(queue)) {
        return;
    }
    else {
        queue->rear = (queue->rear+1)%queue->queueCapacity;
        queue->array[queue->rear] = item;
        queue->size += 1;
    }
}

/*******************************************************!
* @function    isQueueEmpty
* @abstract    pops the item off of the queue
* @param       queue
* @return      item
*******************************************************/
Message popFromQueue(Queue* queue) {
    Message* item = NULL;
    if(isQueueEmpty(queue)) {
        return *item;
    }
    else {
        *item = queue->array[queue->rear];
        queue->rear = (queue->rear - 1)%queue->queueCapacity;
        queue->size = queue->size - 1;
        return *item;
    }
}

/*******************************************************!
* @function    isQueueFull
* @abstract    checks to see if the queue is full
* @param       queue
* @returns     true or false
*******************************************************/
int isQueueFull(Queue* queue) {
    return (queue->size == queue->queueCapacity);
}

/*******************************************************!
* @function    isQueueEmpty
* @abstract    checks to see if the queue is empty
* @param       queue
* @returns     true or false
*******************************************************/
int isQueueEmpty(Queue* queue) {
    return(queue->size == 0);
}

void suspendedCheck(Queue* queue) {
    int isDirty = 0;
    Message message = popFromQueue(queue);

    //set request time to 15ms
    int reqtime = 15000000;

//    if(.dirty == 1){
//        isDirty = 1;
//    }
}

void loadFrame(int processIndex, int pageNumber) {
    int i;
    for(i = 0; i < 256; i++) {

    }
}

static void printFrames() {
    int p;
    fprintf(logfile, "Current memory layout at time %d:%d is:\n", sharedMemClock->seconds, sharedMemClock->nanoSeconds); fflush(logfile);

    for(p = 0; p < 256; p++) {
        if(frames[p].dirty == 0) {
            fprintf(logfile, "U"); fflush(logfile);

        } else if(frames[p].dirty == 1) {
            fprintf(logfile, "D"); fflush(logfile);

        }
        else {
            fprintf(logfile, "."); fflush(logfile);
        }
    }

    fprintf(logfile, "\n"); fflush(logfile);

    for(p = 0; p < 256; p++) {
        if(frames[p].used == 0) {
            fprintf(logfile, "0"); fflush(logfile);

        } else if(frames[p].used == 1) {
            fprintf(logfile, "1"); fflush(logfile);

        } else {
            fprintf(logfile, "."); fflush(logfile);
        }
    }

    fprintf(logfile, "\n"); fflush(logfile);

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
    queue->array = (Message*)malloc(queue->queueCapacity * sizeof(Message));
    return queue;
}