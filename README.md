# COMP428-Assignment2
Parallelized Quicksort using OpenMPI

To create all the executable files for this assignment, run the command "make assignment2". The files can be removed by calling "make clean"

First run "number_generator" to generate an "input.txt" file with random numbers to be sorted. This file will be used by the following programs for sorting.
The number of values can be specified as an argument to the program. For example the command "./number_generator 1000000" will create an "input.txt" file with 1 million numbers.

The "serial_quicksort" program can be executed with the "./serial_quicksort" command to sort the input file sequencially. The result will be written to "output.txt".

The "parallel_quicksort" program can be executed with the "mpirun -np X parallel_quicksort" command where X is the number of processes to run the program on. Note that this value must be a power of 2 in order to maintain the topology of a hypercube. The result will be written to "output.txt".

For both "serial_quicksort" and "parallel_quicksort" can have the number of input values limited by specifying it as a command line argument. For example the command "mpirun -np 4 parallel_quicksort 100" will only sort the first 100 values of "input.txt" over 4 processors.
If you supply your own "input.txt" file or modify an existing one, note that the "parallel_quicksort" does not support sorting negative values.
