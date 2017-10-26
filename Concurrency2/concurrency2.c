#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t print_lock;
pthread_mutex_t fork[5];

struct philosopher_info{
	char name[10];
	int seat_num;
};

void think(struct philosopher_info *person){
	
	pthread_mutex_lock(&print_lock);
	printf("%s is thinking... \n", person->name);
	pthread_mutex_unlock(&print_lock);
	sleep(rand()%20+1);	
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

void put_fork(struct philosopher_info *person){
	int first_fork;
	int second_fork;
	if(person->seat_num+1 == 5){
		first_fork = 0;
		second_fork = person->seat_num;
	}
	else{
		first_fork = person->seat_num;
		second_fork = person->seat_num+1;
	}
	pthread_mutex_lock(&print_lock);
	printf("%s put down the forks... \n", person->name);
	pthread_mutex_unlock(&print_lock);
	pthread_mutex_unlock(&forks[first_fork);
	pthread_mutex_unlock(&forks[second_fork);
	
}

int main(){
	return 0;
}