/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Concurrency 2 
 * October 26, 2017
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>

sem_t no_inserter;
sem_t no_searcher;
sem_t no_deleter;

sem_t scounter;
sem_t icounter;

int searchcounter = 0;
int insertcounter = 0;

struct list_node{
	int data;
	struct list_node *next;
};

struct list_head{
	struct list_node *head;
};

void insert(struct list_head *head, int d)
{
	if (sem_trywait(&icounter) < 0){
		sem_wait(&icounter);
	}

	insertcounter += 1;
	if (insertcounter == 1) {
		sem_wait(&no_inserter);
	}
	sem_post(&icounter);

	struct list_node *iterator;
	struct list_node *temp;

	if (head->head == NULL) {
		head->head = (struct list_node*)malloc(sizeof(struct list_node));
		head->head->data = d;
		head->head->next = NULL;
	}
	else {
	
}

void search(struct list_head *head, int d){

}

void delete(struct list_head *head, int d){

}

void *inserter(void *h) {
	int x;
	struct list_head *head = (struct list_head *)h;

}

void *searcher(void *h) {
	int x;
	struct list_head *head = (struct list_head *)h;

}

void *deleter(void *h) {
	int x;
	struct list_head *head = (struct list_head *)h;
	
}

