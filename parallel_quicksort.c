#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main (int argc, char *argv[])
{

int	taskid,	        /* task ID - also used as seed number */
	numtasks,       /* number of tasks */
	rc,             /* return code */
	i;


    MPI_Status status;
    
    /* Obtain number of tasks and task ID */
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);

    int numRounds = log2(numtasks);
    int* sequence;
    int inputSize;
    
    srand(time(NULL));
    
    if(taskid == 0)
    {
        printf ("%d rounds will execute \n", numRounds);
    
        if(!IsPowerOfTwo(numtasks))
        {
            printf ("The number of processes must be a power of 2\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return rc;
        }
        
        FILE *myFile;
        myFile = fopen("somenumbers.txt", "r");
        
        
        fscanf(myFile, "%d", &inputSize);
        
        //int input[size];
        sequence = malloc(inputSize * sizeof(int));
        int j;        
        for (j = 0; j < inputSize; j++)
        {
            fscanf(myFile, "%d", &sequence[j]);
        }
        
        fclose(myFile);

        printf("Input sequence: ");
        
        for (j = 0; j < inputSize; j++)
        {
            printf ("%d ", sequence[j]);
        }
        
        printf("\n");
        
    }

    int scounts[numtasks];
    int displs[numtasks];
    
    displs[0] = 0;
    
    MPI_Bcast(&inputSize, sizeof(inputSize), MPI_INT, 0, MPI_COMM_WORLD);
    
    //printf("Process %d knows input is %d\n", taskid, inputSize);
    
    if(taskid != 0)
    {
        sequence = malloc(inputSize * sizeof(int));
    }
    
    int baseCount = inputSize / numtasks;
    int remainder = inputSize % numtasks;
    
    //printf("Process %d found a base count of %d and a remainder of %d\n", taskid, baseCount, remainder);
    
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
    
    //printf("Process %d count is %d with an offset of %d\n", taskid, scounts[taskid], displs[taskid]);
    
    MPI_Bcast(sequence, inputSize, MPI_INT, 0, MPI_COMM_WORLD);
    /*
    printf("Process %d sees:", taskid);
    for(i = 0; i < inputSize; i++)
    {
        printf(" %d", sequence[i]);
    }
    printf("\n");
    */
    int rec_buf[scounts[taskid]];
    for(i = 0; i < scounts[taskid]; i++)
    {
        rec_buf[i] = sequence[i + displs[taskid]];
    }
/*
    printf("Process %d received:", taskid);
    for(i = 0; i < scounts[taskid]; i++)
    {
        printf(" %d", rec_buf[i]);
    }
    printf("\n");
    */
    //printf("Process check %d", taskid);
    
    MPI_Comm comm = MPI_COMM_WORLD;
    int currentBufferSize = scounts[taskid];
    int *currentBuffer;
    currentBuffer = malloc(currentBufferSize * sizeof(int));
    
    
    printf("Process %d sees:", taskid);
    for(i = 0; i < currentBufferSize; i++)
    {
        currentBuffer[i] = rec_buf[i];
        printf(" %d", currentBuffer[i]);
    }
    printf("\n");
    
    
    for(i = 0; i < numRounds; i++)
    {
        int toggleBit = 1 << (numRounds - i);
        int partner = taskid ^ toggleBit;
        
        int pivot;
        int groupRank;
        MPI_Comm_rank(comm, &groupRank);
        
        if(groupRank == 0)
        {
            //printf("For round %d Process %d is a master based on %d\n", i, taskid, masterBit | taskid);
            
            int r = rand() % currentBufferSize;            
            pivot = rec_buf[r];
            
            printf("Setting pivot to %d\n", pivot);
        }
        
        MPI_Bcast(&pivot, sizeof(int), MPI_INT, 0, comm);
        printf("On round %d Process %d got a pivot of %d\n", i, taskid, pivot);
        
        int j;
        int maxcount = 0;
        int mincount = 0;
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
        
        printf("Process %d got a maxcount of %d and a mincount of %d\n", taskid, maxcount, mincount);
        
        int max[maxcount];
        int min[mincount];
        
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
        
        printf("    Process %d Maxes are: ", taskid);
        for(j = 0; j < maxcount; j++)
        {
            printf(" %d", max[j]);
        }
        printf(" and Mins are: ");
        for(j = 0; j <mincount; j++)
        {
            printf(" %d", min[j]);
        }
        printf("\n");
        
        
        MPI_Comm_split(MPI_COMM_WORLD, taskid & ((numtasks - 1) << (numRounds - 1 - i)), taskid, &comm);
        
        /*
        int row_rank, row_size;
        MPI_Comm_rank(comm, &row_rank);
        MPI_Comm_size(comm, &row_size);
        
        printf("Process %d on round %d has rank %d in a size of %d\n", taskid, i, row_rank, row_size);*/
        
        break;
    }
    
    MPI_Finalize();
}

int IsPowerOfTwo(int x)
{
    return (x & (x - 1)) == 0;
}