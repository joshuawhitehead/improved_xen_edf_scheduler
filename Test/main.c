//
//  main.c
//  stubdom
//
//  CIS 467

#include <stdio.h>
#include <math.h>
#include <unistd.h>


/* Constant */
#define BUFFER_LENGTH 100
#define WORKLOAD_SIZE 10000000


void workloadProgramSum();
void workloadProgramNOP();
void workloadProgramBlocking();
void workloadProgramWhileTrue();


void clearBuffer(void) {
    char inputBuffer[BUFFER_LENGTH];
    if (fgets(inputBuffer, BUFFER_LENGTH, stdin) == NULL) {
        (void)printf("Problem with input buffer.");
        (void)exit(-1);
    }
}



int main(int argc, const char * argv[])
{
    char userInput = '\0';
    while (1) {
        (void)printf("\nPlease select a program to run.\n'0'\t-\tDefault\n");
        userInput = getchar();
        
        (void)clearBuffer();
        
        (void)sleep(1);
        
        (void)printf("Input: %d\n", userInput);
        
        switch (userInput) {
            case '3':
                (void)printf("Entering blocking state (waiting for user input).\n");
                
            case '2':
                (void)printf("Entering while(1) loop.\n");
                
            case '1':
                (void)printf("Running program 1\n");
                workloadProgramSum();
                break;
            case '0':
            default:
                (void)printf("Invalid program\n");
                workloadProgramNOP();
                break;
        }        
    }
    
    
    return 0;
}



/** test workload example **/
void workloadProgramSum() {
    int i,j;
    int sum = 0;
    for (i = 0; i < WORKLOAD_SIZE; i++) {
        // do some work
        sum += i + pow(0.123456, sqrt(sum*sum));
        
        // printf slows down the loop
        printf("%d\t",i);
    }
    printf("\n---end\n\n");
    return;
}

/** TODO: create a workload according to documentation **/
void workloadProgramNOP() {
    (void)printf("An empty function has been found...\n");
    sleep(1);
    (void)printf("Nothing happened...\n");
    return;
}


// blocking state
// waiting for char
void workloadProgramBlocking() {
    (void)printf("Waiting for user input.\n");
    (void)getchar();
    return;
}


// while (1)
void workloadProgramWhileTrue() {
    while (1) {
        workloadProgramSum();
    }
}
