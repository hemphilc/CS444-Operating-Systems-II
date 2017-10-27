#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t print_lock;
pthread_mutex_t fork[5];

#define TRUE  1

struct philosopher_info{
	char name[100];
	int seat_num;
};


void philosopher_names(struct philosopher_info *person){

	person = malloc(sizeof(struct philosopher_info) * 5);

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

	if (person->seat_num + 1 == 5){
		right_fork = 0;
		left_fork = person->seat_num;
	}
	else{
		right_fork = person->seat_num;
		left_fork = person->seat_num + 1;
	}

	phtread_mutex_lock(&print_lock);
	printf("Fork #%d assign to %s\n", right_fork, person->name);
	phtread_mutex_unlock(&print_lock);
	phtread_mutex_lock(&fork[right_fork]);

	phtread_mutex_lock(&print_lock);
	printf("Fork #%d assign to %s\n", left_fork, person->name);
	phtread_mutex_unlock(&print_lock);
	phtread_mutex_lock(&fork[left_fork]);

}

void put_forks(struct philosopher_info *person){
	int right_fork;
	int left_fork;
	if(person->seat_num+1 == 5){
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
	person = malloc(sizeof(struct philosopher_info) * 5);

	philosopher_names(person);

	int i;
	
	for (i = 0; i < 5; i++){
		person[i].seat_num = i;
	}

	pthread_mutex_init(&print_lock, NULL);
	for (i = 0; i < 5; i++){
		pthread_mutex_init(&fork[i], NULL);
	}

	pthread_t threads[5];
	for (i = 0; i < 5; i++){
		pthread_create(&threads[i], NULL, actions, &person[i]);
	}

	for (i = 0; i < 5; i++){
		pthread_join(threads[i], NULL);
	}

	return 0;
}