#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#define SHMKEY 849213414            //custom key for shared memory
#define SIZE sizeof(int)            //size of each element in a shared memory array
#define CHILDLIMIT 19               //limit for total child processes allowed at once

/*
 * Project : Operating systems assignment 3
 * Author : Scott Tabaka
 * Date Due : 3/16/2020
 * Course : CMPSCI 4760
 * Purpose : This program will process an input file with 2^n numbers of integers. It will then sum all of the integers
 * using two different methods all while utilizing semaphores to make sure only one process is allowed into the log
 * file.
 */

int shmid0;                 //variable to store id for shared memory segment 0
int shmid1;                 //variable to store id for shared memory segment 1
int shmid2;                 //variable to store id for shared memory segment 2
sem_t *mutex;               //shared memory pointer to semaphore
int *fixedArray;            //shared memory pointer to array
int *variableArray;         //shared memory pointer to array
int activeChildren = 0;     //variable to keep track of active children
FILE *input;                //variable for file input
FILE *output;               //variable for file output "adder_log"
FILE *output2;              //variable for file output "waits_log"
FILE *output3;              //variable for storing auxiliary file output used for creating input files
int totalNumOfIntegers = 0; //variable for storing the number of integers contained in the input file
char inputFileName[255] = "inputfile";      //default name for input file
char outputFileName[10] = "adder_log";      //name of the output file
char outputFileName2[10] = "waits_log";     //name of the output2 file
int opt;                    //variable for checking options
static char usage[100];     //array for storing a usage message
static char error[100];     //array for storing a error message


void printHelp()            //function to print help info
{
    printf("%s\n\n", usage);
    printf("This program will process an input file with 2^n numbers of integers. It will then sum all of the integers\n"
           "using two different methods all while utilizing semaphores to make sure only one process is allowed into the "
           "log file.\n\n");
    printf("-h Print a help message and exit.\n");
    printf("-i Change the name of the input file name(Default: \"%s\"\n",inputFileName);

    exit(0);
}

void parentCleanup()            //function to cleanup shared memory and semaphores
{
    sem_destroy(mutex);                                                 //remove the semaphore
    shmdt(mutex);                                                       //detach from shared memory
    shmdt(fixedArray);                                                  //detach from shared memory
    shmdt(variableArray);                                               //detach from shared memory
    shmctl(shmid0,IPC_RMID,NULL);                                       //remove shared memory segment
    shmctl(shmid1,IPC_RMID,NULL);                                       //remove shared memory segment
    shmctl(shmid2,IPC_RMID,NULL);                                       //remove shared memory segment
}

void condenseArray(int numbersToCondense, int groupSize)        //function to move the newly added elements in the array to consecutive elements
{
    int fromPos = groupSize;
    int toPos = 1;
    int i;
    for(i=0;i<numbersToCondense;i++)
    {
        variableArray[toPos] = variableArray[fromPos];
        fromPos += groupSize;
        toPos++;
    }
}

void initializeMessages(char* str)      //function to set text for usage and error messages
{
    strcpy(error,str);
    strcat(error,": Error:");

    strcpy(usage,"Usage: ");
    strcat(usage,str);
    strcat(usage," [-h] | [-i inputfilename]");
}

int isInteger(char* line)          //function to check if char array input is a valid integer or not
{
    int i, len = strlen(line);

    for (i = 0; i < len; ++i)       //loop for checking each character of the input char array
    {
        if (!isdigit(line[i]))     //check if character is a digit or not
        {
            return 0;               //if not a number
        }
    }
    return 1;                       //if all characters were numbers
}

void handle_sigint()                    //signal handler for interrupt signal(Ctrl-C)
{
    printf("\nProgram aborted by user --> %s\n",outputFileName);        //displays end of program message to user
    parentCleanup();
    kill(0,SIGTERM);
    while (waitpid(-1, NULL, WNOHANG) > 0);                             //wait until all children have terminated
    while (activeChildren > 0);

    exit(1);
}

void handle_sigalarm()          //signal handler for alarm signal(for time out condition)
{
    printf("\nTimeout - Ending program --> %s\n",outputFileName);       //displays end of program message to user
    parentCleanup();
    kill(0,SIGTERM);
    while (waitpid(-1, NULL, WNOHANG) > 0);                             //wait until all children have terminated
    while (activeChildren > 0);

    exit(1);
}

void handle_sigchild()      //signal handler for child termination
{
    while (waitpid(-1, NULL, WNOHANG) > 0)          //handle each child that has terminated
    {
        activeChildren--;                           //decrement number of active children
    }
}

int countLinesInFile()      //function to count the lines in the input file
{
    char *line = NULL;
    size_t len = 0;

    int lineCount=0;

    while ((getline(&line, &len, input)) != -1)
    {
        lineCount++;
    }
    return lineCount;
}

void readLinesInFileToArray()       //read each line in input file into the shared memory array
{
    char *line = NULL;
    size_t len = 0;
    input = fopen(inputFileName, "r");
    int i=0;

    while ((getline(&line, &len, input)) != -1)
    {
        variableArray[i] = atoi(line);
        i++;
    }
}

int getRandomNumber(int min, int max)       //function to return a random number seeded by time(needs atleast 1 sec between calls to get different random number
{
    srand(time(0));
    int randomNumber = (min + (rand() % ((max - min) +1 )));
    return randomNumber;
}

void createNewInputFile(int powerOf2, char* newFileName)        //function to create new input file with random numbers
{
    output3 = fopen(newFileName, "w");
    int i;
    for(i=0;i<(int)pow(2,powerOf2);i++)
    {
        fprintf(output3,"%d\n",getRandomNumber(0,256));
        sleep(1);
        printf("Creating number %d/%d\n",i+1,(int)pow(2,powerOf2));
    }
    fclose(output3);

    printf("new input file created --> %s\n", newFileName);
    exit(0);
}

void validateFile()     //function to check input file for the correct amount of integers and that each line is an integer
{
    if(log2(totalNumOfIntegers)-ceil(log2(totalNumOfIntegers)) != 0)
    {
        fprintf(stderr, "%s Number of integers in \"%s\" are not a power of 2\n", error,inputFileName);
        exit(1);
    }

    input = fopen(inputFileName, "r");
    char *line = NULL;
    size_t len = 0;
    int i=1;
    while ((getline(&line, &len, input)) != -1)
    {
        line[strcspn(line, "\r\n")] = 0;
        if(isInteger(line) == 0)
        {
            fprintf(stderr, "%s Line# %d in \"%s\" is not an integer\n", error, i, inputFileName);
            exit(1);
        }
        i++;
    }
}

void printVarArray(char* s)     //function to print the entire shared memory array of integers(for debuging)
{
    printf("%s\n", s);
    int i;
    for (i = 0; i < totalNumOfIntegers; i++)
    {
        printf("[%d]:%d\n", i, variableArray[i]);
    }
}

int main (int argc, char *argv[])
{
//    createNewInputFile(3,"inputfile");    //function to create a new input file with randomized numbers(0-256)

    signal(SIGINT, handle_sigint);          //initialization of signals and which handler will be used for each
    signal(SIGALRM, handle_sigalarm);
    alarm(100);                               //will send alarm signal after 100 seconds
    signal(SIGCHLD, handle_sigchild);

    output = fopen(outputFileName, "w");                //opens output file for writing(if file not present it will create file)
    fclose(output);                                     //close output file
    output2 = fopen(outputFileName2, "w");               //opens output file for writing(if file not present it will create file)
    fclose(output2);                                    //close output file

    initializeMessages(argv[0]);            //function call to set error and usage message text
    opterr = 0;                             //disables some system error messages(using custom error messages so this is not needed)

    while ((opt = getopt(argc, argv, "hi:")) != -1)		//loop for checking option selections
    {
        switch (opt)
        {
            case 'h':                                   //option h
                printHelp();                //function call to print help message
                break;
            case 'i':                                   //option h
                strcpy(inputFileName, optarg);
                break;
            case '?':                                   //check for arguments
                if (optopt == 'i')
                {
                    fprintf(stderr, "%s Option -%c requires an argument.\n", error, optopt);
                } else {
                    fprintf(stderr, "%s Unknown option character '-%c'\n", error, optopt);
                }
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "%s\n", usage);
                exit(EXIT_FAILURE);
        }
    }

    if(argc-optind > 0)             //Checks to make sure there is only 1 argument, if any, after options
    {
        printf("%s: Too many arguments\n",error);
        fprintf(stderr, "%s\n", usage);
        exit(EXIT_FAILURE);
    }

    input = fopen(inputFileName, "r");              //open input file for reading
    if (input == NULL)
    {
        printf("%s: Cannot open file -- \"%s\"\n",error,inputFileName);
        exit(EXIT_FAILURE);
    }

    totalNumOfIntegers = countLinesInFile();        //set variable to the number of integers in file
    validateFile();                                 //function call to validate input file

    printf("Program has started -- Reading %d integers from \"%s\"\n", totalNumOfIntegers, inputFileName);  //Program start message

    shmid0 = shmget(SHMKEY, 4, 0600 | IPC_CREAT);            //creates shared memory id for the semaphore
    if (shmid0 == -1)
    {
        perror("Shared memory0P:");                          //if shared memory does not exist print error message and exit
        return 1;
    }
    mutex = (sem_t*)shmat(shmid0,NULL,0);                    //attaches semaphore to shared memory segment
    if (mutex == (sem_t*)-1)
    {
        perror("Mutex shmat 0P:");                           //if shared memory does not exist print error message and exit
        return 1;
    }
    sem_init(mutex,1,1);                                     //set semaphore initially to 1

    shmid1 = shmget(SHMKEY+1, (3*SIZE)+1, 0600 | IPC_CREAT);     //creates shared memory id for the fixed size array
    if (shmid1 == -1)
    {
        perror("Shared memory1P:");                           //if shared memory does not exist print error message and exit
        return 1;
    }

    fixedArray = (int *)shmat(shmid1, 0, 0);                  //attaches to the fixed shared memory array

    shmid2 = shmget(SHMKEY+2, (totalNumOfIntegers*SIZE)+1, 0600 | IPC_CREAT);     //creates shared memory id for the variable size array
    if (shmid2 == -1)
    {
        perror("Shared memory2P:");                           //if shared memory does not exist print error message and exit
        return 1;
    }

    variableArray = (int *)shmat(shmid2, 0, 0);                     //attaches to the variable size shared memory array

    fixedArray[0] = totalNumOfIntegers;                             //stores how many integers there are
    fixedArray[1] = 0;                                              //used to count how many sum operation occur(for debugging)
    fixedArray[2] = 0;                                              //not used

    readLinesInFileToArray();                                       //function call to read input file into variable shared memory array

    //**********Sum method 1************
    int indexToSum = 0;
    int totalNumOfPasses = (int)log2(totalNumOfIntegers);           //calculate total number of passes needed

    int passIterator;
    for(passIterator = 0; passIterator<totalNumOfPasses; passIterator++)    //pass loop
    {
        indexToSum = 0;
        int totalElementsInPass = totalNumOfIntegers/(2*(int)pow(2,passIterator));

        int elementIterator = 0;
        while(elementIterator < totalElementsInPass)        //operation loop
        {
            if(activeChildren < CHILDLIMIT)     //makes sure there is never more children than the child limit
            {
                activeChildren++;

                int pid = fork();                                           //fork call
                if (pid == 0)                                               //child process
                {
                    char str1[20];
                    snprintf(str1, sizeof(str1), "%d", indexToSum);              //copies process number into a char array
                    char str2[20];
                    snprintf(str2, sizeof(str2), "%d", 2);
                    execl("bin_adder", str1, str2, NULL);                        //exec call
                }

                elementIterator++;
                indexToSum += 2;                                                 //increment index for child starting index
            }
        }

        while(activeChildren > 0);                                      //program will wait for all processes in current pass to complete
        condenseArray(totalElementsInPass, 2);                          //function call to condense the variable shared memory array
    }
    while(activeChildren > 0);                                          //program will wait here to make sure all active child processes have been terminated

//    printf("Total # of operations for method 1: %d\n", fixedArray[1]);      //prints # of operations for each method(for debugging)
//    fixedArray[1] = 0;                                                      //reset counter for # of operations(for debugging)

//    printVarArray("Array Contents using n/2:");                             //function call to print all contents of the variable shared memory array(for debugging)
    printf("Total Sum of all %d integers using n/2 in \"%s\": %d\n",totalNumOfIntegers,inputFileName,variableArray[0]); //prints sum of integers

    readLinesInFileToArray();       //function call to read input file into variable shared memory array
    fclose(input);                  //close input file

    //**********Sum method 2************
    int groupSize = (int)log2(totalNumOfIntegers);      //variable for calculating the group size
    int totalElementsInPass;                            //variable for storing how many group summations need to happen on current pass
    int fullElementsInPass;                             //variable for storing how many full group summations need to happen on current pass
    int remainderElementsInPass;                        //variable for storing how many numbers are in the last group
    int totalElementsInPrevPass = totalNumOfIntegers;   //variable for storing how many group summations happened on previous pass
    int elementIterator = 0;                            //variable for storing starting element number to be sent to child for summation
    int finalPassFlag = 0;                              //variable for storing flag to terminate pass loop

    while(finalPassFlag == 0)           //pass loop
    {
        indexToSum = 0;
        elementIterator = 0;
        totalElementsInPass = ceil(totalElementsInPrevPass/(double)groupSize);
        fullElementsInPass = totalElementsInPass - 1;
        remainderElementsInPass = totalElementsInPrevPass - (groupSize * fullElementsInPass);

        while(elementIterator < fullElementsInPass)     //operation loop
        {
            if(activeChildren < CHILDLIMIT)         //makes sure there is never more children than the child limit
            {
                activeChildren++;

                int pid = fork();                                           //fork call
                if (pid == 0)                                               //child process
                {
                    char str1[20];
                    snprintf(str1, sizeof(str1), "%d", indexToSum);              //copies process number into a char array
                    char str2[20];
                    snprintf(str2, sizeof(str2), "%d", groupSize);
                    execl("bin_adder", str1, str2, NULL);                        //exec call
                }

                elementIterator++;
                indexToSum += groupSize;
            }
        }
//**************Last element of pass calculated here
        elementIterator = 0;
        while(elementIterator < 1)                  //Loop through last group of current pass
        {
            if(activeChildren < CHILDLIMIT)
            {
                activeChildren++;

                int pid = fork();                                           //fork call
                if (pid == 0)                                               //child process
                {
                    char str1[20];
                    snprintf(str1, sizeof(str1), "%d", indexToSum);              //copies process number into a char array
                    char str2[20];
                    snprintf(str2, sizeof(str2), "%d", remainderElementsInPass);
                    execl("bin_adder", str1, str2, NULL);                        //exec call
                }
                elementIterator++;
            }
        }

        if(totalElementsInPass == 1)        //will set loop termination flag if pass is running only 1 group for summation
        {
            finalPassFlag = 1;
        }
        while(activeChildren > 0);          //program will wait here to make sure all active child processes have been terminated

        condenseArray(totalElementsInPass, groupSize);      //function call to condense the variable shared memory array
        totalElementsInPrevPass = totalElementsInPass;      //stores current groups in pass
        groupSize = 2;
    }

//    printf("Total # of operations for method 2: %d\n", fixedArray[1]);    //prints # of operations for each method(for debugging)
//    printVarArray("Array Contents using n/log(n):");                      //reset counter for # of operations(for debugging)
    printf("Total Sum of all %d integers using n/log(n) in \"%s\": %d\n",totalNumOfIntegers,inputFileName,variableArray[0]); //prints sum of integers

    parentCleanup();                                                        //function call to clean up parent shared memory

    printf("Program completed successfully --> %s\n",outputFileName);                         //completed message for user

    return 0;
}