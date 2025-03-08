#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"

void printSum(long long sum)
{
	int  buf_sz = 50;
	char buf[buf_sz];
	snprintf(buf, sizeof(buf), "%lld\n", sum);
	printf("%s", buf);
}

void populateElems(long long *elems, long elems_sz)
{
	for (long i = 0; i < elems_sz; i++) {
		*(elems + i) = i;
	}
}

long elapsedTime(struct timeval *start, struct timeval *end)
{
	return (end->tv_sec - start->tv_sec) * 1e6 +
	       (end->tv_usec - start->tv_usec);
}

long long sumOfElems(long long *elems, long elems_sz, long *elapsed)
{
	struct timeval start, end;
	gettimeofday(&start, NULL); /* Start timer */

	long long sum = 0;
	for (long i = 0; i < elems_sz; i++) {
		sum += *(elems + i);
	}

	gettimeofday(&end, NULL); /* Stop timer */
	*elapsed = elapsedTime(&start, &end);

	return sum;
}

void printElapsed(long elapsed)
{
	int  buf_sz = 50;
	char buf[buf_sz];
	snprintf(buf, buf_sz, "%ld microseconds\n", elapsed);
	printf("%s\n", buf);
}

int main(int argc, char **argv)
{
	// Declare elems array
	char	  *tmp;
	long	   elems_sz = strtol(argv[argc - 1], &tmp, 10);
	long long *elems    = malloc(elems_sz * sizeof(long long));

	// Populate elems array
	populateElems(elems, elems_sz);

	// Compute sum of elems and elapsed time
	long	  elapsed;
	long long sum = sumOfElems(elems, elems_sz, &elapsed);

	// Print final sum
	printSum(sum);

	//Print elapsed time
	/* printElapsed(elapsed); */

	free(elems);

	return 0;
}
