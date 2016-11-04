#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUMBER_OF_NUMBERS 10000000
#define MAX_NUMBER 1000000

int main (int argc, char *argv[])
{
    unsigned seed =  (unsigned)(time(0));
    srand(seed);
    
    FILE *myFile;
    myFile = fopen("input.txt", "w");
       
    int i;
    for(i = 0; i < NUMBER_OF_NUMBERS - 1; i++)
    {
        fprintf(myFile, "%d ", rand() % MAX_NUMBER);
    }
    
    fprintf(myFile, "%d", rand() % MAX_NUMBER);
    
    fclose(myFile);
}