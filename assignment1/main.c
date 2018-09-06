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
void printHelpMessage(char*);
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
int main(int argc, char* const argv[]) {
    char myString [1000];
    pid_t childPid = 0;
    int opt = 0;
    int i, numChildren = 0;
    opterr = 0;

    /*get the options from the argv array*/
    while((opt = getopt(argc, argv, "n:hp")) != -1){
        switch(opt){
            case 'n':
                numChildren = atoi(optarg);
                break;

            case 'h':
                printHelpMessage(argv[0]);
                break;

            case 'p':
                snprintf(myString, sizeof myString, "\n%s: Error: Detailed error message\n", argv[0]);
                printErrorMessage(argv[0]);
                break;

            default:
                if(optopt == 'n' && argc != 3) {
                    snprintf(myString, sizeof myString, "\n%s: Error: Option -%c requires an integer argument.\n", argv[0], optopt);
                    fprintf(stderr, "%s", myString);
                    exit(EXIT_FAILURE);

                } else if(optopt == 'n' && numChildren < 0){
                    snprintf(myString, sizeof myString, "\n%s: Error: The number of child processes must be greater than 0.\n", argv[0]);
                    fprintf(stderr, "%s", myString);
                    exit(EXIT_FAILURE);
                }

                printHelpMessage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /* fork some child processes and break the child from the loop if the parent process successfully forked */
    if(numChildren > 0) {
        for(i = 0; i < numChildren; i++) {
            if((childPid = fork())){
                break;
            }
        }

        /* print out to ensure the processes are being forked by the parent */
        whoAmI();
    }

    /* waiting for all the remaining child processes to finish */
    while(r_wait(NULL) > 0);

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
    snprintf(usageChildren, sizeof usageChildren, "\nUSAGE: %s -n <number of child processes>\nDESCRIPTION: Executes a process chain with the desired number of children.\n", argv);
    snprintf(usageHelp, sizeof usageHelp, "\nUSAGE: %s -h\nDESCRIPTION: Displays help options.\n", argv);
    snprintf(usageError, sizeof usageError, "\nUSAGE: %s -p\nDESCRIPTION: Displays a test error message using the perror function.\n", argv);

    fprintf(stderr, "%s", titleMenu);
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
void printErrorMessage(char* argv) {
    char testErrorMessage [1000];
    snprintf(testErrorMessage, sizeof testErrorMessage, "\n%s: Error: Detailed error message\n", argv);
    perror(testErrorMessage);
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