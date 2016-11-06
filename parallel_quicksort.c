/*********************************************************************
Code for COMP 428 Assignment 2
Michael Bilinsky 26992358
This program performs quick sort on a sequence of 
    numbers over a parallel hypercube topology
The input is read from a input.txt file, see number_generator.c
    to generate this input file
Execution time is measured
OpenMPI's -np parameter must be a power of 2
The number of inputs can be limited by passing a number as a parameter
    to this program
**********************************************************************/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MASTER 0        /* task ID of master task */

// A helper function to print a buffer of numbers
// to the console
void printBuffer(int buffer[], int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        printf(" %d", buffer[i]);
    }
    printf("\n");
}

// A helper function to check if a number is a power of 2
int IsPowerOfTwo(int x)
{
    return (x & (x - 1)) == 0;
}

// A function to compare two numbers
// to be used in the qsort function
int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int main (int argc, char *argv[])
{

int	taskid,	        /* task ID - also used as seed number */
	numtasks,       /* number of tasks */
	rc,             /* return code */
	i;

    // Start the timer
    clock_t begin = clock();;

    MPI_Status status;
    
    /* Obtain number of tasks and task ID */
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);

    int numRounds = log2(numtasks);
    int* sequence;
    int inputSize;
    
    // Seed the random number generator
    unsigned seed =  (unsigned)(time(0) + taskid);
    srand(seed);
    
    // Only the master will deal with reading the file
    if(taskid == MASTER)
    {                
        printf ("%d rounds will execute \n", numRounds);
    
        // Abort if the number of processors is not a power of 2
        if(!IsPowerOfTwo(numtasks))
        {
            printf ("The number of processes must be a power of 2\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return rc;
        }
        
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
        
        sequence = malloc(inputSize * sizeof(int));
        int j;       

        // Read the input values from the file
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
    }

    int scounts[numtasks];
    int displs[numtasks];
    
    displs[0] = 0;
    
    // Broadcast the number of inputs
    MPI_Bcast(&inputSize, sizeof(inputSize), MPI_INT, MASTER, MPI_COMM_WORLD);
    
    // Non-master processes will need to create the sequence buffer for themselves
    if(taskid != MASTER)
    {
        sequence = malloc(inputSize * sizeof(int));
    }
    
    // Calculate the number of elements each process will initial receive
    int baseCount = inputSize / numtasks;
    int remainder = inputSize % numtasks;
    
    for(i = 0; i < numtasks; i++)
    {
        scounts[i] = baseCount;
        
        if(i < remainder)
        {
            scounts[i]++;
        }
        
        if(i != 0)
        {
            displs[i] = displs[i - 1] + scounts[i - 1];
        }
    }
    
    // Broadcast the sequence to all processors
    MPI_Bcast(sequence, inputSize, MPI_INT, MASTER, MPI_COMM_WORLD);

    int currentBufferSize = scounts[taskid];
    int *currentBuffer;
    currentBuffer = malloc(currentBufferSize * sizeof(int));
    
    // Each process will grab their chunk of the input sequence
    for(i = 0; i < scounts[taskid]; i++)
    {
        currentBuffer[i] = sequence[i + displs[taskid]];
    }
    
    MPI_Comm comm = MPI_COMM_WORLD;
    
    for(i = 0; i < numRounds; i++)
    {        
        int j;
        int pivot; // The selected pivot
        int groupRank, groupSize; // The rank and size of the current comm world
        MPI_Comm_rank(comm, &groupRank);
        MPI_Comm_size(comm, &groupSize);
        
        // Each sub world's master is responsible for picking a pivot
        if(groupRank == MASTER)
        {   
            if(currentBufferSize != 0)
            {
                // Randomly select a number from the current buffer
                int r = rand() % currentBufferSize;            
                pivot = currentBuffer[r];
            }
            else
            {
                // A pivot of -1 is an invalid pivot
                pivot = -1;
            }
        }
        
        // Broadcast the pivot to all processes in the current sub comm world
        MPI_Bcast(&pivot, 1, MPI_INT, MASTER, comm);
        
        // If the master could not find a pivot, then all processes
        // in the sub comm world will need to suggest a pivot
        if(pivot < 0)
        {
            // If the process has some input, randomly select a number from it
            if(currentBufferSize != 0)
            {
                int r = rand() % currentBufferSize;            
                pivot = currentBuffer[r];
            }
            else
            {
                pivot = -1;
            }
            int* pivots;
            pivots = malloc(groupSize * sizeof(int));
            
            // The master will collect pivots from all of the process in its sub comm world
            MPI_Gather(&pivot, 1, MPI_INT, pivots, 1, MPI_INT, MASTER, comm);
            
            pivot = -1;
            
            // The master will look for the first process that has a valid pivot and select it
            if(groupRank == MASTER)
            {   
                for(j = 0; j < groupSize; j++)
                {
                    if(pivots[i] >= 0)
                    {
                        pivot = pivots[i];
                        break;
                    }
                }
            }            
            
            free(pivots);
            
            // Broadcast the new pivot
            MPI_Bcast(&pivot, 1, MPI_INT, MASTER, comm);
            
            // If a pivot was still not found, split the comm world and continue to the next round
            if(pivot < 0)
            {
                MPI_Comm_split(comm, groupRank & ((groupSize - 1) << (numRounds - 1 - i)), groupRank, &comm);
                continue;
            }
        }
        
        // Count the number of inputs above and below the pivot
        int maxcount = 0, mincount = 0;
        for(j = 0; j < currentBufferSize; j++)
        {
            if(currentBuffer[j] > pivot)
            {
                maxcount++;
            }
            else
            {
                mincount++;
            }
        }
        
        // Create the buffers to hold the max and min values
        int* max = malloc(maxcount * sizeof(int));
        int* min = malloc(mincount * sizeof(int));
        
        // Place the max and min values from the current buffer into their respective buffers
        int maxpos = 0, minpos = 0;
        for(j = 0; j < currentBufferSize; j++)
        {
            if(currentBuffer[j] > pivot)
            {
                max[maxpos] = currentBuffer[j];
                maxpos++;
            }
            else
            {
                min[minpos] = currentBuffer[j];
                minpos++;
            }
        }
        
        // The bit representing the dimension along which communication will occur
        int toggleBit = 1 << (numRounds - i - 1);
        
        // The Id of the partner
        int partner = groupRank ^ toggleBit;
        
        int pMinSize = 0;
        int pMaxSize = 0;
        
        // The greater process will send its minimum values to its partner
        //     and receive its partner's maximum values
        // The lesser process will receive its partner's minimum values
        //     and send its maximum values
        if(groupRank > partner)
        {
            // Send the number of minimum values, followed by the minimum values themselves
            MPI_Send(&mincount, 1, MPI_INT, partner, 0, comm);
            MPI_Send(min, mincount, MPI_INT, partner, 0, comm);
            
            // Read the number of max values the partner is sending
            MPI_Recv(&pMaxSize, 1, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
            
            // Allocate the buffer and read the max values from the partner
            int* pMaxBuffer = malloc(pMaxSize * sizeof(int));
            MPI_Recv(pMaxBuffer, pMaxSize, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
            
            // Create the new current buffer
            free(currentBuffer);
            currentBufferSize = maxcount + pMaxSize;
            currentBuffer = malloc(sizeof(int) * (currentBufferSize));
            
            // place the current process's max values and those from the partner
            // into the current buffer
            for(j = 0; j < maxcount; j++)
            {
                currentBuffer[j] = max[j];
            }
            
            for(j = 0; j < pMaxSize; j++)
            {
                currentBuffer[maxcount + j] = pMaxBuffer[j];
            }
            
            free(pMaxBuffer);
        }
        else
        {
            // Receive the number of minimum inputs from the partner
            MPI_Recv(&pMinSize, 1, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
            
            // Allocate the buffer and read the min values from the partner
            int* pMinBuffer = malloc(pMinSize * sizeof(int));
            MPI_Recv(pMinBuffer, pMinSize, MPI_INT, partner, 0, comm, MPI_STATUS_IGNORE);
            
            // Send the number of maximum values, followed by the maximum values themselves
            MPI_Send(&maxcount, 1, MPI_INT, partner, 0, comm);
            MPI_Send(max, maxcount, MPI_INT, partner, 0, comm);
            
            // Create the new current buffer
            free(currentBuffer);
            currentBufferSize = mincount + pMinSize;
            currentBuffer = malloc(sizeof(int) * (currentBufferSize));
            
            // place the current process's max values and those from the partner
            // into the current buffer
            for(j = 0; j < mincount; j++)
            {
                currentBuffer[j] = min[j];
            }
            
            for(j = 0; j < pMinSize; j++)
            {
                currentBuffer[mincount + j] = pMinBuffer[j];
            }
            
            free(pMinBuffer);
        }
        
        free(min);
        free(max);
        
    
        // Spilt the current comm world into two worlds depending on the value of the processor id's
        //    most significant bit relative to the current round
        MPI_Comm_split(comm, groupRank & ((groupSize - 1) << (numRounds - 1 - i)), groupRank, &comm);
    }
    
    // Locally sort the current buffer
    qsort(currentBuffer, currentBufferSize, sizeof(int), compare);
    
    // The master will now gather the sorted values from all the other processes,
    // verify the sequence, and write it to an output file
    if(taskid == MASTER)
    {
        // Place the master's buffer into the sequence
        int position = 0;
        for(i = 0; i < currentBufferSize; i++)
        {
            sequence[i] = currentBuffer[i];
        }
        position = currentBufferSize;
        
        // Go through each process and get their inputs
        for(i = 1; i < numtasks; i++)
        {
            // Read how many inputs the process has
            int oSize;
            MPI_Recv(&oSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // If the process has no inputs, continue
            if(oSize == 0)
            {
                continue;
            }
            
            // Allocate the buffer and read the inputs from the process
            int* oBuffer = malloc(oSize * sizeof(int));
            MPI_Recv(oBuffer, oSize, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Copy the process's values into the final sequence
            int j;
            for(j = 0; j < oSize; j++)
            {
                sequence[position] = oBuffer[j];
                position++;
            }
            
            free(oBuffer);
        }
        
        // Check if the final sequence is indeed sorted
        int success = 1;
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
        
        // Write the result to a file
        FILE *myFile;
        myFile = fopen("output_parallel.txt", "w");
        for(i = 0; i < inputSize; i++)
        {
            fprintf(myFile, "%d ", sequence[i]);
        }
        fclose(myFile);
        
        // Stop the clock and record the execution time
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        printf("\nProgram took %10.8f seconds\n",time_spent);
    }
    else
    {
        // Non-master process will send the number of inputs they have to the master
        MPI_Send(&currentBufferSize, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        
        // If they have inputs, they will send their sequence to the master
        if(currentBufferSize != 0)
        {
            MPI_Send(currentBuffer, currentBufferSize, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        }        
    }
    
    free(sequence);
    
    MPI_Finalize();
}


