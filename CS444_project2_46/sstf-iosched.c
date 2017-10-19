/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Project 2
 * October 18, 2017
 * sstf-ioched.c
*/

#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct sstf_data{
	struct list_head queue;
};

static struct request *

struct void sstf_merged_request(struct request_queue *q, struct request *rq, struct request *next){
	list_del_init(&next->queueList);
}

sstf_add(struct, struct){

}

sstf_dispatcher(){
	struct sstf_data *sd = q->elevator->elevator_data;

	if (!list_empty(&sd->queue)){



	}

}

sstf_latter_request(struct, struct){

}

sstf_former_request(struct, struct){

}

sstf_initializer_request(struct, struct){


}



sstf_exit(){

}










