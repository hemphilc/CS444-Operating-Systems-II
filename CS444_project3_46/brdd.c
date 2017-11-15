/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Homework 3 - Kernel Crypto API
 * November 13, 2017
 * brdd.c
*/

/*
 * Original sbull.c example file obtained from:
 * https://github.com/duxing2007/ldd3-examples-3.x/blob/master/sbull/sbull.c
 *
 * Also, see https://static.lwn.net/images/pdf/LDD3/ch16.pdf for further info
 *
 * RAM Disk device driver for the Linux Kernel which allocates a chunk 
 * of memory and presents it as a block device. Uses the Linux Kernel's 
 * Crypto API; block device driver encrypts and decrypts data when it is 
 * written and read.
 */

/*
 * Sample disk driver, from the beginning.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/timer.h>
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>
#include <linux/crypto.h>


MODULE_LICENSE("Dual BSD/GPL");

static int brdd_major = 0;
module_param(brdd_major, int, 0);
static int hardsect_size = 512;
module_param(hardsect_size, int, 0);
static int nsectors = 1024;	/* How big the drive is */
module_param(nsectors, int, 0);
static int ndevices = 4; /* The number of RAM disk devices */
module_param(ndevices, int, 0);

/* Add in variables for using crypto */
#define CIPHER_KEY "1234567890123456"
#define CIPHER_KEY_LEN 16

static char *key = CIPHER_KEY;
static int key_len = CIPHER_KEY_LEN;
static struct crypto_cipher *tfm;

/*
 * The different "request modes" we can use.
 */
enum {
	RM_SIMPLE  = 0,	/* The extra-simple request function */
	RM_FULL    = 1,	/* The full-blown version */
	RM_NOQUEUE = 2,	/* Use make_request */
};
static int request_mode = RM_SIMPLE;
module_param(request_mode, int, 0);

/*
 * Minor number and partition management.
 */
#define BRDD_MINORS	16
#define MINOR_SHIFT	4
#define DEVNUM(kdevnum)	(MINOR(kdev_t_to_nr(kdevnum)) >> MINOR_SHIFT

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE	512

/*
 * After this much idle time, the driver will simulate a media change.
 */
#define INVALIDATE_DELAY	30*HZ

/*
 * The internal representation of our device.
 */
struct brdd_dev {
        int size;                       /* Device size in sectors */
        u8 *data;                       /* The data array */
        short users;                    /* How many users */
        short media_change;             /* Flag a media change? */
        spinlock_t lock;                /* For mutual exclusion */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
        struct timer_list timer;        /* For simulated media changes */
};

static struct brdd_dev *Devices = NULL;

static int bytes_to_sectors_checked(unsigned long bytes)
{
	if(bytes % KERNEL_SECTOR_SIZE)
	{
		printk("brdd: ERROR: BYTE/SECTOR DISCREPANCY\n");
	}
	return bytes / KERNEL_SECTOR_SIZE;
}

/*
 * Print data
 */
 // static void print_data(unsigned char *buffer, unsigned int length) 
 // {
	// int i;

	// printk("brdd: ");
	// for (i = 0; i < length; i++)
		// printk("%02x", *buffer++);
	// printk("\n");
 // }

/*
 * Handle an I/O request.
 */
static void brdd_transfer(struct brdd_dev *dev, unsigned long sector,
		unsigned long nsect, char *buffer, int write)
{
	uint8_t *origin;
	uint8_t *target;
	unsigned long i;
	unsigned long offset = sector*KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE;

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	
	/*
	 * Determine whether we are performing a read or a write
	 */
	if (write) {
		origin = buffer;
		target = dev->data + offset;
		
		printk("brdd: Writing to RAM Disk Device...\n");
		
		// print_data(origin, nbytes);
		
		printk("brdd: Performing Encryption...\n");
		printk("brdd: nbytes = %ld\n", nbytes);
		printk("brdd: offset = %ld\n", offset);
		//printk("brdd: cipher block size = %d\n", crypto_cipher_blocksize(tfm));
		
		for (i = 0; i < nbytes; i += 128) {
			crypto_cipher_encrypt_one(tfm, target + i, origin + i);
		}
		
		// print_data(target, nbytes);
	}
	else {
		origin = dev->data + offset;
		target = buffer;
		
		printk("brdd: Reading from RAM Disk Device...\n");
		
		// print_data(origin, nbytes);
		
		printk("brdd: Performing Decryption...\n");
		printk("brdd: nbytes = %ld\n", nbytes);
		printk("brdd: offset = %ld\n", offset);
		//printk("brdd: cipher block size = %d\n", crypto_cipher_blocksize(tfm));
		
		for (i = 0; i < nbytes; i += 128) {
			crypto_cipher_decrypt_one(tfm, target, origin + i);
		}
		
		// print_data(target, nbytes);
	}
}

/*
 * The simple form of the request function.
 */
static void brdd_request(struct request_queue *q)
{
	struct request *req;
	int ret;

	req = blk_fetch_request(q);
	while (req) {
		struct brdd_dev *dev = req->rq_disk->private_data;
		if ((req->cmd_type != REQ_TYPE_FS) || req == NULL) {
			printk(KERN_NOTICE "Skip non-fs request\n");
			__blk_end_request_all(req, -EIO);
			goto done;
		}
		printk (KERN_NOTICE "Req dev %u dir %d sec %ld, nr %d\n",
			(unsigned)(dev - Devices), rq_data_dir(req),
			(long)blk_rq_pos(req), blk_rq_cur_sectors(req));
		brdd_transfer(dev, blk_rq_pos(req), blk_rq_cur_sectors(req),
				bio_data(req->bio), rq_data_dir(req));
		ret = 0;
	done:
		if(!__blk_end_request_cur(req, ret)){
			req = blk_fetch_request(q);
		}
	}
}

/*
 * Transfer a single BIO.
 */
static int brdd_xfer_bio(struct brdd_dev *dev, struct bio *bio)
{
	struct bio_vec bvec;
	struct bvec_iter iter;
	sector_t sector = bio->bi_iter.bi_sector;

	/* Do each segment independently. */
	bio_for_each_segment(bvec, bio, iter) {
		char *buffer = __bio_kmap_atomic(bio, iter);
		brdd_transfer(dev, sector,bytes_to_sectors_checked(bio_cur_bytes(bio)),
				buffer, bio_data_dir(bio) == WRITE);
		sector += (bytes_to_sectors_checked(bio_cur_bytes(bio)));
		__bio_kunmap_atomic(bio);
	}
	return 0; /* Always "succeed" */
}

/*
 * Transfer a full request.
 */
static int brdd_xfer_request(struct brdd_dev *dev, struct request *req)
{
	struct bio *bio;
	int nsect = 0;
    
	__rq_for_each_bio(bio, req) {
		brdd_xfer_bio(dev, bio);
		nsect += bio->bi_iter.bi_size/KERNEL_SECTOR_SIZE;
	}
	return nsect;
}

/*
 * Smarter request function that "handles clustering".
 */
static void brdd_full_request(struct request_queue *q)
{
	struct request *req;
	struct brdd_dev *dev = q->queuedata;
	int ret;

	while ((req = blk_fetch_request(q)) != NULL) {
		if (req->cmd_type != REQ_TYPE_FS) {
			printk(KERN_NOTICE "Skip non-fs request\n");
			__blk_end_request_all(req, -EIO);
			goto done;
		}
		brdd_xfer_request(dev, req);
		ret = 0;
	done:
		__blk_end_request_all(req, ret);
	}
}

/*
 * The direct make request version.
 */
static void brdd_make_request(struct request_queue *q, struct bio *bio)
{
	struct brdd_dev *dev = q->queuedata;
	int status;

	status = brdd_xfer_bio(dev, bio);
	bio_endio(bio, status);
}

/*
 * Open and close.
 */
static int brdd_open(struct block_device *bdev, fmode_t mode)
{
	struct brdd_dev *dev = bdev->bd_disk->private_data;

	del_timer_sync(&dev->timer);
	spin_lock(&dev->lock);
	if (! dev->users) 
		check_disk_change(bdev);
	dev->users++;
	spin_unlock(&dev->lock);
	return 0;
}

static void brdd_release(struct gendisk *disk, fmode_t mode)
{
	struct brdd_dev *dev = disk->private_data;

	spin_lock(&dev->lock);
	dev->users--;

	if (!dev->users) {
		dev->timer.expires = jiffies + INVALIDATE_DELAY;
		add_timer(&dev->timer);
	}
	spin_unlock(&dev->lock);

}

/*
 * Look for a (simulated) media change.
 */
int brdd_media_changed(struct gendisk *gd)
{
	struct brdd_dev *dev = gd->private_data;
	
	return dev->media_change;
}

/*
 * Revalidate.  WE DO NOT TAKE THE LOCK HERE, for fear of deadlocking
 * with open.  That needs to be reevaluated.
 */
int brdd_revalidate(struct gendisk *gd)
{
	struct brdd_dev *dev = gd->private_data;
	
	if (dev->media_change) {
		dev->media_change = 0;
		memset (dev->data, 0, dev->size);
	}
	return 0;
}

/*
 * The "invalidate" function runs out of the device timer; it sets
 * a flag to simulate the removal of the media.
 */
void brdd_invalidate(unsigned long ldev)
{
	struct brdd_dev *dev = (struct brdd_dev *) ldev;

	spin_lock(&dev->lock);
	if (dev->users || !dev->data) 
		printk (KERN_WARNING "brdd: timer sanity check failed\n");
	else
		dev->media_change = 1;
	spin_unlock(&dev->lock);
}

/*
 * The ioctl() implementation
 */
int brdd_ioctl (struct block_device *bdev,
		 fmode_t mode,
                 unsigned int cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;
	struct brdd_dev *dev = bdev->bd_disk->private_data;

	switch(cmd) {
	    case HDIO_GETGEO:
        	/*
		 * Get geometry: since we are a virtual device, we have to make
		 * up something plausible.  So we claim 16 sectors, four heads,
		 * and calculate the corresponding number of cylinders.  We set the
		 * start of data at sector four.
		 */
		size = dev->size*(hardsect_size/KERNEL_SECTOR_SIZE);
		geo.cylinders = (size & ~0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
		geo.start = 4;
		if (copy_to_user((void __user *) arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}

	return -ENOTTY; /* unknown command */
}

/*
 * The device operations structure.
 */
static struct block_device_operations brdd_ops = {
	.owner           = THIS_MODULE,
	.open 	         = brdd_open,
	.release 	 = brdd_release,
	.media_changed   = brdd_media_changed,
	.revalidate_disk = brdd_revalidate,
	.ioctl	         = brdd_ioctl
};

/*
 * Set up our internal device.
 */
static void setup_device(struct brdd_dev *dev, int which)
{
	/*
	 * Get some memory.
	 */
	memset (dev, 0, sizeof (struct brdd_dev));
	dev->size = nsectors*hardsect_size;
	dev->data = vmalloc(dev->size);
	if (dev->data == NULL) {
		printk (KERN_NOTICE "vmalloc failure.\n");
		return;
	}
	spin_lock_init(&dev->lock);
	
	/*
	 * The timer which "invalidates" the device.
	 */
	init_timer(&dev->timer);
	dev->timer.data = (unsigned long) dev;
	dev->timer.function = brdd_invalidate;
	
	/*
	 * The I/O queue, depending on whether we are using our own
	 * make_request function or not.
	 */
	switch (request_mode) {
	    case RM_NOQUEUE:
		dev->queue = blk_alloc_queue(GFP_KERNEL);
		if (dev->queue == NULL)
			goto out_vfree;
		blk_queue_make_request(dev->queue, brdd_make_request);
		break;

	    case RM_FULL:
		dev->queue = blk_init_queue(brdd_full_request, &dev->lock);
		if (dev->queue == NULL)
			goto out_vfree;
		break;

	    default:
		printk(KERN_NOTICE "Bad request mode %d, using simple\n", request_mode);
        	/* fall into.. */
	
	    case RM_SIMPLE:
		dev->queue = blk_init_queue(brdd_request, &dev->lock);
		if (dev->queue == NULL)
			goto out_vfree;
		break;
	}
	blk_queue_logical_block_size(dev->queue, hardsect_size);
	dev->queue->queuedata = dev;
	/*
	 * And the gendisk structure.
	 */
	dev->gd = alloc_disk(BRDD_MINORS);
	if (! dev->gd) {
		printk (KERN_NOTICE "alloc_disk failure\n");
		goto out_vfree;
	}
	dev->gd->major = brdd_major;
	dev->gd->first_minor = which*BRDD_MINORS;
	dev->gd->fops = &brdd_ops;
	dev->gd->queue = dev->queue;
	dev->gd->private_data = dev;
	snprintf (dev->gd->disk_name, 32, "brdd%c", which + 'a');
	set_capacity(dev->gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
	add_disk(dev->gd);
	return;

  out_vfree:
	if (dev->data)
		vfree(dev->data);
}

static int __init brdd_init(void)
{
	int i;
	
	printk("brdd: Initializing Block RAM Disk");
	/*
	 * Get registered.
	 */
	brdd_major = register_blkdev(brdd_major, "brdd");
	if (brdd_major <= 0) {
		printk(KERN_WARNING "brdd: unable to get major number\n");
		return -EBUSY;
	}
	/*
	 * Allocate the device array, and initialize each one.
	 */
	Devices = kmalloc(ndevices*sizeof (struct brdd_dev), GFP_KERNEL);
	if (Devices == NULL)
		goto out_unregister;
	for (i = 0; i < ndevices; i++) 
		setup_device(Devices + i, i);
	
	/*
	 * Allocate memory for our cipher
	 */
	tfm = crypto_alloc_cipher("aes", 0, 0);
	if (tfm == NULL) {
		printk("brdd: cipher alloc failure - tfm is NULL");
		goto out_unregister;
	}
	/*
	 * Set the cipher key we want to use
	 */
	crypto_cipher_setkey(tfm, key, key_len);
	
	return 0;

  out_unregister:
	unregister_blkdev(brdd_major, "brdd");
	return -ENOMEM;
}

static void brdd_exit(void)
{
	int i;

	printk("brdd: Destroying Block RAM Disk");
	for (i = 0; i < ndevices; i++) {
		struct brdd_dev *dev = Devices + i;

		del_timer_sync(&dev->timer);
		if (dev->gd) {
			del_gendisk(dev->gd);
			put_disk(dev->gd);
		}
		if (dev->queue) {
			if (request_mode == RM_NOQUEUE)
				blk_put_queue(dev->queue);
			else
				blk_cleanup_queue(dev->queue);
		}
		if (dev->data)
			vfree(dev->data);
	}
	
	/*
	 * Free our cipher pointer memory and unregister our block device
	 */
	crypto_free_cipher(tfm);
	unregister_blkdev(brdd_major, "brdd");
	kfree(Devices);
}

module_init(brdd_init);
module_exit(brdd_exit);
