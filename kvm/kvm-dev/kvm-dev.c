#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>

extern unsigned int tzvisor_hypercall_addr;

long sched_penalty = 100 * HZ / 1000;
	
void tzvisor_hypercall_handler(void)
{
	printk(KERN_ALERT "[kvm-dev] tzvisor_hypercall_handler\n");

	asm volatile(
			".arch_extension sec\n\t"
			"smc #0\n\t");

	current->state = TASK_UNINTERRUPTIBLE;
	schedule_timeout(sched_penalty);
}

static int __init kvm_dev_init(void)
{
	printk(KERN_ALERT "[kvm-dev] init the module\n");
	tzvisor_hypercall_addr = (unsigned int)tzvisor_hypercall_handler;
	return 0;
}
static void __exit kvm_dev_exit(void)
{
	printk(KERN_ALERT "[kvm-dev] exit the module\n");
}

module_init(kvm_dev_init);
module_exit(kvm_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("TZVisor Driver for ARM/KVM");
