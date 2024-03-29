/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Homework 2 - sstf I/O Scheduler
 * Team #46
 * October 23, 2017
 * sstf-iosched.c
*/

/*
 * elevator sstf
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

#define DEBUG 1

struct sstf_data {
        struct list_head queue;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
                                 struct request *next)
{
        list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
        struct sstf_data *sd = q->elevator->elevator_data;

        if (!list_empty(&sd->queue)) {
		struct request *rq;
		rq = list_entry(sd->queue.next, struct request, queuelist);
		
		if (DEBUG) {
			char op;
			// Determine which operation we're performing
			if (rq_data_dir(rq) == 0)
				op = 'r';
			else
				op = 'w';
			printk("SSTF: dispatching from queue %c %lu\n", op, (long)blk_rq_pos(rq));
		}

		list_del_init(&rq->queuelist);
            	elv_dispatch_sort(q, rq);
            	return 1;
        }
        return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *sd = q->elevator->elevator_data;
	struct list_head *curr_pos;
	sector_t add_req = blk_rq_pos(rq);
	sector_t end_ref = q->end_sector;
	int is_added = 0; // Set to 0 indicate false

	if (DEBUG) {
		char op;
               	// Determine which operation we're performing
               	if (rq_data_dir(rq) == 0)
			op = 'r';
                else
                	op = 'w';
		printk("SSTF: adding to queue %c %lu\n", op, (long)blk_rq_pos(rq));
	}

	// Add if list is empty regardless of where rq is
	if (list_empty(&sd->queue)) {
		list_add_tail(&rq->queuelist, &sd->queue);
	}
	else {
		// Iterate through the request list
		// and add requests based on the 
		// shortest seek time/distance
		list_for_each(curr_pos, &sd->queue) {
			struct request *curr_node = 
				list_entry(curr_pos, struct request, queuelist);
        		sector_t curr_req = blk_rq_pos(curr_node);

			// Vars for recording distances between
			// add, ref, and curr sectors
        		unsigned int add_diff = 0;
        		unsigned int curr_diff = 0;

			// Figure out the difference between the
			// sector we want to add and the last
			// sector of the disk
			if (add_req >= end_ref) {
				add_diff = add_req - end_ref;
			}
			else {
				add_diff = end_ref - add_req;
			}

			// Figure out the difference between the
			// current sector and the last sector of
			// the disk
			if (curr_req >= end_ref) {
				curr_diff = curr_req - end_ref;
			}
			else {
				curr_diff = end_ref - curr_req;
			}

			// Compare the current reqs distance and
			// the add reqs distance. If the current
			// req is greater, reinsert the current
			// req back into the queue after the the 
			// one we want to add
			if (curr_diff >= add_diff) {
				list_add_tail(&rq->queuelist, curr_pos);
				is_added = 1;
				break;
			}
			else {
                        	// The current request now becomes the
                        	// end disk/outlying sector
				end_ref = curr_req;
			}
		}

		if (is_added != 1) {
			list_add_tail(&rq->queuelist, curr_pos);
		}
	}
}

static struct request *
sstf_former_request(struct request_queue *q, struct request *rq)
{
        struct sstf_data *sd = q->elevator->elevator_data;

        if (rq->queuelist.prev == &sd->queue)
                return NULL;
        return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
sstf_latter_request(struct request_queue *q, struct request *rq)
{
        struct sstf_data *sd = q->elevator->elevator_data;

        if (rq->queuelist.next == &sd->queue)
                return NULL;
        return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
        struct sstf_data *sd;
        struct elevator_queue *eq;

        eq = elevator_alloc(q, e);
        if (!eq)
                return -ENOMEM;

        sd = kmalloc_node(sizeof(*sd), GFP_KERNEL, q->node);
        if (!sd) {
                kobject_put(&eq->kobj);
                return -ENOMEM;
        }
        eq->elevator_data = sd;

        INIT_LIST_HEAD(&sd->queue);

        spin_lock_irq(q->queue_lock);
        q->elevator = eq;
        spin_unlock_irq(q->queue_lock);
        
	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
        struct sstf_data *sd = e->elevator_data;

        BUG_ON(!list_empty(&sd->queue));
        kfree(sd);
}

static struct elevator_type elevator_sstf = {
        .ops = {
                .elevator_merge_req_fn          = sstf_merged_requests,
                .elevator_dispatch_fn           = sstf_dispatch,
                .elevator_add_req_fn            = sstf_add_request,
                .elevator_former_req_fn         = sstf_former_request,
                .elevator_latter_req_fn         = sstf_latter_request,
                .elevator_init_fn               = sstf_init_queue,
                .elevator_exit_fn               = sstf_exit_queue,
        },
        .elevator_name = "sstf",
        .elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
        return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void)
{
        elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);


MODULE_AUTHOR("Jason Ye, Corey Hemphill");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sstf IO scheduler");
