/*********************************************************************
Code for COMP 428 Assignment 2
Michael Bilinsky 26992358
This program performs quick sort on a sequence of numbers
The input is read from a input.txt file, see number_generator.c
    to generate this input file
Execution time is measured
The number of inputs can be limited by passing a number as a parameter
    to this program
**********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// A function to compare two numbers
// to be used in the qsort function
int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int main (int argc, char *argv[])
{
    // start the clock
    clock_t begin = clock();
        
    int* sequence;
    int inputSize;
    
    // Open the input file
    FILE *myFile;
    myFile = fopen("input.txt", "r");
    
    // If no arguments are specified, count the number of lines in the file
    if(argc < 2)
    {
        inputSize = 0;
        int input;
        while(!feof(myFile))
        {
            
            fscanf(myFile, "%d", &input);
            inputSize++;
        }
        
        rewind(myFile);
    }
    else
    {
        inputSize = strtol(argv[1], NULL, 10);
    }
    
    printf("Input size is %d\n", inputSize);
    
    // Read the input from the file
    sequence = malloc(inputSize * sizeof(int));
    int j;        
    for (j = 0; j < inputSize; j++)
    {
        if(feof(myFile))
        {
            printf("The file contained less input elements than specified\n");
            return 1;
        }
        
        fscanf(myFile, "%d", &sequence[j]);
    }
    
    fclose(myFile);

    // Perform the sort
    qsort(sequence, inputSize, sizeof(int), compare);
    
    // Verify that the sequence is indeed sorted
    int success = 1;
    int i;
    for(i = 0; i < inputSize - 1; i++)
    {
        if(sequence[i] > sequence[i + 1])
        {
            success = 0;
            break;
        }
    }
    
    if(success)
    {
        printf("x %d elements sorted successfully\n", inputSize);
    }
    else
    {
        printf("x Failed to sort %d elements\n", inputSize);
    }
    
    // Write the output to a file
    myFile = fopen("output_serial.txt", "w");
    for(i = 0; i < inputSize; i++)
    {
        fprintf(myFile, "%d ", sequence[i]);
    }
    fclose(myFile);
    
    // Measure the execution time
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
    printf("\nProgram took %10.8f seconds\n",time_spent);
}