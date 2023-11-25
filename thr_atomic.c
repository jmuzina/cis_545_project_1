/*
    Julie Muzina
    2713300
    jumuzina
    CIS 545 - Project 1 - thr_atomic.c
*/

// -----Dependencies-----
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h> // Please make sure you use -lm option (GCC) to link the math library
#include <semaphore.h> 
#include <assert.h>
// ----------------------

/** Number of args main() expects */
const int NUM_ARGS = 3;
/** Number that all integer arguments should be a power of*/
const int ARGS_POWER_OF = 2;
/** Shared total of the quadruple root calculation. */
double total = 0;
// Semaphores for mutual exclusion
/** Worker semaphore; main thread waits for this semaphore to be 1 to print final result */
sem_t worker;
/** Dispatcher semaphore; worker threads wait for this semaphore to be 1 to enter their critical section */
sem_t dispatcher;

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
    printf("Correct usage:\t./thr_atomic.c <number of threads> <upper bound> where <number of threads> and <upper bound> are integer powers of 2, and <number of threads> <= <upper bound>.\n");
    printf("Ex:\t./thr_atomic.c 8 64\n");
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

    printf("Thread %d finished calculating %d quadruple roots from %d to %d with a sum of %f.\n", args->threadNum, upperLoopBound - lowerLoopBound + 1, lowerLoopBound, upperLoopBound, threadTotal);
    
    // Wait until no other worker threads are in their critical section
    sem_wait(&dispatcher);
    // Enter the critical section; atomically update the total.
    total += threadTotal;
    // Inform the main thread that this worker has left its critical section
    sem_post(&worker);
    // End critical section

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

    const int numThreads = atoi(argv[1]);
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
    
    // Prepare semaphores to ensure mutex
    sem_init(&dispatcher, 0, 1);
    sem_init(&worker, 0, 1);

    for (int threadNum = 0; threadNum < numThreads; ++threadNum) {
        pthread_t threadId;
        struct RootCalculationThreadArgs* args = malloc(sizeof (struct RootCalculationThreadArgs));
        args->threadNum = threadNum;
        args->numThreads = numThreads;
        args->upperBound = upperBound;
        const int err = pthread_create(&threadId, NULL, &calculateRoot, args);
        if (err) {
            printf("Failed to create thread %d", threadNum);
            exit(1);
        }
    }

    /** Signal worker threads to begin adding their partial sums to the shared total */
    for (int threadNum = 0; threadNum < numThreads; ++threadNum) {
        sem_post(&dispatcher);
    }

    /** 
     * Wait for all worker threads to be finished adding their partial sums to the shared total 
     * We wait one extra time to allow the last thread to finish.
     * */
    for (int threadNum = 0; threadNum <= numThreads; ++threadNum) {
        sem_wait(&worker);
    }

    // Semaphore cleanup
    sem_destroy(&dispatcher);
    sem_destroy(&worker);

    // All worker threads have added their partial sums to `total`. Print the result!
    printf("The sum of fourth roots from %d to %d is %f.\n", 1, upperBound, total);
}