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
int readWriteRatio;
int sharedMemId;
int sharedMemStatsId;
int pcbMemId;
int queueId;
int readWriteConfig;
SharedMemClock* shm;
ProcessStats* processStats;
Message messageFromMaster;
Message requestMemoryMessage;

void sendMessageToMaster(int, Message);
void signalHandlerChild(int);
int random5050();
void receiveMessageFromMaster(int);
int randomReadOrWrite();
int randomNumberGenerator(int, int);
void requestMemory();

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
    readWriteRatio = atoi(argv[2]);

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
    queueId = msgget(QUEUE_KEY, 0666);

    /* attach shared memory */
    shm = (SharedMemClock *) shmat(sharedMemId, NULL, 0);
    printf("\nChild %d attached to shared memory clock\n", childId);

    /* attach shared memory */
    processStats = (ProcessStats *) shmat(sharedMemStatsId, NULL, 0);
    printf("\nChild %d attached to shared memory process stats service\n", childId);
    int numRef = 0;

    while(1) {
        printf("\nChild %d has not received a message yet.\n", childId);
        receiveMessageFromMaster(childId);
        if(messageFromMaster.messageTestMaster == 1){
            printf("\nChild %d received a message from master\n", childId);
            break;
        }
    }

//       if((numRef % 1000) == 0) {
//           if(messageFromMaster.terminate == 1) {
//               printf("\nChild %d received terminate signal from Master\n", childId);
//               break;
//           }
//       } else {
//            requestMemory();
//            printf("\nChild %d requesting memory at page number %d and offset %d.\n", childId, requestMemoryMessage.ref.pageNumber, requestMemoryMessage.ref.offset);
//            numRef += 1;
//       }
//    }

    processStats->totalExecuted += 1;
    processStats->activeProcesses -= 1;
    printf("\nChild %d terminating\n", childId);
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
void sendMessageToMaster(int messageType, Message message) {
    size_t messageSize = sizeof(Message) - sizeof(long);
    message.index = childId;
    msgsnd(queueId, &message, messageSize, 0);
}

/*******************************************************!
* @function    receiveMessage
* @abstract    removes a message from the message queue
* @param       messageType
*******************************************************/
void receiveMessageFromMaster(int messageType) {
    static int messageSize = sizeof(Message);
    ssize_t check;
    printf("\nChild %d waiting on message\n", childId);
    if(msgrcv(queueId, &messageFromMaster, (size_t) messageSize, messageType, IPC_NOWAIT) == -1) {
        printf("\nUser: Failed to get Message!\n");
    } else {
        printf("\nUser: Received message from Master\n");
    }
}

/*******************************************************!
* @function    requestMemory
* @abstract    requests memory from
* @param       messageType
*******************************************************/
void requestMemory() {
    requestMemoryMessage.type = MASTER_ID;
    requestMemoryMessage.index = childId;
    requestMemoryMessage.childProcessTerminating = 0;
    requestMemoryMessage.ref.pageNumber = childId;
    requestMemoryMessage.ref.offset = rand() % 32;
    requestMemoryMessage.pid = getpid();
    requestMemoryMessage.dirty = randomReadOrWrite();

    printf("User: Process %d is requesting Page# %d with offset: %d from OSS!\n", requestMemoryMessage.pid, requestMemoryMessage.ref.pageNumber, requestMemoryMessage.ref.offset);

    msgsnd(queueId, &requestMemoryMessage, sizeof(Message), 1);
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
* @function    randomReadOrWrite
* @returns     random number
**************************************************/
int randomReadOrWrite() {
    if (randomNumberGenerator(1, 100) > readWriteRatio) {
        return 1;
    } else {
        return 0;
    }
}