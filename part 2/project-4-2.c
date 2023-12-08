#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>

#define ENTRY_NAME	"myproc"
#define BUFF_SIZE	PAGE_SIZE

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vincent");

static struct proc_dir_entry *proc_entry = NULL;
static char *module_buffer = NULL;

static ssize_t read_proc(struct file *f, char *user_buf, size_t count, loff_t *off)
{
	/* Check if EOF */
	if (*off >= BUFF_SIZE) 
		return 0;
	/* Adjust count if it goes beyond the end of the buffer */ 
	if (*off + count > BUFF_SIZE)
		count = BUFF_SIZE - *off;
	/* Copy data */
	if (copy_to_user(user_buf, &module_buffer[*off], count) != 0)
		return -EFAULT;
	/* Update offset */
	*off += count;
	return count;
}

static ssize_t write_proc(struct file *f, const char *user_buf, size_t count, loff_t *off)
{
	/* Check if EOF */
	if (*off >= BUFF_SIZE) 
		return 0;
	/* Adjust count if it goes beyond the end of the buffer */ 
	if (*off + count > BUFF_SIZE)
		count = BUFF_SIZE - *off;
	/* Copy data */
	if (copy_from_user(&module_buffer[*off], user_buf, count) != 0)
		return -EFAULT;
	/* Update offset */
	*off += count;
	return count;
}

static struct file_operations fops = {
        .owner    = THIS_MODULE,
        .read     = read_proc,
        .write    = write_proc,
};

static int __init init_proc_module(void)
{
	int ret = 0;
	/* create the entry named myproc */
	proc_entry = proc_create(ENTRY_NAME, 0666, NULL, &fops);
	if (proc_entry == NULL) 
	{
        printk(KERN_INFO "Cannot create /proc/myproc\n");
        return -ENOMEM;
    }
	printk(KERN_INFO "/proc/myproc created.\n");
	/* Allocated memory space for the proc entry */
	module_buffer = kmalloc(BUFF_SIZE, GFP_KERNEL);
	if (module_buffer == NULL)
	{
		printk(KERN_INFO "Cannot allocated memory\n");
		goto mem_err;
	}
	SetPageReserved(virt_to_page(module_buffer));
	memset(module_buffer, 0, BUFF_SIZE);
	return 0;

mem_err:
	remove_proc_entry(ENTRY_NAME, NULL);
	ret = -ENOMEM;

	return ret;
}

static void __exit cleanup_proc_module(void)
{
	/* Remove the proc entry */
	if (proc_entry) 
	{
        remove_proc_entry(ENTRY_NAME, NULL);
		printk(KERN_INFO "/proc/myproc removed\n");
    }
	/* Free allocated data */
	ClearPageReserved(virt_to_page(module_buffer));
	kfree(module_buffer);
}

module_init(init_proc_module);
module_exit(cleanup_proc_module);