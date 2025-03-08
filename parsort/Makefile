seqsort:	seqColumnSort.o driverColumnSort.o
	gcc -o seqsort seqColumnSort.o driverColumnSort.o -lm -lpthread

parsort:	threadColumnSort.o driverColumnSort.o
	gcc -o parsort threadColumnSort.o driverColumnSort.o -lm -lpthread

driverColumnSort.o:	driverColumnSort.c columnSort.h
	gcc -c -O2 -std=c99 driverColumnSort.c

seqColumnSort.o:	seqColumnSort.c
	gcc -c -O2 -std=c99 seqColumnSort.c

threadColumnSort.o:	threadColumnSort.c
	gcc -c -O2 -std=c99 threadColumnSort.c

clean:
	rm -f *.o seqsort parsort
