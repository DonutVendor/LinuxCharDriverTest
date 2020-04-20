/*
 * Devon Mickels
 * 4/20/2020
 * ECE 373
 *
 * Char Driver
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVCNT 5
#define DEVNAME "testCDriver"

static struct mydev_dev {
	struct cdev my_cdev;
	
	int syscall_val;
} mydev;

static dev_t mydev_node;

/* this shows up under /sys/modules/testCDriver/parameters */
static int starting_val = 15;
module_param(starting_val, int, S_IRUSR | S_IWUSR);

static int testCDriver_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "successfully opened!\n");

	mydev.syscall_val = starting_val;

	return 0;
}

static ssize_t testCDriver_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
	int ret; //Get a local kernel buffer sert aside

	/* Make sure our user wasn't bad... */
	if (!buf) {
		ret = -EINVAL;
		goto out;
	}

	if (copy_to_user(buf, &mydev.syscall_val, sizeof(int))) {
		ret = -EFAULT;
		goto out;
	}
	ret = sizeof(int);
	*offset += sizeof(int);

	/* Good to go, so printk the thingy */
	printk(KERN_INFO "User got from us %d\n", mydev.syscall_val);

out:
	return ret;
}

static ssize_t testCDriver_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	/* Have local kernel memory ready */
	char *kern_buf;
	int ret;

	/* Make sure our user isn't bad... */
	if (!buf) {
		ret = -EINVAL;
		goto out;
	}

	/* Get some memory to copy into... */
	kern_buf = kmalloc(len, GFP_KERNEL);

	/* ...and make sure it's good to go */
	if (!kern_buf) {
		ret = -ENOMEM;
		goto out;
	}
	
	//Dont think that I actually need the kern_buf here as I am writing the value directly
	//Would be needed if I was to do some kind of data validation first?
	
	/* Copy from the user-provided buffer */
	if (copy_from_user(&mydev.syscall_val, buf, len)) {
		/* uh-oh... */
		ret = -EFAULT;
		goto mem_out;
	}

	ret = len;

	/* print what userspace gave us */
	printk(KERN_INFO "Userspace wrote \"%d\" to us\n", *(unsigned int*)buf); //kern_buf);

mem_out:
	kfree(kern_buf);
out:
	return ret;
}

/* File operations for our device */
static struct file_operations mydev_fops = {
	.owner = THIS_MODULE,
	.open = testCDriver_open,
	.read = testCDriver_read,
	.write = testCDriver_write,
};

static int __init testCDriver_init(void)
{
	printk(KERN_INFO "testCDriver module loading... "); //", exam, exam_nosysfs);

	if (alloc_chrdev_region(&mydev_node, 0, DEVCNT, DEVNAME)) {
		printk(KERN_ERR "alloc_chrdev_region() failed!\n");
		return -1;
	}

	printk(KERN_INFO "Allocated %d devices at major: %d\n", DEVCNT,
	       MAJOR(mydev_node));

	/* Initialize the character device and add it to the kernel */
	cdev_init(&mydev.my_cdev, &mydev_fops);
	mydev.my_cdev.owner = THIS_MODULE;

	if (cdev_add(&mydev.my_cdev, mydev_node, DEVCNT)) {
		printk(KERN_ERR "cdev_add() failed!\n");
		/* clean up chrdev allocation */
		unregister_chrdev_region(mydev_node, DEVCNT);

		return -1;
	}

	return 0;
}

static void __exit testCDriver_exit(void)
{
	/* destroy the cdev */
	cdev_del(&mydev.my_cdev);

	/* clean up the devices */
	unregister_chrdev_region(mydev_node, DEVCNT);

	printk(KERN_INFO "testCDriver module unloaded!\n");
}

MODULE_AUTHOR("Devon Mickels");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(testCDriver_init);
module_exit(testCDriver_exit);
