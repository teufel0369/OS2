#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<time.h>
#include "shared.h"

int childId;
int sharedMemId;
int queueId;
SharedMemClock *shm;

void signalHandlerChild(int);
void sendMessageToMaster(int, int);
void receiveMessageFromMaster(int);

/*******************************************************!
* @function    main
* @abstract    runs the show
* @param       argc
* @param       argv
* @return      0 if it doesn't blow up
*******************************************************/
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: Missing process Num.\n");
        exit(1);
    }
    childId = atoi(argv[1]);

    /* register signal handler */
    if (signal(SIGINT, signalHandlerChild) == SIG_ERR) {
        perror("Error: Couldn't catch SIGTERM.\n");
        exit(errno);
    }

    /* access message queue*/
    queueId = msgget(QUEUE_KEY, 0600);

    /* access shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_KEY, sizeof(SharedMemClock), 0600)) < 0) {
        perror("Error: shmget");
        exit(errno);
    }

    /* attach shared memory */
    shm = shmat(sharedMemId, NULL, 0);

    /* loop here at some point */
    srand(time(NULL));
    int runtime = (rand() % 100000)+ 1;
    int elapsedTime = 0;
    while(1) {
        receiveMessageFromMaster(childId); /* receive permission from MASTER to enter critical section */
        int startTime = shm->seconds * 1000000000 + shm->nanoSeconds;
        sendMessageToMaster(MASTER_ID, 0);
        receiveMessageFromMaster(childId);
        int stopTime = shm->seconds * 1000000000 + shm->nanoSeconds;
        elapsedTime += stopTime - startTime;
        int doneFlag = (elapsedTime > runtime)? 1: 0;
        printf("Child %d Seconds: %d\tNano: %d\n", childId, shm->seconds, shm->nanoSeconds);
        sendMessageToMaster(MASTER_ID, doneFlag);
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

/*******************************************************!
* @function    sendMessageToMaster
* @abstract    inserts a message into the message queue
* @param       messageType
* @param       isDone
*******************************************************/
void sendMessageToMaster(int messageType, int isdone) {
    Envelope message;
    static int messageSize = sizeof(Envelope) - sizeof(long);
    message.messageType = messageType;
    message.childId = childId;
    message.doneFlag = isdone;
    message.seconds = shm->seconds;
    message.nanoSeconds = shm->nanoSeconds;
    msgsnd(queueId, &message, messageSize, 0);
}

/*******************************************************!
* @function    receiveMessage
* @abstract    removes a message from the message queue
* @param       messageType
*******************************************************/
void receiveMessageFromMaster(int messageType) {
    Envelope message;
    static int messageSize = sizeof(Envelope) - sizeof(long);
    msgrcv(queueId, &message, messageSize, messageType, 0);
}
