#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include <signal.h>
#include "shared.h"

/*prototypes*/
void printHelpMessage(char*);
void signalHandlerMaster(int);
int detachAndRemove(int, void*);
pid_t r_wait(int*);
int whoAmI();


/* GLOBALS */
Process *pid;
SharedMemClock *shm;
int numChildren;
int sharedMemId;
int queueId;
int indexCounter;
int numTerminated = 0;

int main(int argc, char* const argv[]) {
    int opt = 0;
    int i, numChildren, maxChildren = 0;
    int wait_status;
    char myString [1000];
    char workerId[100];
    opterr = 0;

    /*get the options from the argv array*/
    while((opt = getopt(argc, argv, ":n:s::h")) != -1) {
        switch (opt) {
            case 'n':
                numChildren = atoi(optarg);
                break;

            case 's':
                maxChildren = atoi(optarg);
                break;

            case 'h':
                printHelpMessage(argv[0]);
                break;


            default:


                printHelpMessage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

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

    /* create shared memory segment */
    if ((sharedMemId = shmget(SHARED_MEM_KEY, sizeof(SharedMemClock), IPC_CREAT | 0600)) < 0) {
        perror("[-]ERROR: Failed to create shared memory segment.");
        exit(errno);
    }

    /* attach the shared memory */
    shm = shmat(sharedMemId, NULL, 0);

    /* initialize the shared memory */
    shm->seconds = 0;
    shm->milliseconds = 0;

    /* fork the processes */
    for (i = 0; i < numChildren; i++) {
        pid[i].pidIndex = i + 1;
        pid[i].actualPid = fork();

        if (pid[i].actualPid == 0) { /* if this is the child process */
            sprintf(workerId, "%d", pid[i].pidIndex);
            execl("./worker", "./worker", workerId, NULL); /* exec it off */
        }
        else if (pid[i].actualPid < 0) {
            perror("[-]ERROR: Failed to fork CHILD process.\n");
            exit(errno);
        }
    }

    /* send signal to kill any remaining children */
//    for (i = 0; i < numChildren; i++) {
//        kill(pid[i].actualPid, SIGINT);
//    }

    /* wait for any remaining child processes to finish */
    while (wait(&wait_status) > 0) { ; }

    /* detach and remove the message queue, shared memory, and any allocated memory */
    free(pid);
    detachAndRemove(sharedMemId, shm);

    return 0;
}

/*************************************************!
 * @function    printHelpMessage
 * @abstract    Prints usage statement
 **************************************************/
void printHelpMessage(char* argv) {
    char usageChildren [1000];
    char usageHelp [1000];
    char usageError [1000];
    char titleMenu [1000];

    snprintf(titleMenu, sizeof titleMenu, "\n################################ HELP MENU ################################");
    snprintf(usageChildren, sizeof usageChildren, "\nUSAGE: %s -n <NUMBER> -s <NUMBER>\nDESCRIPTION: Executes worker processes with -n allowing a max number of workers with -s.\n", argv);
    snprintf(usageHelp, sizeof usageHelp, "\nUSAGE: %s -h\nDESCRIPTION: Displays help options.\n", argv);

    fprintf(stderr, "%s", titleMenu);
    fprintf(stderr, "%s", usageChildren);
    fprintf(stderr, "%s", usageHelp);
    fprintf(stderr, "%s", usageError);
    exit(EXIT_SUCCESS);
}

/*************************************************!
 * @function    r_wait
 * @abstract    waits but doesn't block
 * @param       stat_loc    return status
 * @citation    pg. 72 Unix Systems Programming
 **************************************************/
pid_t r_wait(int* stat_loc){
    int retval;

    while(((retval = wait(stat_loc)) == -1) && (errno == EINTR));
    return retval;
}

/*************************************************!
 * @function    whoAmI
 * @abstract    used for testing
 * @param       stat_loc    return status
 **************************************************/
int whoAmI(){
    printf("Child %d  My Parent is %d\n\n", getpid(), getppid());
    return 1;
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
        exit(0);
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
/***************************************************!
 * @function  getOptCheck
 * @abstract  checks the arguments provided to getOpt
 * @param     argc
 * @param     maxChildren
 * @param     argv
 * @param     myString
 **************************************************/
void getOptCheck(int argc, int maxChildren, char* const argv, char* myString) {
    if (optopt == 'n' && argc != 3) {
        snprintf(myString, sizeof myString, "\n%s: Error: Option -%c requires an integer argument.\n",
                 argv[0], optopt);
        fprintf(stderr, "%s", myString);
        exit(EXIT_FAILURE);

    } else if ((optopt == 'n' && numChildren < 0)) {
        snprintf(myString, sizeof myString,
                 "\n%s: Error: The number of workers must be greater than 0.\n", argv[0]);
        fprintf(stderr, "%s", myString);
        exit(EXIT_FAILURE);

    } else if (optopt == 's' && maxChildren < 0) {
        snprintf(myString, sizeof myString, "\n%s: Error: The maximum number of workers must be greater than 0.\n", argv[0]);

    } else if (argc < 2) {
        fprintf(stderr, "\n%s: Error: You must provide an argument.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}
#pragma clang diagnostic pop