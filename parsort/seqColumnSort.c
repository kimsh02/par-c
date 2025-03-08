#include <stdio.h>
#include <stdlib.h>
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

void qsortRows(int *A, int len, int size)
{
	/* Call qsort on the rows of A */
	for (int i = 0; i < size; i += len) {
		qsort(A + i, len, sizeof(int), intCompar);
	}
}

void transpose(int *original, int *transposed, int trows, int tcols, int size)
{
	/* Initialize indices for transposed matrix */
	int rowCtr = 0;
	int rowIdx = 0;
	int colIdx = 0;
	/* For every element in original, place it in the correct position in
	   transposed */
	for (int i = 0; i < size; i++) {
		if (rowCtr == trows) {
			rowCtr = 0;
			rowIdx = 0;
			colIdx++;
		}
		*(transposed + rowIdx + colIdx) = *(original + i);
		rowCtr++;
		rowIdx += tcols;
	}
}

void shiftRight(int *original, int *shifted, int size, int shift)
{
	int half = shift / 2;
	/* Right shift original by half into shifted */
	for (int i = 0; i < size + shift; i++) {
		if (i < half) {
			*(shifted + i) = INT16_MIN; /* -inf */
		} else if (i >= size + half) {
			*(shifted + i) = INT16_MAX; /* inf */
		} else {
			*(shifted + i) = *(original + i - half);
		}
	}
}

void shiftLeft(int *original, int *shifted, int size, int shift)
{
	int half = shift / 2;
	/* Left shift original by half into shifted */
	for (int i = half; i < size + half; i++) {
		*(shifted + i - half) = *(original + i);
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

void columnSort(int *A, int numThreads, int r, int s, double *elapsed)
{
	/* Doing row sort instead of column sort */
	/* Treating 1D array like a 2D matrix by indexing into the 1D array as
	   if it were a 2D matrix */

	/* Initialize size of A */
	int size = r * s;
	/* Initialize temporary matrix for transpose and shift operations */
	/* Note that (s + 1) is to support shifted matrix */
	int *tmp = malloc((r) * (s + 1) * sizeof(int));

	struct timeval start, end;
	gettimeofday(&start, NULL); /* Start timer */

	/* Step 1: qsort rows of A */
	qsortRows(A, r, size);

	/* Step 2: transpose A into tmp */
	transpose(A, tmp, s, r, size);
	/* No reshaping is necessary because we simply index into the 1D array
	   as if it were reshaped accordingly. */

	/* Step 3: qsort rows of tmp */
	qsortRows(tmp, r, size);

	/* Step 4: transpose tmp into A */
	transpose(tmp, A, r, s, size);

	/* Step 5: qsort rows of A */
	qsortRows(A, r, size);

	/* Step 6: shift A right by s/2 into tmp */
	/* Since we are doing row sort, we do shift right instead of shift
	   down */
	shiftRight(A, tmp, size, r);

	/* Step 7: qsort rows of tmp */
	qsortRows(tmp, r, size + r);

	/* Step 8: shift tmp left by s/2 into A */
	shiftLeft(tmp, A, size, r);

	gettimeofday(&end, NULL); /* Stop timer */
	*elapsed = elapsedTime(&start, &end);

	free(tmp);
}
