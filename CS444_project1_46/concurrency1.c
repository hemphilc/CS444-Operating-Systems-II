#include <studio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mt19937ar.c"
#include <time> 

#define BUFFER_SIZE 32

struct Data{
	int number;
	int wait_time;
};

struct Data item[BUFFER_SIZE];
int buffer_num = 0;

pthread_mutex_t mutLock;
pthread_cond_t condition_producer;
pthread_cond_t condition_consumer;

//***********************************************************************
//Reference: rdrand_test.c from Materials from Class by Dr. Kevin McGrath
//***********************************************************************
int rdrand(int min, int max)
{
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	int r;

	char vendor[13];

	eax = 0x01;

	__asm__ __volatile__(
		"cpuid;"
		: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
		: "a"(eax)
		);

	if (ecx & 0x40000000){
		unsigned int tmp = 0;
		__asm__ __volatile__(
			"rdrand %0;"
			: "=r" (tmp)
			);
		r = tmp % (max - min + 1) + min;

	}
	else{
		//use mt19937
		r = (genrand_int32() % (max - min + 1)) + min;
	}

	return r;
}

void *producer(void *arg){
	int position = 0;
	struct Data data;

	sleepTime = rdrand(3, 7);

	sleep(sleepTime);

	while (1){
		pthread_mutex_lock(&mutLock);

		if (buffer_num >= BUFFER_SIZE){
			pthread_mutex_unlock(&mutLock);
			continue;
		}
		
		item[buffer_num] = data;
		buffer_num++;

		printf("Producer has produced # %d...\n", data.number);

		pthread_mutex_unlock(&mutLock);
		position = 1;
	}

	printf("Producer Thread ends...\n");

}


void *consumer(void *arg){
	int position = 0;
	struct Data data;
	printf("Consumer Thread begins...\n");

	while (1){
		printf("Consumer Thread %d is working atm...\n", (int)arg);
		pthread_mutex_lock(&mutLock);
		
		if (buffer_num == 0){
			printf("Waiting for produce...\n");
			pthread_mutex_unlock(&mutLock);
			continue;
		}

		buffer_num--;
		position = 1;
		sleep(item[buffer_num].wait_time);

		printf("Consumer %d found \n", (int)arg);
		pthread_mutex_unlock(&mutLock);

		
	}
	printf("Consumer Thread end...\n");
}
