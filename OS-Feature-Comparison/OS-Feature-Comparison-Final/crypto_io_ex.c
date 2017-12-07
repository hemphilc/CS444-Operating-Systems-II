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
		
		for (i = 0; i < nbytes; i += crypto_cipher_blocksize(tfm)) {
			crypto_cipher_encrypt_one(tfm, target + i, origin + i);
		}
	}
	else {
		origin = dev->data + offset;
		target = buffer;
		
		printk("brdd: Reading from RAM Disk Device...\n");
		
		for (i = 0; i < nbytes; i += crypto_cipher_blocksize(tfm)) {
			crypto_cipher_decrypt_one(tfm, target, origin + i);
		}
	}
}
