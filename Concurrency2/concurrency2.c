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

#define TRUE  1
#define NUM_PHILOSOPHERS 5

pthread_mutex_t print_lock;
pthread_mutex_t fork[NUM_PHILOSOPHERS];

struct philosopher_info{
	char name[100];
	int seat_num;
};


void philosopher_names(struct philosopher_info *person){

	person = malloc(sizeof(struct philosopher_info) * NUM_PHILOSOPHERS);

	//delcaring their names
	strcpy(person[0].name, "Plato";);
	strcpy(person[1].name, "Aristotle";);
	strcpy(person[2].name, "Epicurus";);
	strcpy(person[3].name, "Parmenides";);
	strcpy(person[4].name, "Socrates";);
}

void think(struct philosopher_info *person){
	
	pthread_mutex_lock(&print_lock);
	printf("%s is thinking... \n", person->name);
	pthread_mutex_unlock(&print_lock);
	sleep(rand()%20+1);	
	printf("%s finished thinking... \n", person->name);
}

void eat(struct philosopher_info *person){
	pthread_mutex_lock(&print_lock);
	printf("%s is eating... \n", person->name);
	pthread_mutex_unlock(&print_lock);
	sleep(rand()%8+2);
	
	pthread_mutex_lock(&print_lock);
	printf("%s finishes eating... \n", person->name);
	pthread_mutex_unlock(&print_lock);
	
}

void get_forks(struct philosopher_info *person){

	int right_fork;
	int left_fork;

	if (person->seat_num + 1 == NUM_PHILOSOPHERS){
		right_fork = 0;
		left_fork = person->seat_num;
	}
	else{
		right_fork = person->seat_num;
		left_fork = person->seat_num + 1;
	}
	//first fork
	phtread_mutex_lock(&print_lock);
	printf("Fork #%d assign to %s\n", right_fork, person->name);
	phtread_mutex_unlock(&print_lock);
	phtread_mutex_lock(&fork[right_fork]);

	//second fork
	phtread_mutex_lock(&print_lock);
	printf("Fork #%d assign to %s\n", left_fork, person->name);
	phtread_mutex_unlock(&print_lock);
	phtread_mutex_lock(&fork[left_fork]);

}

void put_forks(struct philosopher_info *person){
	int right_fork;
	int left_fork;
	if(person->seat_num+1 == NUM_PHILOSOPHERS){
		right_fork = 0;
		left_fork = person->seat_num;
	}
	else{
		right_fork = person->seat_num;
		left_fork = person->seat_num+1;
	}

	pthread_mutex_lock(&print_lock);
	printf("%s put down the forks... \n", person->name);
	pthread_mutex_unlock(&print_lock);
	pthread_mutex_unlock(&forks[right_fork);
	pthread_mutex_unlock(&forks[left_fork);
	
}

void actions(void *philosopher){

	//assigning each philosopher's actions
	struct philosopher_info *person = (struct philosopher_info *)philosopher;
	
	while (TRUE){
		think(person);
		get_fork(person);
		eat(person);
		put_forks(person);
	}
	
}

int main(){

	struct philosopher_info *person;
	//five philosohper will be on the table
	person = malloc(sizeof(struct philosopher_info) * NUM_PHILOSOPHERS);

	philosopher_names(person);

	//assign each person a seat
	int i;
	
	for (i = 0; i < NUM_PHILOSOPHERS; i++){
		person[i].seat_num = i;
	}

	//mutex initialization
	pthread_mutex_init(&print_lock, NULL);
	for (i = 0; i < NUM_PHILOSOPHERS; i++){
		pthread_mutex_init(&fork[i], NULL);
	}

	pthread_t threads[NUM_PHILOSOPHERS];
	for (i = 0; i < NUM_PHILOSOPHERS; i++){
		pthread_create(&threads[i], NULL, actions, &person[i]);
	}

	for (i = 0; i < NUM_PHILOSOPHERS; i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}