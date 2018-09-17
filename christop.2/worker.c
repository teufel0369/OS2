#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* const argv[]) {

    int workerIndexNumber = atoi(argv[1]);


    printf("Hello World, Worker%d", workerIndexNumber);
}
