#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kvm_host.h>
#include <linux/slab.h>

#include "tzvisor.h"

/** ##Backup code## 
 * long sched_penalty = 100 * HZ / 1000;
 * current->state = TASK_UNINTERRUPTIBLE;
 * schedule_timeout(sched_penalty);
 **/

// function pointer in KVM (Linux) hypercal handler
extern unsigned int tzvisor_hypercall_addr;

// convert guest physical address to host physical address
// temporarily not used
unsigned int stage2_gpa_to_hpa(struct kvm *kvm, unsigned int addr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pgd = kvm->arch.pgd + pgd_index(addr);
	pud = pud_offset(pgd, addr);
	pmd = pmd_offset(pud, addr);
	pte = pte_offset_kernel(pmd, addr);

	return (*pte & PAGE_MASK) | (addr & 0xfff);
}

// register a PAL
void pal_registration(struct kvm_vcpu *vcpu, struct kvm *kvm)
{
	// param if from r1 register of VM
	unsigned int param_gpa = vcpu->arch.regs.usr_regs.ARM_r1;
	struct kvm_tzv_file *ktf;
	char *file_cont = NULL;
	unsigned int smc_command = TZV_COMM_REGISTRATION;
	int ret;

	ktf = (struct kvm_tzv_file *)kmalloc(sizeof(struct kvm_tzv_file), GFP_KERNEL);
	if(ktf == NULL)
	{
		printk(KERN_ALERT "[kvm-dev] error: kmalloc failed\n");
		goto out;
	}

	ret = kvm_read_guest(kvm, param_gpa, ktf, sizeof(struct kvm_tzv_file));
	if(ret != 0)
	{
		printk(KERN_ALERT "[kvm-dev] error: kvm_read_guest\n");
		goto out;
	}

	file_cont = (char *)kmalloc(ktf->file_len, GFP_KERNEL);
	ret = kvm_read_guest(kvm, ktf->file_cont_gpa, file_cont, ktf->file_len);
	if(ret != 0)
	{
		printk(KERN_ALERT "[kvm-dev] error: kvm_read_guest\n");
		goto out;
	}

	printk(KERN_ALERT "[kvm-dev] bin: %s\n", file_cont);

	// make secure monitor call
	// r0 is command, r1 is physical address of PAL elf file
	asm volatile(
			".arch_extension sec\n\t"
			"mov r0, %[v]\n\t"
			"mov r1, %[w]\n\t"
			"smc #0\n\t"
			: :[v]"r" (smc_command), [w]"r" (__pa(file_cont)):);

out:
	if(ktf)
		kfree(ktf);
	if(file_cont)
		kfree(file_cont);
}

// unregister a PAL, unimplemented now
void pal_unregister(void)
{
	return;
}

// call a PAL, unimplemented now

void pal_call(void)
{
	return;
}

// hook function
void tzvisor_hypercall_handler(struct kvm_vcpu *vcpu, struct kvm *kvm)
{
	// command in r0 register 
	unsigned int comm  = vcpu->arch.regs.usr_regs.ARM_r0;

	switch(comm)
	{
		case TZV_COMM_REGISTRATION:        // register a PAL
			pal_registration(vcpu, kvm);
			schedule();
			break;

		case TZV_COMM_UNREGISTER:          // unregister a PAL
			pal_unregister();
			break;

		case TZV_COMM_PALCALL:             // call a PAL function
			pal_call();
			break;

		default:
			printk(KERN_ALERT "[kvm-dev] error:unknown command from vm\n");
			return;
	}

	return;
}

static int __init kvm_dev_init(void)
{
	printk(KERN_ALERT "[kvm-dev] init the module\n");
	// register the hook function so that for each hypercall 
	// tzvisor_hypercall_handler will be called.
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
