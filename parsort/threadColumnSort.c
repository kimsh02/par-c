#include <math.h>
#include <pthread.h>
/* #include <stdint.h> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "columnSort.h"

double elapsedTime(struct timeval *start, struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) +
	       (end->tv_usec - start->tv_usec) * 1e-6;
}

int intCompar(const void *p1, const void *p2)
{
	return *(int *)p1 - *(int *)p2;
}

void qsortRow(int *row, int r)
{
	/* Call qsort on the row of A that thread i handles */
	qsort(row, r, sizeof(int), intCompar);
}

void transpose(int *row, int *transposed, int i, int numThreads, int r, int len)
{
	int rowIdx = 0;
	int rowCtr = 0;
	int colIdx = i * len / numThreads;
	for (int j = 0; j < len; j++) {
		if (rowCtr == numThreads) {
			rowCtr = 0;
			rowIdx = 0;
			colIdx++;
		}
		*(transposed + colIdx + rowIdx) = *(row + j);
		rowCtr++;
		rowIdx += r;
	}
}

void shiftRight(int *original, int *shifted, int r)
{
	int half = r / 2;
	/* Right shift original by half into shifted */
	for (int i = 0; i < r; i++) {
		*(shifted + i + half) = *(original + i);
	}
}

void shiftLeft(int *original, int *shifted, int r)
{
	int half = r / 2;
	/* Left shift original by half into shifted */
	for (int i = 0; i < r; i++) {
		*(shifted + i) = *(original + i + half);
	}
}

/* For debugging */
void printMatrix(int *mat, int rows, int cols)
{
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int val = *(mat + i * cols + j);
			switch (val) {
			case INT16_MIN:
				printf("-inf\t");
				break;
			case INT16_MAX:
				printf("inf\t");
				break;
			default:
				printf("%d\t", *(mat + i * cols + j));
			}
		}
		printf("\n");
	}
	printf("\n");
}

/* Dissemination barrier */
void barrier(volatile int *arrive, int numThreads, int i)
{
	for (int j = 1; j <= ceil(log2(numThreads)); j++) {
		while (arrive[i] != 0) {};
		arrive[i]   = j;
		int waitFor = ((i + (1 << (j - 1))) % numThreads);
		while (arrive[waitFor] != j) {};
		arrive[waitFor] = 0;
	}
}

typedef struct {
	int *A;
	int *row;
	int *tmp;
	int  offset;
	int  r;
	int  i;
	int  numThreads;

	volatile int *arrive;
} threadData;

void *columnSortHelper(void *args)
{
	threadData *td = (threadData *)args;
	/* Step 1: qsort row of A */
	qsortRow(td->row, td->r);
	/* Wait for other threads */
	barrier(td->arrive, td->numThreads, td->i);

	/* Step 2: transpose row of A into column of tmp */
	transpose(td->row,
		  td->tmp,
		  td->i,
		  td->numThreads,
		  td->r,
		  td->r); /* No reshaping is necessary because we simply index
	   into the 1D array as if it were reshaped accordingly. */
	/* Wait for other threads */
	barrier(td->arrive, td->numThreads, td->i);

	/* Step 3: qsort row of tmp */
	qsortRow(td->tmp + td->offset, td->r);
	/* Wait for other threads */
	barrier(td->arrive, td->numThreads, td->i);

	/* Step 4: tranpose row of tmp into column of A */
	transpose(td->tmp + td->offset,
		  td->A,
		  td->i,
		  td->r,
		  td->numThreads,
		  td->r);
	/* Wait for other threads */
	barrier(td->arrive, td->numThreads, td->i);

	/* Step 5: qsort row of A */
	qsortRow(td->row, td->r);
	/* Wait for other threads */
	barrier(td->arrive, td->numThreads, td->i);

	/* Step 6: shift A right by r/2 into tmp */
	/* Since we are doing row sort, we do shift right instead of shift
	   down */
	shiftRight(td->row, td->tmp + td->offset, td->r);
	/* Wait for other threads */
	barrier(td->arrive, td->numThreads, td->i);

	/* Step 7: qsort row of tmp */
	/* In particular, the first thread corresponding to the first row does
	   not sort its row while the other threads should sort their rows */
	if (td->i != 0) {
		qsortRow(td->tmp + td->offset, td->r);
	}
	/* Wait for other threads */
	barrier(td->arrive, td->numThreads, td->i);

	/* Step 8: shift tmp left by r/2 into A */
	shiftLeft(td->tmp + td->offset, td->row, td->r);

	/* /\* Wait for other threads *\/ */
	/* barrier(td->arrive, td->numThreads, td->i); */

	return NULL;
}

void columnSort(int *A, int numThreads, int r, int s, double *elapsed)
{
	/* Doing row sort instead of column sort */
	/* Treating 1D array like a 2D matrix by indexing into the 1D array as
	   if it were a 2D matrix */

	/* Initialize size of A */
	int size = r * s;
	/* Resize length of rows based on number of threads */
	r = size /
	    numThreads; /* Matrix is now reshaped to numThread rows and r columns */
	/* Initialize temporary matrix for transpose and shift operations */
	/* Note that (s + 1) is to support shifted matrix */
	int *tmp = malloc((size_t)r * (numThreads + 1) * sizeof(int));

	/* Dissemination barrier */
	volatile int arrive[numThreads];
	/* Initialize elements to all zeroes */
	memset((void *)arrive, 0, numThreads * sizeof(*arrive));
	/* Create numThreads threads */
	pthread_t threads[numThreads];
	/* Create numThreads threadData */
	threadData data[numThreads];

	struct timeval start, end;
	gettimeofday(&start, NULL); /* Start timer */

	/* Assign task for each thread which is responsible for each row */
	int offset = 0;
	for (int i = 0; i < numThreads; i++) {
		(data + i)->A	       = A;
		(data + i)->row	       = A + offset;
		(data + i)->tmp	       = tmp;
		(data + i)->r	       = r;
		(data + i)->offset     = offset;
		(data + i)->i	       = i;
		(data + i)->arrive     = arrive;
		(data + i)->numThreads = numThreads;
		offset += r;
		pthread_create(threads + i, NULL, columnSortHelper, data + i);
	}

	/* Join back the threads */
	for (int i = 0; i < numThreads; i++) {
		pthread_join(*(threads + i), NULL);
	}

	/* printMatrix(A, numThreads, r); */

	gettimeofday(&end, NULL); /* Stop timer */
	*elapsed = elapsedTime(&start, &end);

	free(tmp);
	return;
}
