/*
    Julie Muzina
    2713300
    jumuzina
    CIS 545 - Project 1 - thr_reduce.c
*/

// -----Dependencies-----
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h> // Please make sure you use -lm option (GCC) to link the math library
#include <assert.h>
// ----------------------

/** Number of args main() expects */
const int NUM_ARGS = 3;
/** Number that all integer arguments should be a power of*/
const int ARGS_POWER_OF = 2;
/** Shared total of the quadruple root calculation. */
double * partialSums;

/** Arguments provided to threads in the `calculateRoot` function */
struct RootCalculationThreadArgs {
    /** Index of the thread */
    int threadNum;
    /** How many threads the application will use to calculate the root summation */
    int numThreads;
    /** Maximum number to consider in root consideration */
    int upperBound;
};

/** Prints generic usage hint */
void printUsageHint() {
    printf("Correct usage:\t./thr_reduce.c <number of threads> <upper bound> where <number of threads> and <upper bound> are integer powers of 2, and <number of threads> <= <upper bound>.\n");
    printf("Ex:\t./thr_reduce.c 8 64\n");
}

/** 
 * @param radicand Number to check against base
 * @param base base / nth root.
 * @returns whether `radicand` is a power of `base`
*/
bool isPowerOf(int radicand, int base) {
    if (radicand <= 0) return false;

    while (radicand % base == 0) radicand /= base;
    return radicand == 1;
}

void * calculateRoot(void *_args) {
    // Args pre-processing
    struct RootCalculationThreadArgs *args = (struct RootCalculationThreadArgs *) _args;

    // Define bounds of the computation
    const int lowerLoopBound = ((args->threadNum) * (args->upperBound / args->numThreads)) + 1;
    const int upperLoopBound = ((args->threadNum + 1) * (args->upperBound / args->numThreads));

    assert(upperLoopBound >= lowerLoopBound);

    /** Running sum of quadruple roots from lowerLoopBound to upperLoopBound */
    double threadTotal = 0;

    // Calculate the quadruple roots and accumulate resulting sum in `threadTotal`.
    for (int i = lowerLoopBound; i <= upperLoopBound; ++i) {
        threadTotal += pow(i, 1.0/4);
    }
    
    // store our partial total in the partialSums arr.
    partialSums[args->threadNum] = threadTotal;

    // Thread cleanup
    free(args);
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char ** argv) {
    // Input validation and pre-processing
    if (argc != NUM_ARGS) {
        printf("Invalid number of arguments received; expected %d, received %d\n", NUM_ARGS, argc);
        printUsageHint();
        exit(1);
    }

    int numThreads = atoi(argv[1]);
    const int upperBound = atoi(argv[2]);

    if (numThreads <= 0) {
        printf("Must use at least one thread.\n");
        printUsageHint();
        exit(1);
    }

    const bool numThreadsValidPow = isPowerOf(numThreads, ARGS_POWER_OF);
    const bool upperBoundValidPow = isPowerOf(upperBound, ARGS_POWER_OF);
    
    if (!numThreadsValidPow || !upperBoundValidPow) {
        printf("Invalid integer argument. Integer arguments must be a power of %d.\n", ARGS_POWER_OF);
        if (!numThreadsValidPow) printf("numThreads(%d) is not a power of %d.\n", numThreads, ARGS_POWER_OF);
        if (!upperBoundValidPow) printf("upperBound(%d) is not a power of %d.\n", upperBound, ARGS_POWER_OF);
        printUsageHint();
        exit(1);
    }

    // If there are more threads than there are integers 
    if (numThreads > upperBound) {
        printf("Number of threads must be less than or equal to the upper bound.\n");
        printUsageHint();
        exit(1);
    }

    partialSums = malloc(numThreads * sizeof(double));

    /** Array of thread identifiers */
    pthread_t * threadIds = malloc(numThreads * sizeof(pthread_t));
    
    for (int threadNum = 0; threadNum < numThreads; ++threadNum) {
        pthread_t threadId;
        struct RootCalculationThreadArgs* args = malloc(sizeof(struct RootCalculationThreadArgs));
        args->threadNum = threadNum;
        args->numThreads = numThreads;
        args->upperBound = upperBound;
        const int err = pthread_create(&threadId, NULL, &calculateRoot, args);
        if (err) {
            printf("Failed to create thread %d", threadNum);
            exit(1);
        } else {
            threadIds[threadNum] = threadId;
        }
    }

    /** The number of times the partial sums array has been reduced in half */
    int numReductions = 0;
    
    // Loop over `partialSums`, changing it in-place by adding two elements and reducing its size in half each time.
    while (numThreads > 1) {
        // Loop over the remainder of `partialSums`. 
        // We start by picking a thread at the start of the array and one at the middle, and work our way to the end.
        for (int threadNum = 0; threadNum < numThreads / 2; ++threadNum) {
            const int partnerThreadNum = (numThreads / 2) + threadNum;
            // Only wait for the threads to finish if this is the first reduction. 
            // After the first reduction, all threads have completed. 
            if (!numReductions) {
                pthread_join(threadIds[threadNum], NULL);
                pthread_join(threadIds[partnerThreadNum], NULL);
            }
            // Store the sum of these two elements in `partialSums`, overwriting the previous value.
            partialSums[threadNum] = partialSums[threadNum] + partialSums[partnerThreadNum];
        }
        ++numReductions;
        numThreads /= 2;
        // Shrink all thread-related heap data accordingly
        if (numThreads > 0) {
            partialSums = realloc(partialSums, numThreads * sizeof(double));
            threadIds = realloc(threadIds, numThreads * sizeof(pthread_t));
        }
    }
    
    // At the end, there is only one element in the array, and it is the total.
    const double total = partialSums[0];

    // All worker threads have added their partial sums to `total`. Print the result!
    printf("The sum of fourth roots from %d to %d is %f.\n", 1, upperBound, total);

    free(partialSums);
    free(threadIds);
}