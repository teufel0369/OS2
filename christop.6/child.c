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
int sharedMemStatsId;
int pcbMemId;
int queueId;
SharedMemClock* shm;
ProcessStats* processStats;

void sendMessageToMaster(int, int);
void signalHandlerChild(int);
int random5050();
Message receiveMessageFromMaster(int);

/*******************************************************!
* @function    main
* @abstract    runs the show
* @param       argc
* @param       argv
* @return      0 if it doesn't blow up
*******************************************************/
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "\n[-]ERROR: Missing process Num.\n");
        exit(1);
    }
    childId = atoi(argv[1]);

    /* register signal handler */
    if (signal(SIGINT, signalHandlerChild) == SIG_ERR) {
        perror("\n[-]ERROR: Couldn't catch SIGTERM.\n");
        exit(errno);
    }

    /* access shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_CLOCK_KEY, sizeof(SharedMemClock), 0600)) < 0) {
        perror("\n[-]ERROR: shmget failed on shared memory clock.\n");
        exit(errno);
    }

    /* access shared memory segment */
    if ((sharedMemStatsId = shmget(SHARED_MEM_STATS_KEY, sizeof(ProcessStats), 0600)) < 0) {
        perror("\n[-]ERROR: shmget failed on shared memory clock.\n");
        exit(errno);
    }

    /* access message queue*/
    queueId = msgget(QUEUE_KEY, 0600);

    /* attach shared memory */
    shm = (SharedMemClock *) shmat(sharedMemId, NULL, 0);
    printf("\nChild %d attached to shared memory clock\n", childId);

    /* attach shared memory */
    processStats = (ProcessStats *) shmat(sharedMemStatsId, NULL, 0);
    printf("\nChild %d attached to shared memory process stats service\n", childId);


    processStats->totalExecuted += 1;
    processStats->activeProcesses -= 1;
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
    Message message;
    size_t messageSize = sizeof(Message) - sizeof(long);

    msgsnd(queueId, &message, messageSize, 0);
}

/*******************************************************!
* @function    receiveMessage
* @abstract    removes a message from the message queue
* @param       messageType
*******************************************************/
Message receiveMessageFromMaster(int messageType) {
    Message message;
    int messageSize = sizeof(Message);
    msgrcv(queueId, &message, messageSize, messageType, 0);
    return message;
}