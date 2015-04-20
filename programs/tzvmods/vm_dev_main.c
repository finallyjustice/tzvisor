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

#include "tzvisor.h"

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
	//printk(KERN_ALERT "[vm-dev] vm_dev_open\n");
	return 0;
}

int vm_dev_release(struct inode *inode, struct file *filep)
{
	//printk(KERN_ALERT "[vm-dev] vm_dev_release\n");
	return 0;
}

long vm_dev_ioctl(struct file *filp, unsigned int ioctl, unsigned long arg)
{
	void *argp = (void *)arg;
	//printk(KERN_ALERT "[vm-dev] vm_dev_ioctl\n");

	// ioctl is command
	switch(ioctl)
	{
		case TZV_IOREG:
		{
			char          *file_cont_va = NULL;   // virtual address of elf binary
			unsigned int   file_cont_pa;          // physical address of elf binary
			struct kvm_tzv_file  *vktf  = NULL;   // virtual address of params for hypercall
			unsigned int          pktf;           // physical address of params for hypercall
			int command;                          // command to hypervisor

			// vft in kernel space
			struct vm_tzv_file *vtf =  (struct vm_tzv_file *)argp;
			
			int file_len = vtf->file_len;
			char *elf_virt = (char *)kmalloc(file_len, GFP_KERNEL);  // alloc buf in kernel space
			if(elf_virt == NULL)
			{
				printk(KERN_ALERT "[vm-dev] kalloc failed\n");
				goto ioreg_out;;
			}
			memset(elf_virt, 0, file_len);
			// copy pal elf from user to kernel
			if(copy_from_user(elf_virt, vtf->file_cont, file_len) != 0)
			{
				printk(KERN_ALERT "[vm-dev] failed to copy from user\n");
				goto ioreg_out;
			}
			printk(KERN_ALERT "[vm-dev] bin: %s\n", elf_virt);
			printk(KERN_ALERT "[vm-dev] len: %d\n", file_len);

			// hypercal params
			file_cont_va = elf_virt;
			file_cont_pa = __pa(file_cont_va);

			vktf = (struct kvm_tzv_file *)kmalloc(sizeof(struct kvm_tzv_file), GFP_KERNEL); 
			vktf->file_len = file_len;
			vktf->file_cont_gpa = file_cont_pa;
			pktf = __pa(vktf);

			// make hypercall
			command = TZV_COMM_REGISTRATION;
			asm volatile(
					".arch_extension virt\n\t"
					".arch_extension sec\n\t"
					"mov r0, %[w]\n\t"
					"mov r1, %[v]\n\t"
					"ldr r2, =0x0\n\t"
					"hvc #0\n\t"
					: :[v]"r" (pktf), [w]"r" (command):) ;	

ioreg_out:
			if(file_cont_va)
				kfree(file_cont_va);
			if(vktf)
				kfree(vktf);

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
