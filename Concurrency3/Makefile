 # Jason Ye - yeja@oregonstate.edu
 # Corey Hemphill - hemphilc@oregonstate.edu
 # CS444 - Concurrency 3 Makefile
 # November 26, 2017
 # Makefile

#************************************************************************
# Reference: makefile from Materials from Class by Dr. Kevin McGrath
#************************************************************************

all: prob1_conc3 prob2_conc3

prob1_conc3: prob1_conc3.c 
	gcc -std=c99 prob1_conc3.c -o prob1_conc3 -lpthread
	
prob2_conc3: prob2_conc3.c 
	gcc -std=c99 prob2_conc3.c -o prob2_conc3 -lpthread

clean:
	rm prob1_conc3 prob2_conc3