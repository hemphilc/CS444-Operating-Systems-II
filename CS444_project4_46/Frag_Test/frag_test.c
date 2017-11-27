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
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

/*
 * Added system calls for testing the best-fit algorithm
 * implementation in slob.c. Added syscall declarations to
 * the following files:
 * - linux/arch/sh/include/uapi/asm/unistd_32.h
 * - arch/x86/syscalls/syscall_32.tbl
 * - include/linux/syscalls.h
 */
#define sys_get_slob_amt_claimed syscall(369)
#define sys_get_slob_amt_free syscall(370)

#define DEF_NUM_TESTS 10

int main(int argc, char **argv) {
	unsigned int i;
	float frag;
	int num_tests;
	
	if (argc > 2) {
		printf("Usage: frag_test <# of test iterations>\n\n");
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
		
		printf("\n************ TEST #%d ************\n", i + 1);
		printf("Fragmentation: %%f\n", frag * 100);
		printf("Claimed Memory: %lu\n", sys_get_slob_amt_claimed);
		printf("Free Memory: %lu\n", sys_get_slob_amt_free);
		
		sleep(10);
	}
}
