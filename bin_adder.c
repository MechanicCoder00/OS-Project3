#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#define SHMKEY 849213414        //custom key for shared memory
#define SIZE sizeof(int)        //size of each element in a shared memory array

sem_t *mutex;                   //shared memory pointer to semaphore
int *fixedArray;                //shared memory pointer to clock simulator array
int *variableArray;             //shared memory pointer to prime array
FILE *output;                   //variable for file output "adder_log"
FILE *output2;                  //variable for file output "waits_log"
time_t waitTime;                            //variable for storing the wait time
char outputFileName[10] = "adder_log";      //name of the output file
char outputFileName2[10] = "waits_log";     //name of the output2 file

void childCleanup()                     //function to detach child from shared memory
{
    shmdt(mutex);                                                       //detach from shared memory
    shmdt(fixedArray);                                                  //detach from shared memory
    shmdt(variableArray);                                               //detach from shared memory
}

void handle_sigterm()                    //signal handler for terminate signal
{
    childCleanup();                     //function call to clean up child shared memory
    exit(1);
}

int sumRange(int startingIndex, int range)      //function to sum numbers together
{
    int tempNum = 0;
    int i=0;
    for(i=0;i<range;i++)                        //loops through the given numbers to sum
    {
        tempNum += variableArray[startingIndex+i];
    }
    return tempNum;
}

void storeTime()                //function to store current time into "waitTime" variable
{
    time_t clk = time(NULL);
    waitTime = clk;
}

void printWaitTimeOutput2(char* message, char* message2)        //function to print wait time and semaphore acquire time to "waits_log"
{
    fprintf(output2, "%-20s %d ", message, getpid());
    fprintf(output2, "%s", ctime(&waitTime));

    fprintf(output2, "%-20s %d ", message2, getpid());
    time_t clk = time(NULL);
    fprintf(output2, "%s", ctime(&clk));
}

void printMessageToOutput(char* message)        //function to print message with time to "adder_log"
{
    fprintf(output, "%-27s ", message);
    time_t clk = time(NULL);
    fprintf(output, "%s", ctime(&clk));
}

void printMessageToStderr(char* message)        //function to print message with time to standard error
{
    fprintf(stderr, "%-27s ", message);
    time_t clk = time(NULL);
    fprintf(stderr, "%s", ctime(&clk));
}

int getRandomNumber(int min, int max)           //function to generate a random number within the ranges given
{
    srand(time(0));
    int randomNumber = (min + (rand() % ((max - min) +1 )));
    return randomNumber;
}


int main (int argc, char *argv[])
{
    signal(SIGTERM, handle_sigterm);        //initialization of termination signal handler

    int shmid0 = shmget(SHMKEY, 4, 0600 | IPC_CREAT);       //creates shared memory id for the semaphore
    if (shmid0 == -1)
    {
        perror("Shared memory0C:");                         //if shared memory does not exist print error message and exit
        return 1;
    }
    mutex = (sem_t*)shmat(shmid0,NULL,0);                   //attaches semaphore to shared memory segment
    if (mutex == (sem_t*)-1)
    {
        perror("Mutex shmat 0C:");                          //if shared memory does not exist print error message and exit
        return 1;
    }

    int shmid1 = shmget (SHMKEY+1, (3*SIZE)+1, 0600 | IPC_CREAT);     //creates shared memory id for the fixed size array
    if (shmid1 == -1)
    {
        perror("Shared memory1C:");                                   //if shared memory does not exist print error message and exit
        return 1;
    }

    fixedArray = (int *)shmat(shmid1, 0, 0);                        //attaches to the fixed shared memory array
    int totalNumOfIntegers = fixedArray[0];

    int shmid2 = shmget(SHMKEY+2, (totalNumOfIntegers*SIZE)+1, 0600 | IPC_CREAT);    //creates shared memory id for the variable size array
    if (shmid2 == -1)
    {
        perror("Shared memory2C:");                                  //if shared memory does not exist print error message and exit
        return 1;
    }

    variableArray = (int *)shmat(shmid2, 0, 0);                     //attaches to the variable size shared memory array

    int startingIndex = atoi(argv[0]);      //index to start summation
    int numOfNumbers = atoi(argv[1]);       //number of numbers to sum

    variableArray[startingIndex] = sumRange(startingIndex,numOfNumbers);    //function call to sum numbers and store them in variable array

#ifdef DEBUG            //DEBUG MODE
    int i;
    for(i=0;i<5;i++)    //loops 5 times
    {
//        printf ("Sleep0\n");
        int num = getRandomNumber(0,3);     //gets a random number and stores in "num" variable
//        printf("Going to sleep for %d: %d loop:%d\n", num, getpid(),i);
        sleep(num);     //waits for "num" seconds
#endif                  //END DEBUG MODE

    storeTime();        //function call to store time when arriving at sem_wait
    sem_wait(mutex);    //semaphore decrement
    //*********************************Entering Critical Section*****************************
    output = fopen(outputFileName, "a");        //open file for appending
    output2 = fopen(outputFileName2, "a");      //open file for appending

    printMessageToStderr("Entering Critical Section");      //function call to print to standard error output
    printMessageToOutput("Semaphore Acquired");             //function call to print to "adder_log"
    printWaitTimeOutput2("Process waiting:", "Semaphore Acquired");     //function call to print to "waits_log"

//    fixedArray[1] = fixedArray[1]+1;      //increments shared memory element to calculate total operations per summation method(for debugging)

#ifdef DEBUG        //DEBUG MODE
//        printf ("Sleep1\n");
        sleep(1);       //wait 1 second
#endif              //END DEBUG MODE


    fprintf(output, "%-7d %-3d %-5d\n", getpid(), startingIndex, numOfNumbers);     //prints PID, starting index to sum, number of numbers to sum,

#ifdef DEBUG        //DEBUG MODE
//        printf ("Sleep2\n");
        sleep(1);       //wait 1 second
#endif              //END DEBUG MODE

    printMessageToStderr("Exiting Critical Section");       //function call to print to standard error output
    printMessageToOutput("Releasing Semaphore");            //function call to print to "adder_log"

    fclose(output);             //close adder_log
    fclose(output2);            //close waits_log

    //*********************************Exiting Critical Section*****************************
    sem_post(mutex);
#ifdef DEBUG        //DEBUG MODE
    }
#endif              //END DEBUG MODE

    childCleanup();     //function call to clean up child shared memory

    return 0;
}