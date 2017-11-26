/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Concurrency 3 part 1 
 * November 25, 2017
 * References: https://lab.cs.ru.nl/algemeen/images/8/8f/Opgavenserie9.pdf
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

sem_t mutex;
sem_t blocker;

int active;
int waiting;
int must_wait = 0;

pthread_t threads[5];

void* shareable(void *arg);

int main(){
	sem_init(&mutex, 0, 1);
	sem_init(&blocker, 0, 0);

	for (int i = 0; i < 5; i++){
		printf("Creating threads\n");
		pthread_create(&threads[i], NULL, shareable, NULL);
	}

	for (int i = 0; i < 5; i++){
		pthread_join(threads[i], NULL);
	}
	return 0;
}

void* shareable(void *arg){
	while (1){
		sem_wait(&mutex);
		
		if (must_wait){         
			sem_post(&mutex);
			sem_wait(&blocker);
			waiting--;
		}

		active++;

		printf("Current active: %d\n", active);

		if (active == 3){  
			sleep(5);     
			must_wait = 1;
		}
		else{
			must_wait = 0;
		}

		if (waiting > 0 && must_wait == 0){
			printf("Unblock a waiting process\n");
			sem_post(&blocker);
		}
		else{
			printf("Open the mutual exclusion\n");
			sem_post(&mutex);
		}

		sem_wait(&mutex);
		printf("Entered mutal exclusion\n");
		active--;
		if (active == 0){
			must_wait = 0;
		}
		
		if (waiting != 0 && must_wait == 0){
			printf("Unblocking a thread\n");
			sem_post(&blocker);
		}
		else{
			printf("Opening mutex exclusion\n");
			sem_post(&mutex);
		}
	}
}



