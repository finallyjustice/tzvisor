#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>

static int __init hvc_test_init(void)
{
	char *msg_va;
	unsigned int msg_pa;

	printk(KERN_ALERT "[module] init the module\n");

	msg_va = kmalloc(4096, GFP_KERNEL);
	memcpy(msg_va, "PUPPY", 6);

	msg_pa = __pa(msg_va);

	printk(KERN_ALERT "[module] pass message to STK: 0x%s\n", msg_va);
	printk(KERN_ALERT "[module] gva: 0x%x, gpa: 0x%x\n", (unsigned int)msg_va, msg_pa);

	asm volatile(
			".arch_extension virt\n\t"
			".arch_extension sec\n\t"
			"mov r0, %[v]\n\t"
			"ldr r1, =0x0\n\t"
			"ldr r2, =0x0\n\t"
			"smc #0\n\t"
			: :[v]"r" (msg_pa):) ;

	kfree(msg_va);

	return 0;
}

static void __exit hvc_test_exit(void)
{
	printk(KERN_ALERT "[module] exit the module\n");
}

module_init(hvc_test_init);
module_exit(hvc_test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dongli Zhang");
MODULE_DESCRIPTION("ARM Virtualization hvc Test");
