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
#include <zconf.h>
#include <unistd.h>
#include <stdlib.h>

/*************************************************!
 * @function    main
 * @abstract    orchestrates the madness
 * @param       argc    number of arguments.
 * @param       argv    array of arguments
 * @returns     0 if it runs correctly.
 **************************************************/
int main(int argc, const char* argv[]) {
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
                // TODO: create function for standard error message
                break;
            default:
                printHelpMessage();
                exit(EXIT_FAILURE);
        }

        if(numChildren <= 0){
            printErrorMessage(NULL);
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

/*************************************************!
 * @function    printHelpMessage
 * @abstract    Prints usage statement
 **************************************************/
void printHelpMessage() {
    char* usageChildren = "\nUsage: ass1 -n <number of child processes>\nExecutes a process chain with the desired number of children.\n";
    char* usageHelp = "\nUsage: ass1 -h\nDisplays help options.\n";
    char* usageError = "\nUsage: ass1 -p\nDisplays a test error message using the perror function.\n";

    perror(usageChildren);
    perror(usageHelp);
    perror(usageError);
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
    printHelpMessage();
}