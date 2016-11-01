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
    
    int baseCount = inputSize / numtasks;
    int remainder = inputSize % numtasks;
    
    for(i = 1; i < numtasks; i++)
    {
        scounts[0] = baseCount;
        
        if(i < remainder)
        {
            scounts[0]++;
        }
        
        if(i != 0)
        {
            displs[i] = displs[i - 1] + scounts[i - 1];
        }
    }
    
    int rec_buf[scounts[taskid]];
    MPI_SCATTERV(sequence, &scounts, &displs, MPI_INT, &rec_buf, scounts[taskid], MPI_INT, 0, MPI_COMM_WORLD);

    printf("Process %d received:", taskid);
    for(i = 0; i < scounts[taskid]; i++)
    {
        printf(" %d", rec_buf[i]);
    }
    printf("\n");
    
    
    for(i = 0; i < numRounds; i++)
    {
        
    }
    
    MPI_Finalize();
}

int IsPowerOfTwo(int x)
{
    return (x & (x - 1)) == 0;
}