/*********************************************************************************
* Author: Chris Thompson
* Project Name: o2-thompson.3
* Project Desc: This project will demonstrate interprocess communication using
*		message queues to pass messages between processes and shared
*		memory to
* Initial Commit: 15 October 2018
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

#define DEFAULT_NUM_CHILDREN 5
#define DEFAULT_FILENAME "data.out"
#define DEFAULT_TIMER 20
#define TOTAL_PROCESSES 100

/* GLOBALS */
Process *pid;
int numChildren;
int sharedMemId;
int queueId;
SharedMemClock *shm;
int indexCounter;
int numTerminated = 0;
char filename[64];

/* PROTOTYPES */
int getIndex(int);
int detachAndRemove(int, void*);
void signalHandlerMaster(int);
void sendMessageToChild(int);
void receiveMessageFromChild(int);
void displayHelp(int);

/*************************************************!
* @function    main
* @abstract    orchestrates the madness
* @param       argc    number of arguments.
* @param       argv    array of arguments
* @returns     0 if it runs correctly.
**************************************************/
int main (int argc, char **argv) {
    int i, wait_status, timerAmt;
    char childId[3];
    char *svalue = NULL;
    char *lvalue = NULL;
    char *tvalue = NULL;
    int c;

    opterr = 0;
    while ((c = getopt (argc, argv, "hs:l:t:")) != -1) {
        switch (c) {
            case 'h':
                displayHelp(1);
                break;
            case 's':
                svalue = optarg;
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

    numChildren = (svalue == NULL) ? DEFAULT_NUM_CHILDREN : atoi(svalue);
    timerAmt = (tvalue == NULL) ? DEFAULT_TIMER : atoi(tvalue);
    (lvalue == NULL) ? strcpy(filename, DEFAULT_FILENAME) : strcpy(filename, lvalue);
    
    indexCounter = numChildren + 1;

    /* allocate some memory for the array of processes */
    pid = (Process *) malloc(sizeof(Process) * numChildren);

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
    alarm(timerAmt);

    /* create message queue */
    queueId = msgget(QUEUE_KEY, IPC_CREAT | 0600);

    /* create shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_KEY, sizeof(SharedMemClock), IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment.");
        exit(errno);
    }

    /* attach the shared memory */
    shm = shmat(sharedMemId, NULL, 0);

    /* initialize the shared memory */
    shm->seconds = 0;
    shm->nanoSeconds = 0;

    /* fork the processes */
    for (i = 0; i < numChildren; i++) {
        pid[i].pidIndex = i + 1;
        pid[i].actualPid = fork();

        if (pid[i].actualPid == 0) { /* if this is the child process */
            sprintf(childId, "%d", pid[i].pidIndex);
            execl("./child", "./child", childId, NULL); /* exec it off */
        }
        else if (pid[i].actualPid < 0) {
            perror("[-]ERROR: Failed to fork CHILD process.\n");
            exit(errno);
        }

    }

    /* MASTER section --> the child process get a execl() call and go off and do their own thing */

    while (shm->seconds < 2 && numTerminated < TOTAL_PROCESSES) {
        int i;
        for(i = 0; i < numChildren; i++) {
            if(pid[i].pidIndex != -1) {
                sendMessageToChild(pid[i].pidIndex);   /* send message to notify critical section is free */
                receiveMessageFromChild(MASTER_ID);   /* receive WAIT for child to enter */
                sendMessageToChild(pid[i].pidIndex); /* send ok to run */
                receiveMessageFromChild(MASTER_ID); /* receive WAIT for child to unlock or terminate */
            }
        }
    }

    /* send signal to kill any remaining children */
    for (i = 0; i < numChildren; i++) {
        kill(pid[i].actualPid, SIGINT);
    }

    /* wait for any remaining child processes to finish */
    while (wait(&wait_status) > 0) { ; }

    if(shm->seconds >= 2)
        printf("MASTER terminating because shared clock exceeded 2 seconds!\n");
    if (numTerminated >= TOTAL_PROCESSES)
        printf("MASTER terminating. %d processes executed\n", TOTAL_PROCESSES);

    /* detach and remove the message queue, shared memory, and any allocated memory */
    free(pid);
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
            kill(pid[i].actualPid, SIGINT);
        }

        while (wait(&wait_status) > 0) { ; }

        free(pid);
        detachAndRemove(sharedMemId, shm);
        msgctl(queueId, IPC_RMID, NULL);
        exit(0);
    }
}

/*************************************************!
* @function    sendMessageToChild
* @abstract    inserts a message into the message
*              queue for the child
* @param       messageType
**************************************************/
void sendMessageToChild(int messageType) {
    Envelope message;
    static int messageSize = sizeof(Envelope) - sizeof(long);
    message.messageType = messageType;
    msgsnd(queueId, &message, messageSize, 0);
}

/*************************************************!
 * @function    receiveMessageFromChild
 * @abstract    get the index from the array of
 *              children... it'll be useful
 * @param       pidIndex index of the pidIndex
 * @returns     actual index from the pid array
 **************************************************/
void receiveMessageFromChild(int messageType) {
    FILE *fp;
    Envelope message;
    static int messageSize = sizeof(Envelope) - sizeof(long);
    msgrcv(queueId, &message, messageSize, messageType, 0);

    srand(time(NULL));
    int runtime = (rand() % 10001)+ 1;

    shm->nanoSeconds += runtime;
    if(shm->nanoSeconds > 1000000000){
        ++(shm->seconds);
        shm->nanoSeconds -= 1000000000;
    }

    if (message.doneFlag) {
        int index = getIndex(message.childId);
        char childId[3];

        kill(pid[index].actualPid, SIGINT);
        int status;

        waitpid(pid[index].actualPid, &status, 0);
        numTerminated++;
        
        fp = fopen(filename, "a");
        if (indexCounter <= TOTAL_PROCESSES) {
            pid[index].pidIndex = indexCounter++;
            fprintf(fp, "\nMASTER: creating new CHILD %d at my time %d.%d", pid[index].pidIndex, shm->seconds, shm->nanoSeconds);
            pid[index].actualPid = fork();

            if (pid[index].actualPid == 0) {
                sprintf(childId, "%d", pid[index].pidIndex);
                execl("./child", "./child", childId, NULL);
            }
            else if (pid[index].actualPid < 0) {
                perror("[-]ERROR: Failed to fork CHILD process.\n");
                exit(errno);
            }
        }
        else {
            pid[index].pidIndex = -1;
        }

        fprintf(fp, "\nMASTER: CHILD %d is terminating at my time %d.%d because it reached %d.%d",
                message.childId, shm->seconds, shm->nanoSeconds, message.seconds, message.nanoSeconds);
        fclose(fp);
    }
}

/*************************************************!
* @function    getIndex
* @abstract    get the index from the array of
*              children.
* @param       pidIndex index of the pidIndex
* @returns     actual index from the pid array
**************************************************/
int getIndex(int logical) {
    int i;
    for(i = 0; i < numChildren; i++)
        if(pid[i].pidIndex == logical)
            return i;
    return -1;
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
        printf("\t-s x        : Set number of children to 'x'. Defaults to '%d'.\n", DEFAULT_NUM_CHILDREN);
        printf("\t-l filename : Set file to 'filename'. Defaults to '%s'.\n", DEFAULT_FILENAME);
        printf("\t-t z        : Set time (in seconds) before master terminates. Defaults to '%d'.\n", DEFAULT_TIMER);
        exit(0);
    }
}
