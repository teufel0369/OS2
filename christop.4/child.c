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
SharedMemClock* shm;
PCB* pcb;

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

    /* access message queue*/
    queueId = msgget(QUEUE_KEY, 0600);

    /* access shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_CLOCK_KEY, sizeof(SharedMemClock), 0600)) < 0) {
        perror("\n[-]ERROR: shmget failed on shared memory clock.\n");
        exit(errno);
    }

    /* attach shared memory */
    shm = (SharedMemClock*) shmat(sharedMemId, NULL, 0);
    printf("\nChild %d attached to shared memory clock\n", childId);

    /* access shared memory segment */
    if ((pcbMemId = shmget(SHARED_MEM_PCB_KEY, sizeof(PCB), 0600)) < 0) {
        perror("\n[-]ERROR: shmget failed on shared memory process control block in child\n");
        exit(errno);
    }

    /* attach shared memory */
    pcb = (PCB*) shmat(pcbMemId, NULL, 0);
    printf("\nChild %d attached to shared memory process control block\n", childId);


    while(1) {
        if(pcb[childId].isScheduled == 1) {
            //TODO: need to add message queueing on scheduled and done
            printf("\nChild %d was scheduled\n", childId);
            printf("\nChild %d has PID of %d and PPID of %d\n", childId, getpid(), getppid());
            printf("\nChild %d shared memory clock current time: %d.%d\n", childId, shm->seconds, shm->nanoSeconds);
            sendMessageToMaster(MASTER_ID, 1);

            break;
        }
    }

    return 0;
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
* @function    random5050
* @abstract    generates even or odd with equal
*              probability
* @return      returns 1 if odd, 0 if even
*******************************************************/
int random5050() {
    return rand() & 1;
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
    message.messageType = messageType;
    message.childId = childId;
    message.doneFlag = isdone;
    message.seconds = shm->seconds;
    message.nanoSeconds = shm->nanoSeconds;
    msgsnd(queueId, &message, messageSize, 0);
}