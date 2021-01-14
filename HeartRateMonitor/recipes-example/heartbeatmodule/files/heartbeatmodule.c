#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "data.h"	// sensor data

static dev_t heartbeatmodule_dev;
struct cdev heartbeatmodule_cdev;
static char buffer[64];
static int index = 0; //index resets every 2048 reads

ssize_t heartbeatmodule_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	int data;	
	if (index == 2048) 
		index = 0;
	
	data = ppg[index];
	index++;

	// printk(KERN_INFO "[heartbeatmodule] read (count=%d, offset=%d)\n", (int)count, (int)*f_pos);

	copy_to_user(buf, &data, sizeof(int));
	return count;
}	

struct file_operations heartbeatmodule_fops = {
	.owner = THIS_MODULE,
	.read = heartbeatmodule_read,
};

static int __init heartbeatmodule_module_init(void) {
	printk(KERN_INFO "Loading heartbeatmodule\n");

	alloc_chrdev_region(&heartbeatmodule_dev, 0, 1, "heartbeatmodule_dev");
	printk(KERN_INFO "%s\n", format_dev_t(buffer, heartbeatmodule_dev));

	cdev_init(&heartbeatmodule_cdev, &heartbeatmodule_fops);
	heartbeatmodule_cdev.owner = THIS_MODULE;
	cdev_add(&heartbeatmodule_cdev, heartbeatmodule_dev, 1);
	return 0;
}

static void __exit heartbeatmodule_module_cleanup(void) {
	printk(KERN_INFO "Cleaning-up heartbeatmodule_dev.\n");

	cdev_del(&heartbeatmodule_cdev);
	unregister_chrdev_region(heartbeatmodule_dev,1);
	
}

module_init(heartbeatmodule_module_init);
module_exit(heartbeatmodule_module_cleanup);

MODULE_AUTHOR("Kristina Rogacheva");
MODULE_LICENSE("GPL");

