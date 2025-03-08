// columnsort routine you must implement
// parameters:
//   first parameter is a pointer to a one-dimensional array; after executing this array is sorted
//   second parameter is number of threads (1 if sequential)
//   third and fourth parameters are r and s, respectively, from the columnsort algorithm
//   fifth parameter is the address of a double into which this routine must write the elapsed time
void columnSort(int *A, int numThreads, int length, int width,
		double *elapsedTime);
