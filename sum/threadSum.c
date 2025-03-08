#include <stdlib.h>

#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"

void printSum(long long sum)
{
	int  buf_sz = 50;
	char buf[buf_sz];
	snprintf(buf, buf_sz, "%lld\n", sum);
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

typedef struct {
	long long *elems;
	long long *partials;
	long	   sz;
} threadData;

void *threadWork(void *arg)
{
	/* printf("Thread spawned.\n"); */
	threadData *td = (threadData *)arg;
	for (long i = 0; i < td->sz; i++) {
		*td->partials += *(td->elems + i);
	}
	return NULL;
}

long long sumOfElems(long long *elems, long elems_sz, long num_threads,
		     long *elapsed)
{
	struct timeval start, end;
	gettimeofday(&start, NULL); /* Start timer */

	long long  partials[num_threads];
	pthread_t  threads[num_threads];
	threadData td[num_threads];
	long	   partials_sz = elems_sz / num_threads;

	// Assign task for each thread
	long pos = 0;
	for (long i = 0; i < num_threads; i++) {
		*(partials + i)	   = 0;
		(td + i)->elems	   = elems + pos;
		(td + i)->partials = partials + i;
		if (i == num_threads - 1) {
			(td + i)->sz = elems_sz - pos;
		} else {
			(td + i)->sz = partials_sz;
		}
		pos += partials_sz;
		pthread_create(threads + i, NULL, threadWork, td + i);
	}

	// Join back each thread and compute final sum
	long long sum = 0;
	for (long i = 0; i < num_threads; i++) {
		pthread_join(*(threads + i), NULL);
		sum += *(partials + i);
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
	long	   elems_sz = strtol(argv[argc - 2], &tmp, 10);
	long long *elems    = malloc(elems_sz * sizeof(long long));
	// Initialize number of threads
	long num_threads = strtol(argv[argc - 1], &tmp, 10);

	// Populate elems array
	populateElems(elems, elems_sz);

	// Compute sum of elems and elapsed time
	long	  elapsed;
	long long sum = sumOfElems(elems, elems_sz, num_threads, &elapsed);

	// Print final sum
	printSum(sum);

	// Print elapsed time
	/* printElapsed(elapsed); */

	free(elems);

	return 0;
}
