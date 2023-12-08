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

#define ENTRY_NAME	        "myinfo"
#define DIR_NAME            "mydir"
#define BUFF_SIZE	        PAGE_SIZE
#define STATIC_ARR_SIZE     12

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vincent");

static struct proc_dir_entry *proc_entry = NULL, *dir_entry = NULL;
static char *module_buffer = NULL;
static unsigned char array[STATIC_ARR_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static int myproc_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
    unsigned long pfn;

    if (size > BUFF_SIZE) 
    {
        printk(KERN_ERR "Size exceeds buffer size\n");
        return -EINVAL;
    }
    /* Get phy addr */
    pfn = virt_to_phys(module_buffer) >> PAGE_SHIFT;
    /* Map the physical space */
    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot)) 
    {
        printk(KERN_ERR "Failed to map memory\n");
        return -EAGAIN;
    }
    printk(KERN_INFO "Successfully mmap\n");
    return 0;
}

static struct file_operations fops = {
        .owner  = THIS_MODULE,
        .mmap   = myproc_mmap
};

static int __init init_proc_module(void)
{
    /* Create directory entry */
    dir_entry = proc_mkdir(DIR_NAME, NULL);
    if (dir_entry == NULL) 
    {
        printk(KERN_ERR "Failed to create /proc/%s directory\n", DIR_NAME);
        goto dir_err;
    }
	/* Create the entry named myproc */
	proc_entry = proc_create(ENTRY_NAME, 0666, dir_entry, &fops);
	if (proc_entry == NULL) 
	{
        printk(KERN_INFO "Cannot create /proc/myproc\n");
        goto proc_err;
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
    /* Copy static buffer to allocated page */
    memcpy(module_buffer, array, STATIC_ARR_SIZE);
    printk(KERN_INFO "Init myproc module successfully\n");
	return 0;

mem_err:
	remove_proc_entry(ENTRY_NAME, NULL);
proc_err:
    remove_proc_entry(DIR_NAME, NULL);
dir_err:
    return -ENOMEM;
}

static void __exit cleanup_proc_module(void)
{
	/* Remove the proc entry */
	if (proc_entry) 
	{
        remove_proc_entry(ENTRY_NAME, dir_entry);
		printk(KERN_INFO "/proc/%s/%s removed\n", DIR_NAME, ENTRY_NAME);
    }
    if (dir_entry)
    {
        printk(KERN_INFO "/proc/%s removed\n", DIR_NAME);
        remove_proc_entry(DIR_NAME, NULL);
    }
	/* Free allocated data */
    ClearPageReserved(virt_to_page(module_buffer));
	kfree(module_buffer);
}

module_init(init_proc_module);
module_exit(cleanup_proc_module);