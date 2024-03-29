/*
 * Jason Ye - yeja@oregonstate.edu
 * Corey Hemphill - hemphilc@oregonstate.edu
 * CS444 - Homework 3 - Kernel Crypto API
 * November 13, 2017
 * brdd.c
*/

/*
 * Original Simple Block Driver sbd.c example file obtained from:
 * http://blog.superpat.com/2010/05/04/a-simple-block-driver-for-linux-kernel-2-6-31/
 *
 * Also, see https://static.lwn.net/images/pdf/LDD3/ch16.pdf for further info
 *
 * RAM Disk device driver for the Linux Kernel which allocates a chunk 
 * of memory and presents it as a block device. Uses the Linux Kernel's 
 * Crypto API; block device driver encrypts and decrypts data when it is 
 * written and read.
 */

/*
 * A sample, extra-simple block driver. Updated for kernel 2.6.31.
 *
 * (C) 2003 Eklektix, Inc.
 * (C) 2010 Pat Patterson <pat at superpat dot com>
 * Redistributable under the terms of the GNU GPL.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/crypto.h>

MODULE_LICENSE("Dual BSD/GPL");
//static char *Version = "1.4";


static struct crypto_cipher *tfm; // Crypto cipher structure
static char* key = "1234567890123456"; // Crypto key
module_param(key, charp, 0);
static int major_num = 0;
module_param(major_num, int, 0);
static int logical_block_size = 512;
module_param(logical_block_size, int, 0);
static int nsectors = 1024; /* How big the drive is */
module_param(nsectors, int, 0);

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE 512

/*
 * Our request queue.
 */
static struct request_queue *Queue;

/*
 * The internal representation of our device.
 */
static struct brdd_device {
	unsigned long size;
	spinlock_t lock;
	u8 *data;
	struct gendisk *gd;
} Device;

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
static void brdd_transfer(struct brdd_device *dev, sector_t sector,
		unsigned long nsect, char *buffer, int write) {
	uint8_t *origin;
	uint8_t *target;
	unsigned long i;
	unsigned long offset = sector * logical_block_size;
	unsigned long nbytes = nsect * logical_block_size;

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
		
		for (i = 0; i < nbytes; i += crypto_cipher_blocksize(tfm)) {
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
		
		for (i = 0; i < nbytes; i += crypto_cipher_blocksize(tfm)) {
			crypto_cipher_decrypt_one(tfm, target, origin + i);
		}
		
		// print_data(target, nbytes);
	}
}

static void brdd_request(struct request_queue *q) {
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		// blk_fs_request() was removed in 2.6.36 - many thanks to
		// Christian Paro for the heads up and fix...
		//if (!blk_fs_request(req)) {
		if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
			printk (KERN_NOTICE "Skip non-CMD request\n");
			__blk_end_request_all(req, -EIO);
			continue;
		}
		brdd_transfer(&Device, blk_rq_pos(req), blk_rq_cur_sectors(req),
				bio_data(req->bio), rq_data_dir(req));
		if ( ! __blk_end_request_cur(req, 0) ) {
			req = blk_fetch_request(q);
		}
	}
}

/*
 * The HDIO_GETGEO ioctl is handled in blkdev_ioctl(), which
 * calls this. We need to implement getgeo, since we can't
 * use tools such as fdisk to partition the drive otherwise.
 */
int brdd_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
	long size;

	/* We have no real geometry, of course, so make something up. */
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	return 0;
}

/*
 * The device operations structure.
 */
static struct block_device_operations brdd_ops = {
		.owner  = THIS_MODULE,
		.getgeo = brdd_getgeo
};

static int __init brdd_init(void) {
	printk("brdd: Initializing Block RAM Disk");
	
	/*
	 * Set up our internal device.
	 */
	Device.size = nsectors * logical_block_size;
	spin_lock_init(&Device.lock);
	Device.data = vmalloc(Device.size);
	if (Device.data == NULL)
		return -ENOMEM;
	/*
	 * Get a request queue.
	 */
	Queue = blk_init_queue(brdd_request, &Device.lock);
	if (Queue == NULL)
		goto out;
	blk_queue_logical_block_size(Queue, logical_block_size);
	/*
	 * Get registered.
	 */
	major_num = register_blkdev(major_num, "brdd");
	if (major_num < 0) {
		printk(KERN_WARNING "brdd: unable to get major number\n");
		goto out;
	}
	/*
	 * And the gendisk structure.
	 */
	Device.gd = alloc_disk(16);
	if (!Device.gd)
		goto out_unregister;
	Device.gd->major = major_num;
	Device.gd->first_minor = 0;
	Device.gd->fops = &brdd_ops;
	Device.gd->private_data = &Device;
	strcpy(Device.gd->disk_name, "brdd0");
	set_capacity(Device.gd, nsectors);
	Device.gd->queue = Queue;
	add_disk(Device.gd);
	
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
	crypto_cipher_setkey(tfm, key, strlen(key));

	return 0;

out_unregister:
	unregister_blkdev(major_num, "brdd");
out:
	vfree(Device.data);
	return -ENOMEM;
}

static void __exit brdd_exit(void)
{
	printk("brdd: Destroying Block RAM Disk");
	del_gendisk(Device.gd);
	put_disk(Device.gd);
	/*
	 * Free our cipher pointer memory and unregister our block device
	 */
	crypto_free_cipher(tfm);
	unregister_blkdev(major_num, "brdd");
	blk_cleanup_queue(Queue);
}

module_init(brdd_init);
module_exit(brdd_exit);
