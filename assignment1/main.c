/**********************************************************************
 * Author: Chris Thompson
 * Project Name: ass1
 * Project Desc: The goal of this homework is to become familiar
 *               with the environment in hoare while practicing
 *               system calls.
 * Initial Commit: 29 August 2018
 * Github Repo: https://github.com/teufel0369/OS2.git
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*prototypes*/
void printHelpMessage();
void printErrorMessage(char*);
pid_t r_wait(int*);
int whoAmI();

/*************************************************!
 * @function    main
 * @abstract    orchestrates the madness
 * @param       argc    number of arguments.
 * @param       argv    array of arguments
 * @returns     0 if it runs correctly.
 **************************************************/
int main(int argc, const char* argv[]) {
    char* firstArg = NULL;
    char myString [1000];
    pid_t childPid = 0;
    int opt = 0;
    int i, numChildren;
    int processCount = 1; /* always start off with one child process */

    /*get the options from the argv array*/
    while((opt = getopt(argc, argv, "n:hp")) != -1){
        switch(opt){
            case 'n':
                numChildren = atoi(optarg);
                break;

            case 'h':
                printHelpMessage();
                break;

            case 'p':
                snprintf(myString, sizeof myString, "%s: Error: Detailed error message", argv[0]);
                printErrorMessage(firstArg);
                break;

            default:
                if(optopt == 'n' && argc != 3) {
                    fprintf(stderr, "\n[ERROR]: Option -%c requires an integer argument\n", optopt);
                    exit(EXIT_FAILURE);

                } else if(optopt == 'n' && numChildren <= 0){
                    printErrorMessage("\n[ERROR]: Number of children must be greater than zero\n");
                    exit(EXIT_FAILURE);
                }

                printHelpMessage();
                exit(EXIT_FAILURE);
        }


    }

    for(i = 0; i < numChildren; i++) {
        if(childPid = fork())
            break;
    }

    whoAmI(childPid);

    while(r_wait(NULL) > 0);  /* waiting for all the remaining child processes to finish */

    return 0;
}

/*************************************************!
 * @function    printHelpMessage
 * @abstract    Prints usage statement
 **************************************************/
void printHelpMessage() {
    char* usageChildren = "\nUSAGE: ./ass1 -n <number of child processes>\nDESCRIPTION: Executes a process chain with the desired number of children.\n";
    char* usageHelp = "\nUSAGE: ./ass1 -h\nDESCRIPTION: Displays help options.\n";
    char* usageError = "\nUSAGE: ./ass1 -p\nDESCRIPTION: Displays a test error message using the perror function.\n";

    fprintf(stderr, "%s", usageChildren);
    fprintf(stderr, "%s", usageHelp);
    fprintf(stderr, "%s", usageError);
    exit(EXIT_SUCCESS);
}

/*************************************************!
 * @function    printErrorMessage
 * @param       message    specific error message
 * @abstract    Prints error message
 **************************************************/
void printErrorMessage(char* message) {
    char* incorrectNumArgs = "\nERROR: There were an incorrect number of arguments entered.\n";
    message == NULL ? perror(incorrectNumArgs) : perror(message);
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

int whoAmI(){
    printf("Child %d  My Parent is %d \n\n", getpid(), getppid());
    return 1;
}