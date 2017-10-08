/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Concurrency Problem 1
 * October 9, 2017
 * producer_consumer.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include "mt19937ar.c"

#define MAX_BUFFER_SIZE 32
#define NUM_PRODUCERS 10
#define NUM_CONSUMERS 10

struct Data {
    int number;
    int wait_time;
};

struct Data buffer[MAX_BUFFER_SIZE];
int num_items;

sem_t sem_c;
sem_t sem_p;
pthread_mutex_t mutLock;
pthread_cond_t condition_producer;
pthread_cond_t condition_consumer;

int rdrand(int min, int max);
void *producer(/*void *arg*/);
void *consumer(void *arg);

int main(/*int argc, char **argv*/) {
    int i;
    num_items = 0;
    pthread_t producer_threads[NUM_PRODUCERS];
    pthread_t consumer_threads[NUM_CONSUMERS];

    if(sem_init(&sem_c, 0, 0) != 0) {
        perror("Error initializing sem_min");
    }

    if(sem_init(&sem_p, 0, MAX_BUFFER_SIZE) != 0) {
        perror("Error initializing sem_max");
    }

    pthread_mutex_init(&mutLock, 0);

    for(i = 0; i < NUM_PRODUCERS; i++) {
        if(pthread_create(&producer_threads[i], NULL, (void*)producer, NULL) != 0) {
            perror("Error creating producer pthread");
        }
    }

    for(i = 0; i < NUM_CONSUMERS; i++) {
        if(pthread_create(&consumer_threads[i], NULL, (void*)consumer, NULL)) {
            perror("Error creating consumer pthread");
        }
    }

    for(i = 0; i < NUM_CONSUMERS; i++) {
        if(pthread_join(consumer_threads[i], NULL) != 0) {
            perror("Error joining consumer pthread");
        }
    }

    return 0;
}

/***********************************************************************
 Reference: rdrand_test.c from Materials from Class by Dr. Kevin McGrath
***********************************************************************/
/**
  @brief Generates a pseudo-random integer value between a given range
  @param min - the lower bound of the integer range
  @param max - the upper bound of the integer range
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
  @brief
  @param
  @return
*/
void *producer(/*void *arg*/) {
    struct Data data;

    while (1) {
        // Wait a random amount of time in between 3-7 secs
        int sleep_time = rdrand(3, 7);
        sleep(sleep_time);

        pthread_mutex_lock(&mutLock);

        if (num_items >= MAX_BUFFER_SIZE){
            pthread_mutex_unlock(&mutLock);
            continue;
        }

        buffer[num_items] = data;

        num_items++;

        printf("Producer has produced # %d...\n", data.number);

        pthread_mutex_unlock(&mutLock);
    }

    printf("Producer thread has ended...\n");
}

/**
  @brief
  @param
  @return
*/
void *consumer(void *arg) {
    //struct Data data;

    printf("Consumer Thread begins...\n");

    while (1) {
        printf("Consumer Thread %d is working...\n", (int)arg);

        pthread_mutex_lock(&mutLock);

        if (num_items == 0) {
            printf("Waiting for a producer...\n");
            pthread_mutex_unlock(&mutLock);
            continue;
        }

        num_items--;
        sleep(buffer[num_items].wait_time);

        printf("Consumer %d found \n", (int)arg);
        pthread_mutex_unlock(&mutLock);
    }
    printf("Consumer Thread ends...\n");
}

