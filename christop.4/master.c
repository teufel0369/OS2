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
Queue* lowPriorityQueue;
Queue* highPriorityQueue;

/* GLOBAL MISC */
char fileName[1000];
char* pcbAddress;
int indexCounter;
int numChildren;
int numTerminated;
int pcbSharedMemId;
int queueSharedMemId;
int sharedMemClockId;
int timerAmount;

/* PROTOTYPES */
void advanceSharedMemoryClock();
int calculateTimeDifference(int);
long checkLogFile(FILE*);
void displayHelp(int);
int detachAndRemove(int, void*);
int determineQueuePlacement();
void forkFirstProcess(pid_t, char*);
Queue* generateQueue(unsigned int);
int isQueueEmpty(Queue* queue);
int isQueueFull(Queue*);
int popFromQueue(Queue*, char*);
void processControlBlocksSetup();
void pushToQueue(Queue*, int, char*);
int random5050();
int randomNumberGenerator(int, int);
Message receiveMessageFromChild(int);
void sendMessageToChild(int);
void sharedMemoryClockSetup();
void signalHandlerMaster(int);
void userProcessesSetup();

/*************************************************!
* @function    main
* @abstract    orchestrates the madness
* @param       argc
* @param       argv
* @returns     0... hopefully
**************************************************/
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

    /* set up the timer */
    alarm(timerAmount);

    /* create message queue */
    queueSharedMemId = msgget(QUEUE_KEY, IPC_CREAT | 0600);

    /* set up the shared memory clock */
    sharedMemoryClockSetup();

    /* set up the shared memory process control blocks */
    processControlBlocksSetup();

    /* set up the user processes */
    userProcessesSetup();

    /* set up the high and low priority queues */
    lowPriorityQueue = generateQueue(18);
    highPriorityQueue = generateQueue(18);

    pid_t childPid = 0;
    int processCount = 0;
    unsigned int previousClockTime = 0;
    int spawnTime = 0;
    int clockDifference;

    /* fork the first process in order to do the time comparisons */
    forkFirstProcess(childPid, childId); //TODO: this is killing the rest of the program. Need to fix it.

    while(processCount < 18) {
        for(i = 0; i < 18; i++) {
            spawnTime = randomNumberGenerator(2, 0);

            if(calculateTimeDifference(previousClockTime) > spawnTime) {
                childPid = fork();

                /* if this is the child process */
                if(childPid == 0) {
                    printf("%d", getpid());

                    pcb[i].isScheduled = 1;
                    pcb[i].pidIndex = i;
                    pcb[i].actualPid = childPid;

                    snprintf(childId, 10,"%d", i);

                    /* exec it off */
                    execl("./child", "./child", childId, NULL);

                } else if (childPid < 0) {
                    perror("[-]ERROR: Failed to fork CHILD process.\n");
                    exit(errno);

                }

                /* increment process count */
                processCount++;

                previousClockTime = sharedMemClock->seconds;

                /* advance the shared memory clock */
                advanceSharedMemoryClock();
            }
        }

    }

    /* wait for any remaining child processes to finish */
    while (wait(&wait_status) > 0) { ; }

    /* detach and remove the message queue, shared memory, and any allocated memory */
    detachAndRemove(sharedMemClockId, sharedMemClock);
    detachAndRemove(pcbSharedMemId, pcb);
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
    /* create Process Control Block shared memory segment */
    size_t size = sizeof(struct ProcessControlBlock) * 18;
    if ((pcbSharedMemId = shmget(SHARED_MEM_PCB_KEY, size, IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment in master.");
        exit(errno);
    }

    /* attach the shared memory process control blocks */
    pcb = shmat(pcbSharedMemId, NULL, 0);
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
* @function    userProcessSetup
* @abstract    sets up the user processes
**************************************************/
void userProcessesSetup() {
    /* Allocate memory for user processes, but will not be in shared memory. Seems easier to manage this way */
    userProcess = (struct UserProcess*) malloc(sizeof(struct UserProcess) * 18);

    int i;
    for(i = 0; i < 18; i++) {
        userProcess[i].actualPid = -1;
        userProcess[i].index = i;
    }
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

    //TODO: the below code may create an issue
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

/*******************************************************!
* @function    isQueueEmpty
* @abstract    adds the item to the queue if there is
*              space available
* @param       queue
* @param       item
* @param       priority
*******************************************************/
void pushToQueue(struct Queue* queue, int item, char* priority) {
    if(isQueueFull(queue)) {
        return;
    }
    else {
        queue->rear = (queue->rear+1)%queue->queueCapacity;
        queue->array[queue->rear] = item;
        queue->size += 1;
        printf("\n%d was added to the %s priority queue\n", item, priority);
    }
}

/*******************************************************!
* @function    isQueueEmpty
* @abstract    pops the item off of the queue
* @param       queue
* @return      item
*******************************************************/
int popFromQueue(Queue* queue, char* priority) {
    if(isQueueEmpty(queue)) {
        return 0;
    }
    else {
        int item = queue->array[queue->front];
        queue->front = (queue->front + 1)%queue->queueCapacity;
        queue->size = queue->size - 1;
        printf("\n%d was popped from the %s priority queue\n", item, priority);
        return item;
    }
}

/*******************************************************!
* @function    determineQueuePlacement
* @abstract    determines the probability of queue
*              placement
* @param       queue
* @return      1 or 0
*******************************************************/
int determineQueuePlacement() {
    return random5050() | random5050();
}

/*******************************************************!
* @function    random5050
* @abstract    generates even or odd with equal
*              probability
* @return      returns 1 if odd, 0 if even
*******************************************************/
int random5050() {
    return rand() & 1;
}

/*************************************************!
* @function    sendMessageToChild
* @abstract    inserts a message into the message
*              queue for the child
* @param       messageType
**************************************************/
void sendMessageToChild(int messageType) {
    Message message;
    message.message = "test test test from the parent";
    static int messageSize = sizeof(Message);
    message.messageType = messageType;
    msgsnd(queueSharedMemId, &message, messageSize, 0);
}


/*************************************************!
 * @function    receiveMessageFromChild
 * @abstract    get the index from the array of
 *              children... it'll be useful
 * @param       pidIndex index of the pidIndex
 * @returns     actual index from the pid array
 **************************************************/
Message receiveMessageFromChild(int messageType) {
    Message message;
    static int messageSize = sizeof(Message) - sizeof(long);
    msgrcv(queueSharedMemId, &message, messageSize, messageType, 0);
    return message;
}

/*************************************************!
 * @function    forkFirstProcess
 * @abstract    forks the first process
 * @param       childPid
 * @param       childId
 * @param       previousClockTime
 * @param       processCount
 **************************************************/
void forkFirstProcess(pid_t childPid, char* childId) {
    /* if this is the child process */
    if(childPid > 0) {
        printf("%d", getpid());

        pcb[0].isScheduled = 1;
        pcb[0].pidIndex = 0;
        pcb[0].actualPid = childPid;

    } else if (childPid < 0) {
        perror("[-]ERROR: Failed to fork CHILD process.\n");
        exit(errno);

    } else {
        snprintf(childId, 10,"%d", 0);

        /* exec it off */
        execl("./child", "./child", childId, NULL);

        /* advance the shared memory clock */
        advanceSharedMemoryClock();
        advanceSharedMemoryClock();
    }
}

/*************************************************!
 * @function    calculateTimeDifference
 * @abstract    calculates the time difference
 * @return      timeDifference
 **************************************************/
int calculateTimeDifference(int previousTime) {
    return sharedMemClock->seconds - previousTime;
}