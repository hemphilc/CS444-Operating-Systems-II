/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Concurrency 3 
 * November 25, 2017
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>

sem_t inserter_mut;
sem_t searcher_mut;
sem_t deleter_mut;

sem_t scounter;
sem_t icounter;

int search_counter = 0;
int insert_counter = 0;

pthread_t insert_threads[3];
pthread_t search_threads[3];
pthread_t delete_threads[3];

struct list_node{
	int data;
	struct list_node *next;
};

struct list_head{
	struct list_node *head;
};

void insert(struct list_head *head, int newValue)
{
	if (sem_trywait(&icounter) < 0){
		sem_wait(&icounter);
	}

	insert_counter += 1;
	if (insert_counter == 1) {
		sem_wait(&inserter_mut);
	}
	sem_post(&icounter);

	struct list_node *iterator;
	struct list_node *temp;

	if (head->head == NULL) {
		head->head = (struct list_node*)malloc(sizeof(struct list_node));
		head->head->data = newValue;
		head->head->next = NULL;
		printf("Inserted into head of the list with value %d\n", newValue);
	}
	else {
		iterator = head->head;
		while (iterator->next != NULL) {
			iterator = iterator->next;
		}
		temp = (struct list_node*)malloc(sizeof(struct list_node));
		temp->data = newValue;
		temp->next = NULL;
		iterator->next = temp;

		printf("Inserted into end of the list with value %d\n", newValue);
	}

	printf("Inserter %lu: Inserted %d.\n", pthread_self(), newValue);

	sem_wait(&icounter);
	insert_counter -= 1;
	if (insert_counter == 0) {
		sem_post(&inserter_mut);
	}
	sem_post(&icounter);
}

void *inserter(void *h) {
	int num;
	struct list_head *head = (struct list_head *)h;
	while (1){
		num = rand() % 10;
		insert(head, num);
		sleep(rand() % 4);
	}
}


void search(struct list_head *head, int newValue)
{
	//check for any block issue in searcher 
	if (sem_trywait(&scounter) < 0){
		sem_wait(&scounter);
	}

	search_counter += 1;
	if (search_counter == 1 && sem_trywait(&searcher_mut) < 0) {
		sem_wait(&searcher_mut);
	}

	sem_post(&scounter);
	int searched = 0;
	struct list_node *iterator;
	iterator = head->head;

	while (iterator != NULL) {
		if (iterator->data == newValue) {
			printf("Searcher %d: Found %d.\n", pthread_self(), newValue);
			searched = 1;
			break;
		}
		else {
			iterator = iterator->next;
		}
	}
	if (searched == 0) {
		printf("Searcher %d: Could not find %d.\n", pthread_self(), newValue);
	}

	sem_wait(&scounter);
	search_counter -= 1;
	if (search_counter == 0) {
		sem_post(&searcher_mut);
	}
	sem_post(&scounter);
}

void *searcher(void *h) {
	int num;
	struct list_head *head = (struct list_head *)h;
	while (1){
		num = rand() % 10;
		search(head, num);
		sleep(rand() % 4);
	}
}

void delete(struct list_head *head, int newValue){
	sem_wait(&searcher_mut);
	sem_wait(&inserter_mut);
	sem_wait(&deleter_mut);

	struct list_node *iterator, *iteratorp;
	iteratorp = NULL;
	int pos = 0;
	int deleted = 0;
	if (head->head == NULL) {
		printf("Deleter %d: Head is empty.\n", pthread_self());
		deleted = 1;
	}
	//check and print deleted threads 
	for (iterator = head->head; iterator != NULL; iteratorp = iterator, iterator = iterator->next) {
		if (iterator->data == newValue) {
			if (iteratorp == NULL) {
				head->head = iterator->next;
				free(iterator);
				printf("Deleter %d: Deleted %d head in the list.\n", pthread_self(), newValue);
				deleted = 1;
				break;
			}
			else {
				iteratorp->next = iterator->next;
				free(iterator);
				printf("Deleter %d: Deleted %d.\n", pthread_self(), newValue);
				deleted = 1;
				break;
			}
		}
	}

	if (deleted == 0) {
		printf("Deleter %d: Could not find %d.\n", pthread_self(), newValue);
	}

	sem_post(&searcher_mut);
	sem_post(&inserter_mut);
	sem_post(&deleter_mut);
}

void *deleter(void *h) {
	int num;
	struct list_head *head = (struct list_head *)h;
	while (1){
		num = rand() % 10;
		delete(head, num);
		sleep(rand() % 4);
	}
}

int main()
{
	sem_init(&inserter_mut, 0, 1);
	sem_init(&searcher_mut, 0, 1);
	sem_init(&deleter_mut, 0, 1);

	sem_init(&scounter, 0, 1);
	sem_init(&icounter, 0, 1);

	struct list_head *head;
	head = malloc(sizeof(struct list_head));
	srand(time(NULL));
	int i;

	for (i = 0; i < 3; i++) {
		pthread_create(&insert_threads[i], NULL, inserter, head);
		pthread_create(&search_threads[i], NULL, searcher, head);
		pthread_create(&delete_threads[i], NULL, deleter, head);
	}

	for (i = 0; i < 3; i++) {
		pthread_join(insert_threads[i], NULL);
		pthread_join(search_threads[i], NULL);
		pthread_join(delete_threads[i], NULL);
	}

	return 0;
}

