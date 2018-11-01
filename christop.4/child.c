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
int pcbMemId;
int queueId;
SharedMemClock *shm;
PCB* pcb;

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
    if ((sharedMemId = shmget(SHARED_MEM_CLOCK_KEY, sizeof(SharedMemClock), 0600)) < 0) {
        perror("Error: shmget");
        exit(errno);
    }

    /* attach shared memory */
    shm = shmat(sharedMemId, NULL, 0);

    /* setup shared memory for pcb */
    pcbMemId = shmget(PCB_KEY, 2048, 0666);
    if(pcbMemId < 0) {
        perror("[-]ERROR: Creating PCB shared memory Failed!!\n");
        exit(1);
    }
    pcb = (PCB* )shmat(pcbMemId, NULL, 0);

    receiveMessageFromMaster(childId);
    return 0;
}


/*******************************************************!
* @function    receiveMessage
* @abstract    removes a message from the message queue
* @param       messageType
*******************************************************/
void receiveMessageFromMaster(int messageType) {
    Message message;
    static int messageSize = sizeof(Message) - sizeof(long);
    msgrcv(queueId, &message, messageSize, messageType, 0);
    printf("%s", message.message);
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