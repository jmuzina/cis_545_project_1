/*
    Julie Muzina
    2713300
    jumuzina
    CIS 545 - Project 1 - thr_atomic.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <semaphore.h> 
#include <assert.h>

const int NUM_ARGS = 3;
const int ARGS_POWER_OF = 2;
double total = 0;

sem_t semaphore;

void printUsageHint() {
    printf("Correct usage:\t./thr_atomic.c <number of threads> <upper bound> where <number of threads> and <upper bound> are integer powers of 2.\n");
    printf("Ex:\t./thr_atomic.c 8 64");
}

bool isPowerOf(int n, int base) {
    while (n % base == 0) n /= base;
    return n == 1;
}

struct RootCalculationThreadArgs {
    int threadNum;
    int numThreads;
    int upperBound;
};

void * calculateRoot(void *_args) {
    // Args pre-processing
    struct RootCalculationThreadArgs *args = (struct RootCalculationThreadArgs *) _args;

    // Define bounds of the computation
    const int lowerLoopBound = (
        (args->threadNum) * (args->upperBound / args->numThreads)
    ) + 1;
    const int upperLoopBound = (
        (args->threadNum + 1) * (args->upperBound / args->numThreads)
    );

    assert(upperLoopBound > lowerLoopBound);

    /** Running sum of quadruple roots from lowerLoopBound to upperLoopBound */
    double threadTotal = 0;

    // Calculate the quadruple roots and accumulate resulting sum in `threadTotal`.
    for (int i = lowerLoopBound; i < upperLoopBound; i += 1.0f) {
        threadTotal += pow(i * 1.0, 1.0/4);
    }

    // Critical section. Thread now waits for exclusive access to the `threadTotal` shared variable.
    sem_wait(&semaphore);

    // Thread has entered critical section. It atomically adds its total to the global total.
    printf("Thread %d finished calculating %d quadruple roots from %d to %d with a sum of %f.\n", args->threadNum, upperLoopBound - lowerLoopBound + 1, lowerLoopBound, upperLoopBound, threadTotal);
    free(args);
    total += threadTotal;
    sem_post(&semaphore);

    // End critical section

    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char ** argv) {
    if (argc != NUM_ARGS) {
        printf("Invalid number of arguments received; expected %d, received %d\n", NUM_ARGS, argc);
        exit(1);
    }

    const int numThreads = atoi(argv[1]);
    const int upperBound = atoi(argv[2]);
    const bool numThreadsValidPow = isPowerOf(numThreads, ARGS_POWER_OF);
    const bool upperBoundValidPow = isPowerOf(upperBound, ARGS_POWER_OF);
    
    if (!numThreadsValidPow || !upperBoundValidPow) {
        printf("Invalid integer argument. Integer arguments must be a power of %d.\n", ARGS_POWER_OF);
        if (!numThreadsValidPow) printf("numThreads(%d) is not a power of %d.\n", numThreads, ARGS_POWER_OF);
        if (!upperBoundValidPow) printf("upperBound(%d) is not a power of %d.\n", upperBound, ARGS_POWER_OF);
        exit(1);
    }

    sem_init(&semaphore, 0, 1);

    for (int threadNum = 0; threadNum < numThreads; ++threadNum) {
        pthread_t threadId;
        struct RootCalculationThreadArgs* args = malloc(sizeof (struct RootCalculationThreadArgs));
        args->threadNum = threadNum;
        args->numThreads = numThreads;
        args->upperBound = upperBound;
        int err = pthread_create(&threadId, NULL, &calculateRoot, args);
        if (err) {
            printf("Failed to create thread %d", threadNum);
            exit(1);
        }
    }

    sem_wait(&semaphore);
    sem_destroy(&semaphore);

    printf("The sum of fourth roots from %d to %d is %f.\n", 1, upperBound, total);
}