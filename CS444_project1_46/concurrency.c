/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Concurrency Problem 1
 * October 9, 2017
 * concurrency.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include "mt19937ar.c"

#define MAX_BUFFER_SIZE 32
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define TRUE 1
#define DEBUG 1

struct Data {
    int value;
    int wait_time;
};

struct Data buffer[MAX_BUFFER_SIZE];
int num_items;

pthread_mutex_t mutex_c;
pthread_mutex_t mutex_p;

int rdrand(int min, int max);
void *producer();
void *consumer();

int main(/*int argc, char **argv*/) {
    int i;
    num_items = 0;
    pthread_t producer_threads[NUM_PRODUCERS];
    pthread_t consumer_threads[NUM_CONSUMERS];

    // Initialize the producer mutex
    if(pthread_mutex_init(&mutex_p, 0) != 0) {
        if(DEBUG) {
            perror("Error initializing producer mutex");
            exit(1);
        }
    }

    // Initialize the consumer mutex
    if(pthread_mutex_init(&mutex_c, 0) != 0) {
        if(DEBUG) {
            perror("Error initializing consumer mutex");
            exit(1);
        }
    }

    // Create the producer pthreads
    for(i = 0; i < NUM_PRODUCERS; i++) {
        if(pthread_create(&producer_threads[i], NULL, (void*)producer, NULL) != 0) {
            if(DEBUG) {
                perror("Error creating producer pthread");
                exit(1);
            }
        }
    }

    // Create the consumer pthreads
    for(i = 0; i < NUM_CONSUMERS; i++) {
        if(pthread_create(&consumer_threads[i], NULL, (void*)consumer, NULL) != 0) {
            if(DEBUG) {
                perror("Error creating consumer pthread");
                exit(1);
            }
        }
    }

    // Join the producer pthreads
    for(i = 0; i < NUM_PRODUCERS; i++) {
        if(pthread_join(producer_threads[i], NULL) != 0) {
            if(DEBUG) {
                perror("Error joining producer pthread");
                exit(1);
            }
        }
    }

    // Join the consumer pthreads
    for(i = 0; i < NUM_CONSUMERS; i++) {
        if(pthread_join(consumer_threads[i], NULL) != 0) {
            if(DEBUG) {
                perror("Error joining consumer pthread");
                exit(1);
            }
        }
    }

    return 0;
}

/***********************************************************************
 Reference: rdrand_test.c from Materials from Class by Dr. Kevin McGrath
***********************************************************************/
/**
  @brief rdrand generates a pseudo-random integer value within a range
  @param min - the lower bound of the acceptable range
  @param max - the upper bound of the acceptable range
  @return a random integer value within the min and max range
*/
int rdrand(int min, int max) {
    unsigned int eax = 0x01;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    int r;

    __asm__ __volatile__(
        "cpuid;"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
        );

    if (ecx & 0x40000000) {
        unsigned int tmp = 0;
        __asm__ __volatile__(
            "rdrand %0;"
            : "=r" (tmp)
            );
        r = tmp % (max - min + 1) + min;
    }
    else {
        // Use MT genrand function from mt19937ar.c
        r = (genrand_int32() % (max - min + 1)) + min;
    }

    return r;
}

/**
  @brief Producer generates a Data item and adds to buffer
  @return none
*/
void *producer() {
    printf("Producer thread has begun...\n");

    struct Data data;

    while(TRUE) {
        // Wait a random amount of time in between 3-7 secs
        int sleep_time = rdrand(3, 7);
        sleep(sleep_time);

        if(pthread_mutex_lock(&mutex_p) != 0) {
            if(DEBUG) {
                perror("Error locking producer mutex");
            }
        }

        // Block until a consumer removes an item
        if (num_items >= MAX_BUFFER_SIZE) {
            pthread_mutex_unlock(&mutex_p);
            printf("Waiting for a consumer to remove an item...\n");
            continue;
        }

        // Generate a Data item and add it to the buffer
        data.value = rdrand(0, 100);
        data.wait_time = rdrand(2, 9);
        buffer[num_items] = data;
        num_items++;

        if(pthread_mutex_unlock(&mutex_p) != 0) {
            if(DEBUG) {
                perror("Error unlocking producer mutex");
            }
        }

        printf("Producer has produced #%d...\n", data.value);
    }

    printf("Producer thread has ended...\n");
}

/**
  @brief Consumer removes a Data item from buffer
  @return none
*/
void *consumer() {
    printf("Consumer thread has begun...\n");

    struct Data data;

    while(TRUE) {
        printf("Consumer thread is working...\n");

        if(pthread_mutex_lock(&mutex_c) != 0) {
            if(DEBUG) {
                perror("Error locking consumer mutex");
            }
        }

        // Block until a producer adds a new item
        if (num_items == 0) {
            pthread_mutex_unlock(&mutex_c);
            printf("Waiting for a producer to add a new item...\n");
            continue;
        }

        // Remove an item from the buffer and sleep
        data = buffer[num_items];
        num_items--;
        sleep(data.wait_time);

        if(pthread_mutex_unlock(&mutex_c) != 0) {
            if(DEBUG) {
                perror("Error unlocking consumer mutex");
            }
        }

        printf("Consumer has consumed #%d\n", data.value);
    }

    printf("Consumer thread has ended...\n");
}
