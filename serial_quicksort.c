#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int main (int argc, char *argv[])
{
    
    clock_t begin = clock();
        
    int* sequence;
    int inputSize;
    
    FILE *myFile;
    myFile = fopen("input.txt", "r");
    
    
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
    
    //int input[size];
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

    qsort(sequence, inputSize, sizeof(int), compare);
    
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
    
    myFile = fopen("output_serial.txt", "w");
    for(i = 0; i < inputSize; i++)
    {
        fprintf(myFile, "%d ", sequence[i]);
    }
    fclose(myFile);
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
    printf("\nProgram took %10.8f seconds\n",time_spent);
}