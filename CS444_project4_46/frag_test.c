/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Project 4 - The SLOB SLAB
 * November 26, 2017
 * frag_test.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define sys_get_slob_amt_claimed syscall(369)
#define sys_get_slob_amt_free syscall(370)

#define DEF_NUM_TESTS 10

int main(int argc, char **argv) {
	unsigned int i;
	float frag;
	int num_tests;
	
	if (argc > 2) {
		perror("Usage: frag_test <# of test iterations>\n");
		exit(1);
	}
	else if (argc == 2) {
		num_tests = atoi(argv[1]);
	}
	else {
		num_tests = DEF_NUM_TESTS;
	}
	
	printf("Running %d frag tests...\n", num_tests);

	for (i = 0; i < num_tests; i++) {
		frag = (float)sys_get_slob_amt_free / (float)sys_get_slob_amt_claimed;
		
		printf("************TEST #%d************\n", i);
		printf("Fragmentation %: %f\n", frag);
		printf("Claimed Memory: %lu\n", sys_get_slob_amt_claimed);
		printf("Free Memory: %lu\n", sys_get_slob_amt_free);
		printf("********************************\n");
		sleep(1);
	}
}
