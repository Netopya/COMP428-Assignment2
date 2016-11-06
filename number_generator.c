/*********************************************************************
Code for COMP 428 Assignment 2
Michael Bilinsky 26992358
This program generates a sequence of random numbers
    and saves them to a file
The number of numbers generated can be specified as
    a command line arguement
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUMBER_OF_NUMBERS 10000000
#define MAX_NUMBER 1000000

int main (int argc, char *argv[])
{
    // seed the randomizer
    unsigned seed =  (unsigned)(time(0));
    srand(seed);
    
    int numbersOfNumbers;
    
    // If an argument is specified, use it
    // as the number of inputs to generate
    if(argc < 2)
    {
        numbersOfNumbers = NUMBER_OF_NUMBERS;
    }
    else
    {
        numbersOfNumbers = strtol(argv[1], NULL, 10);
    }
    
    // Open the file for writing
    FILE *myFile;
    myFile = fopen("input.txt", "w");
       
    // generate numbers and output them to a file
    int i;
    for(i = 0; i < numbersOfNumbers - 1; i++)
    {
        fprintf(myFile, "%d ", rand() % MAX_NUMBER);
    }
    
    // write the last number without a delimiting space
    fprintf(myFile, "%d", rand() % MAX_NUMBER);
    
    fclose(myFile);
}