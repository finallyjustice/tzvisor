#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include <asm/current.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

#include "ioctl-io.h"

struct cdev cdev;
int dev_major = 50;
int dev_minor = 0;

/** ## Backup Code ##
 * long mytimeout = 100 * HZ / 1000;
 * current->state = TASK_UNINTERRUPTIBLE;
 * schedule_timeout(mytimeout);
 **/

int vm_dev_open(struct inode *inode, struct file *filep)
{
	printk(KERN_ALERT "[vm-dev] vm_dev_open\n");
	return 0;
}

int vm_dev_release(struct inode *inode, struct file *filep)
{
	printk(KERN_ALERT "[vm-dev] vm_dev_release\n");
	return 0;
}

long vm_dev_ioctl(struct file *filp, unsigned int ioctl, unsigned long arg)
{
	void *argp = (void *)arg;
	printk(KERN_ALERT "[vm-dev] vm_dev_ioctl\n");
	
	switch(ioctl)
	{
		case TZV_IOREG:
		{
			char *msg_va;
			unsigned int msg_pa;

			struct tzv_file_info *tzv =  (struct tzv_file_info *)argp;
			int len = tzv->len;
			char *bin = kmalloc(4096, GFP_KERNEL);
			copy_from_user(bin, tzv->bin, len);
			printk(KERN_ALERT "[vm-dev] bin: %s\n", bin);

			// TrustZone
			msg_va = bin;
			msg_pa = __pa(msg_va);

			asm volatile(
					".arch_extension virt\n\t"
					".arch_extension sec\n\t"
					"mov r0, %[v]\n\t"
					"ldr r1, =0x0\n\t"
					"ldr r2, =0x0\n\t"
					"smc #0\n\t"
					: :[v]"r" (msg_pa):) ;	

			kfree(msg_va);

			break;
		}
		case TZV_IOCALL:
		{
			printk(KERN_ALERT "[vm-dev] command: Call\n");
			break;
		}
		case TZV_IOUNREG:
		{
			printk(KERN_ALERT "[vm-dev] command: UnRegistration\n");
			break;
		}
		default:
		{
			printk(KERN_ALERT "[vm-dev] command: Unknown\n");
		}
	}

	return 0;
}

struct file_operations vm_dev_fops = {
	owner:           THIS_MODULE, 
	open:            vm_dev_open,    
	release:         vm_dev_release,
	unlocked_ioctl : vm_dev_ioctl,
};

int cdev_vm_dev_install(void)
{
	int ret;
	dev_t devno = MKDEV(dev_major, dev_minor);

	if(dev_major)  // assign static dev number
	{
		ret = register_chrdev_region(devno, 2, "tzvisor-vm");
	}
	else  // assign dynamic dev number
	{
		ret = alloc_chrdev_region(&devno, 0, 2, "tzvisor-vm");
		dev_major = MAJOR(devno);
	}

	if(ret < 0)
	{
		printk(KERN_ALERT "[vm-dev] /dev/tzvisor register failed\n");
		return ret;
	}
	else
	{
		printk(KERN_ALERT "[vm-dev] /dev/tzvisor register successfully\n");
	}

	// init the cdev
	cdev_init(&cdev, &vm_dev_fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &vm_dev_fops;
	// register the char device
	cdev_add(&cdev, MKDEV(dev_major, 0), 1);

	return 0;
}

void cdev_vm_dev_uninstall(void)
{
	cdev_del(&cdev);
	unregister_chrdev_region(MKDEV(dev_major, 0), 1);
}

static int __init vm_dev_init(void)
{
	int ret;
	printk(KERN_ALERT "[vm-dev] init the module\n");
	ret = cdev_vm_dev_install();
	return ret;
}
static void __exit vm_dev_exit(void)
{
	printk(KERN_ALERT "[vm-dev] exit the module\n");
	cdev_vm_dev_uninstall();
}

module_init(vm_dev_init);
module_exit(vm_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("DEV driver for TrustZone");
