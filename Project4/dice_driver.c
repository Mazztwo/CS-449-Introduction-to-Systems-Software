// dice_driver for project4

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/random.h>

#include <asm/uaccess.h>


// Define helper function 
unsigned char get_random_byte(int max)
{
	unsigned char c;
	get_random_bytes(&c,1);
	return c%max;
}

static ssize_t dice_read(struct file * file, char * buf, size_t count, loff_t *ppos)
{	
	char dice;
	dice = (char)get_random_byte(6);

	if (copy_to_user(buf, &dice, (int)1))
		return -EINVAL;

	*ppos = count;

	return count;

}

/*
 * The only file operation we care about is read.
 */

static const struct file_operations dice_fops = {
	.owner		= THIS_MODULE,
	.read		= dice_read,
};

static struct miscdevice dice_driver = {
	/*
	 * We don't care what minor number we end up with, so tell the
	 * kernel to just pick one.
	 */
	MISC_DYNAMIC_MINOR,
	/*
	 * Name ourselves /dev/dice_driver.
	 */
	"dice_driver",
	/*
	 * What functions to call when a program performs file
	 * operations on the device.
	 */
	&dice_fops
};

static int __init dice_init(void)
{
	int ret;
	ret = misc_register(&dice_driver);
	if (ret)
		printk(KERN_ERR "Unable to register dice device\n");

	return ret;
}

module_init(dice_init);

static void __exit dice_exit(void)
{
	misc_deregister(&dice_driver);
}


module_exit(dice_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alessio Mazzone");
MODULE_DESCRIPTION("Dice Driver");
MODULE_VERSION("dev");
