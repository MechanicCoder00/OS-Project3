Readme for CMPSCI 4760 Assignment 3

Author : Scott Tabaka
Date Due : 3/16/2020
Course : CMPSCI 4760
Purpose : This program will process an input file with 2^n numbers of integers. It will then sum all of the integers
using two different methods all while utilizing semaphores to make sure only one process is allowed into the log file.

Instructions:

To compile: make

To clean up: make clean

To run	: master [-h] | [filename]

        -h Print a help message and exit.
        -i Change the name of the input file name(Default: "inputfile")

**To run with the specified pauses for the project, enable debug mode**

        Remove the "#" before the "--DDEBUG" in the makefile and recompile

Output files:

        adder_log
            -time a process acquires the semaphore
            -PID , starting element to sum, how many numbers to sum
            -time a process releases the semaphore

        waits_log
            -PID and time when a process starts to wait for the semaphore
            -PID and time when a process acquires the semaphore

Comparison of the two methods
    The second method will be faster as n grows larger, since the second method will run less sum operations
    than the first method.
