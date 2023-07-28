#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>

#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aaron Rumpler");
MODULE_DESCRIPTION("The Character Device That Never Ends");

static const char LYRICS[] = "This is the character device that never ends,\n"
			     "It just goes on and on my friends.\n"
			     "Some process started reading it, not knowing what it was,\n"
			     "And it'll continue reading it forever just because...\n\n";

static const size_t LYRICS_LENGTH = sizeof(LYRICS) - 1;

struct tcdtne_dev {
	struct cdev cdev;
	struct class *class;
	struct device *device;
} tcdtne_dev;

loff_t tcdtne_llseek(struct file *filp, loff_t off, int whence)
{
	loff_t newpos;

	printk(KERN_DEBUG "tcdtne: Seeking with filp->f_pos=%lld, off=%lld, whence=%d\n", filp->f_pos, off, whence);

	switch (whence) {
		case SEEK_SET:
			newpos = off;
			break;
		case SEEK_CUR:
			newpos = filp->f_pos + off;
			break;
		default:
			printk(KERN_WARNING "tcdtne: Whence value of %d not supported\n", whence);
			return -EINVAL;
	}

	if (newpos < 0) {
		printk(KERN_WARNING "tcdtne: Seek position of %lld is out of range\n", newpos);
		return -EINVAL;
	}

	printk(KERN_DEBUG "tcdtne: Seeking to pos=%lld\n", newpos);
	filp->f_pos = newpos;

	return newpos;
}

ssize_t tcdtne_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	size_t bytes_copied = 0;

	printk(KERN_DEBUG "tcdtne: Reading, *f_pos=%lld, count=%zu\n", *f_pos, count);

	while (bytes_copied < count) {
		size_t bytes_available_to_copy = LYRICS_LENGTH - (*f_pos % LYRICS_LENGTH);
		size_t bytes_to_copy = min(bytes_available_to_copy, count - bytes_copied);

		printk(KERN_DEBUG "tcdtne: Copying, *f_pos=%lld, bytes_copied=%zu, bytes_available_to_copy=%zu, bytes_to_copy=%zu, (*f_pos %% LYRICS_LENGTH)=%llu\n",
		       *f_pos, bytes_copied, bytes_available_to_copy, bytes_to_copy, *f_pos % LYRICS_LENGTH);

		if (copy_to_user(buf + bytes_copied, LYRICS + (*f_pos % LYRICS_LENGTH), bytes_to_copy)) {
			return -EFAULT;
		}

		bytes_copied += bytes_to_copy;
		*f_pos += bytes_to_copy;
	}

	printk(KERN_DEBUG "tcdtne: Read complete, bytes_copied=%zu\n", bytes_copied);

	return bytes_copied;
}

int tcdtne_open(struct inode *inode, struct file *filp)
{
	printk(KERN_DEBUG "tcdtne: File opened\n");
	return 0;
}

int tcdtne_release(struct inode *inode, struct file *filp)
{
	printk(KERN_DEBUG "tcdtne: File released\n");
	return 0;
}

struct file_operations tcdtne_fops = {
	.owner = THIS_MODULE,
	.llseek = tcdtne_llseek,
	.read = tcdtne_read,
	.open = tcdtne_open,
	.release = tcdtne_release,
};

int __init tcdtne_init_module(void)
{
	int rv;
	dev_t dev;

	printk(KERN_INFO "tcdtne: Loading\n");

	rv = alloc_chrdev_region(&dev, 0, 1, "tcdtne");

	if (rv < 0) {
		printk(KERN_WARNING "tcdtne: Failed to alloc_chrdev_region with error code %d\n", rv);
		goto err_alloc_chrdev_region;
	}

	memset(&tcdtne_dev, 0, sizeof(tcdtne_dev));
	cdev_init(&tcdtne_dev.cdev, &tcdtne_fops);
	tcdtne_dev.cdev.owner = THIS_MODULE;

	rv = cdev_add(&tcdtne_dev.cdev, dev, 1);

	if (rv < 0) {
		printk(KERN_WARNING "tcdtne: Failed to cdev_add with error code %d\n", rv);
		goto err_cdev_add;
	}

	tcdtne_dev.class = class_create(THIS_MODULE, "tcdtne");

	if (IS_ERR(tcdtne_dev.class)) {
		printk(KERN_WARNING "tcdtne: Failed to class_create\n");
		rv = PTR_ERR(tcdtne_dev.class);
		goto err_class_create;
	}

	tcdtne_dev.device = device_create(tcdtne_dev.class, NULL, dev, &tcdtne_dev, "%s", "tcdtne");

	if (IS_ERR(tcdtne_dev.device)) {
		printk(KERN_WARNING "tcdtne: Failed to device_create\n");
		rv = PTR_ERR(tcdtne_dev.device);
		goto err_device_create;
	}

	printk(KERN_INFO "tcdtne: Loaded\n");

	return 0;

err_device_create:
	class_destroy(tcdtne_dev.class);

err_class_create:
	cdev_del(&tcdtne_dev.cdev);

err_cdev_add:
	unregister_chrdev_region(dev, 1);

err_alloc_chrdev_region:
	printk(KERN_WARNING "tcdtne: Failed to load\n");
	return rv;
}

void __exit tcdtne_exit_module(void)
{
	dev_t dev = tcdtne_dev.cdev.dev;

	device_destroy(tcdtne_dev.class, dev);
	class_destroy(tcdtne_dev.class);
	cdev_del(&tcdtne_dev.cdev);
	unregister_chrdev_region(dev, 1);

	printk(KERN_INFO "tcdtne: Exiting\n");
}

module_init(tcdtne_init_module);
module_exit(tcdtne_exit_module);
