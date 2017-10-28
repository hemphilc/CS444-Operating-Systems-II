/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Concurrency 2 
 * October 26, 2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define TRUE 1
#define VERBOSE 1
#define NUM_PHILOSOPHERS 5
#define NAME_LEN 20
#define THINK_TIME 20
#define EAT_TIME 8

pthread_mutex_t print_lock;
pthread_mutex_t forks[NUM_PHILOSOPHERS];

struct philosopher_info {
	char name[NAME_LEN];
	int seat_num;
};

void think(struct philosopher_info *philosopher);
void eat(struct philosopher_info *philosopher);
void get_forks(struct philosopher_info *philosopher);
void put_forks(struct philosopher_info *philosopher);
void *philosopher_action(void *philosopher);

int main() {
	struct philosopher_info *philosophers;
	//five philosophers will be on the table
	philosophers = malloc(sizeof(struct philosopher_info) * NUM_PHILOSOPHERS);

	//clear all name buffers
	memset(philosophers[0].name, '\0', sizeof(philosophers[0].name));
        memset(philosophers[1].name, '\0', sizeof(philosophers[1].name));
        memset(philosophers[2].name, '\0', sizeof(philosophers[2].name));
        memset(philosophers[3].name, '\0', sizeof(philosophers[3].name));
        memset(philosophers[4].name, '\0', sizeof(philosophers[4].name));

	//assign philosopher names
	strcpy(philosophers[0].name, "Plato");
        strcpy(philosophers[1].name, "Aristotle");
        strcpy(philosophers[2].name, "Epicurus");
        strcpy(philosophers[3].name, "Parmenides");
        strcpy(philosophers[4].name, "Socrates"); //pronounced "So-Crates"

	int i;
	//assign each philosopher a seat
	for (i = 0; i < NUM_PHILOSOPHERS; i++) {
		philosophers[i].seat_num = i;
	}

	//mutex initialization
	pthread_mutex_init(&print_lock, NULL);
	for (i = 0; i < NUM_PHILOSOPHERS; i++) {
		pthread_mutex_init(&forks[i], NULL);
	}

	//thread creation
	pthread_t threads[NUM_PHILOSOPHERS];
	for (i = 0; i < NUM_PHILOSOPHERS; i++) {
		pthread_create(&threads[i], NULL, philosopher_action, &philosophers[i]);
	}

	//thread joining
	for (i = 0; i < NUM_PHILOSOPHERS; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}

void think(struct philosopher_info *philosopher) {
	pthread_mutex_lock(&print_lock);
	
	if (VERBOSE)
		printf("%s is thinking...\n", philosopher->name);
	
	pthread_mutex_unlock(&print_lock);
	
	sleep((rand() % THINK_TIME) + 1);	
	
	if (VERBOSE)
		printf("%s finished thinking...\n", philosopher->name);
}

void eat(struct philosopher_info *philosopher) {
	pthread_mutex_lock(&print_lock);

	if (VERBOSE)
		printf("%s is eating...\n", philosopher->name);
	
	pthread_mutex_unlock(&print_lock);
	
	sleep(rand() % EAT_TIME + 2);
	
	pthread_mutex_lock(&print_lock);
	
	if (VERBOSE)
		printf("%s finished eating...\n", philosopher->name);
	
	pthread_mutex_unlock(&print_lock);
}

void get_forks(struct philosopher_info *philosopher) {
	int right_fork;
	int left_fork;

	if ((philosopher->seat_num + 1) == NUM_PHILOSOPHERS) {
		right_fork = 0;

		left_fork = philosopher->seat_num;
	}
	else {
		right_fork = philosopher->seat_num;
		
		left_fork = philosopher->seat_num + 1;
	}

	//right fork
	pthread_mutex_lock(&print_lock);
	
	if (VERBOSE)
		printf("Fork #%d assigned to %s\n", right_fork, philosopher->name);
	
	pthread_mutex_unlock(&print_lock);
	pthread_mutex_lock(&forks[right_fork]);

	//left fork
	pthread_mutex_lock(&print_lock);
	
	if (VERBOSE)
		printf("Fork #%d assigned to %s\n", left_fork, philosopher->name);
	
	pthread_mutex_unlock(&print_lock);
	pthread_mutex_lock(&forks[left_fork]);
}

void put_forks(struct philosopher_info *philosopher) {
	int right_fork;
	int left_fork;

	if (philosopher->seat_num + 1 == NUM_PHILOSOPHERS) {
		right_fork = 0;

		left_fork = philosopher->seat_num;
	}
	else {
		right_fork = philosopher->seat_num;

		left_fork = philosopher->seat_num + 1;
	}

	pthread_mutex_lock(&print_lock);
	
	if (VERBOSE)
		printf("%s has put down the fork...\n", philosopher->name);
	
	pthread_mutex_unlock(&print_lock);
	pthread_mutex_unlock(&forks[right_fork]);
	pthread_mutex_unlock(&forks[left_fork]);
	
}

void *philosopher_action(void *philosopher) {
	//assigning each philosopher's actions
	struct philosopher_info *philosophers = (struct philosopher_info *)philosopher;
	
	while (TRUE) {
		think(philosophers);
		get_forks(philosophers);
		eat(philosophers);
		put_forks(philosophers);
	}
}
