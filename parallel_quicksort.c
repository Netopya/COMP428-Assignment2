#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void printBuffer(int buffer[], int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        printf(" %d", buffer[i]);
    }
    printf("\n");
}


int IsPowerOfTwo(int x)
{
    return (x & (x - 1)) == 0;
}

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
    
    unsigned seed =  (unsigned)(time(0) + taskid);
    srand(seed);
    //printf("x Random seed: %d\n", seed);
    
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

        /*
        printf("Input sequence: ");
        
        for (j = 0; j < inputSize; j++)
        {
            printf ("%d ", sequence[j]);
        }
        
        printf("\n");
        */
    }

    int scounts[numtasks];
    int displs[numtasks];
    
    displs[0] = 0;
    
    MPI_Bcast(&inputSize, sizeof(inputSize), MPI_INT, 0, MPI_COMM_WORLD);
    
    if(taskid != 0)
    {
        sequence = malloc(inputSize * sizeof(int));
    }
    
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
    
    
    MPI_Bcast(sequence, inputSize, MPI_INT, 0, MPI_COMM_WORLD);

    int currentBufferSize = scounts[taskid];
    int *currentBuffer;
    currentBuffer = malloc(currentBufferSize * sizeof(int));
    
    for(i = 0; i < scounts[taskid]; i++)
    {
        currentBuffer[i] = sequence[i + displs[taskid]];
    }
    
    MPI_Comm comm = MPI_COMM_WORLD;
    
    
    /*printf("Process %d sees:", taskid);
    for(i = 0; i < currentBufferSize; i++)
    {
        printf(" %d", currentBuffer[i]);
    }
    printf("\n");*/
    
    for(i = 0; i < numRounds; i++)
    {        
        
        int j;
        int pivot;
        int groupRank, groupSize;
        MPI_Comm_rank(comm, &groupRank);
        MPI_Comm_size(comm, &groupSize);
        
        if(groupRank == 0)
        {   
            if(currentBufferSize != 0)
            {
                int r = rand() % currentBufferSize;            
                pivot = currentBuffer[r];
                
                //printf("%d1 Setting pivot to %d\n", i, pivot);
            }
            else
            {
                pivot = -1;
            }
        }
        
        MPI_Bcast(&pivot, 1, MPI_INT, 0, comm);
        
        //printf("%d20 On round %d Process %d got a pivot of %d\n", i, i, taskid, pivot);
        
        if(pivot < 0)
        {
            //printf("%d2a On round %d Process %d will find its own pivot\n", i, i, taskid);
            
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
            
            //printf("%d2a On round %d Process %d will suggest pivot %d with size %d\n", i, i, taskid, pivot, groupSize);
                        
            MPI_Gather(&pivot, 1, MPI_INT, pivots, 1, MPI_INT, 0, comm);
            
            pivot = -1;
            
            if(groupRank == 0)
            {
                //printf("%d2b On round %d Process %d gathered the pivots: ", i, i, taskid);
                //printBuffer(pivots, groupSize);
                
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
            
            MPI_Bcast(&pivot, sizeof(int), MPI_INT, 0, comm);
            
            if(pivot < 0)
            {
                //printf("%d2c On round %d Process %d failed to find an alternate pivot\n", i, i, taskid);
                MPI_Comm_split(MPI_COMM_WORLD, taskid & ((numtasks - 1) << (numRounds - 1 - i)), taskid, &comm);
                continue;
            }
            //printf("%d2d On round %d Process %d got an alternate pivot of %d\n", i, i, taskid, pivot);
        }
        

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
        
        //printf("Process %d got a maxcount of %d and a mincount of %d\n", taskid, maxcount, mincount);
        
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
        
        /*printf("%d3 On Round %d Process %d Maxes are: ", i, i, taskid);
        for(j = 0; j < maxcount; j++)
        {
            printf(" %d", max[j]);
        }
        printf(" and Mins are: ");
        for(j = 0; j <mincount; j++)
        {
            printf(" %d", min[j]);
        }
        printf("\n");*/
        
        int toggleBit = 1 << (numRounds - i - 1);
        int partner = taskid ^ toggleBit;
        
        //printf("Process %d will trade with %d with toggle %d\n", groupRank, partner, toggleBit);
        
        int pMinSize = 0;
        int pMaxSize = 0;
        
        //printf("%d4 On Round %d Process %d (rank %d) will trade with %d\n", i, i, taskid, groupRank, partner);
        
        //printf("%d40 Process %d Comm: %d\n", i, taskid, comm);
        
        if(taskid > partner)
        {
            
            MPI_Send(&mincount, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            MPI_Send(&min, mincount, MPI_INT, partner, 0, MPI_COMM_WORLD);
            
            MPI_Recv(&pMaxSize, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
           // printf("%d41 Process %d received a max size of %d\n", i, taskid, pMaxSize);
            
            int pMaxBuffer[pMaxSize];
            MPI_Recv(&pMaxBuffer, pMaxSize, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            //printf("%d42 Process %d received a max buffer of ", i, taskid);
            //printBuffer(pMaxBuffer, pMaxSize);
            
            free(currentBuffer);
            currentBufferSize = maxcount + pMaxSize;
            currentBuffer = malloc(sizeof(int) * (currentBufferSize));
            
            for(j = 0; j < maxcount; j++)
            {
                currentBuffer[j] = max[j];
            }
            
            for(j = 0; j < pMaxSize; j++)
            {
                currentBuffer[maxcount + j] = pMaxBuffer[j];
            }
        }
        else
        {
            MPI_Recv(&pMinSize, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            //printf("%d41 Process %d received a min size of %d\n", i, taskid, pMinSize);
            
            int pMinBuffer[pMinSize];
            MPI_Recv(&pMinBuffer, pMinSize, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            //printf("%d42 Process %d received a min buffer of ", i, taskid);
            //printBuffer(pMinBuffer, pMinSize);
            
            MPI_Send(&maxcount, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
            MPI_Send(&max, maxcount, MPI_INT, partner, 0, MPI_COMM_WORLD);
            
            free(currentBuffer);
            currentBufferSize = mincount + pMinSize;
            currentBuffer = malloc(sizeof(int) * (currentBufferSize));
            
            for(j = 0; j < mincount; j++)
            {
                currentBuffer[j] = min[j];
            }
            
            for(j = 0; j < pMinSize; j++)
            {
                currentBuffer[mincount + j] = pMinBuffer[j];
            }
        }
        
        /*printf("%d5 On round %d Process %d sees:", i, i, taskid);
        for(j = 0; j < currentBufferSize; j++)
        {
            printf(" %d", currentBuffer[j]);
        }
        printf("\n");*/
    
        MPI_Comm_split(MPI_COMM_WORLD, taskid & ((numtasks - 1) << (numRounds - 1 - i)), taskid, &comm);
        
        /*
        int row_rank, row_size;
        MPI_Comm_rank(comm, &row_rank);
        MPI_Comm_size(comm, &row_size);
        
        printf("Process %d on round %d has rank %d in a size of %d\n", taskid, i, row_rank, row_size);*/
    }
    
    printf("x Final contents of Process %d", taskid);
    printBuffer(currentBuffer, currentBufferSize);
    
    MPI_Finalize();
}


