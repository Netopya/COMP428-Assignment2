CC=mpicc

parallel_quicksort: parallel_quicksort.c
	$(CC) parallel_quicksort.c -o parallel_quicksort

clean:
	rm parallel_quicksort