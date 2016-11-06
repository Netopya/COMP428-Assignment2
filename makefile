CC=mpicc

assignment2:
	$(CC) parallel_quicksort.c -o parallel_quicksort
	gcc serial_quicksort.c -o serial_quicksort
	gcc number_generator.c -o number_generator
clean:
	rm parallel_quicksort
	rm serial_quicksort
	rm number_generator