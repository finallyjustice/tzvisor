#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kvm_host.h>

/** ##Backup code## 
 * long sched_penalty = 100 * HZ / 1000;
 * current->state = TASK_UNINTERRUPTIBLE;
 * schedule_timeout(sched_penalty);
 **/

extern unsigned int tzvisor_hypercall_addr;

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

void tzvisor_hypercall_handler(struct kvm_vcpu *vcpu, struct kvm *kvm)
{
	unsigned int *vbase;
	unsigned int pgd;
	unsigned int hpa;
	unsigned int gpa;

	printk(KERN_ALERT "[kvm-dev] tzvisor_hypercall_handler\n");

	pgd = (unsigned int)kvm->arch.pgd;
	vbase = phys_to_virt(kvm->arch.pgd);

	gpa = vcpu->arch.regs.usr_regs.ARM_r0;
	hpa = stage2_gpa_to_hpa(kvm, gpa);
	printk(KERN_ALERT "[kvm-dev] hpa: 0x%x\n", hpa);

	asm volatile(
			".arch_extension sec\n\t"
			"mov r0, %[v]\n\t"
			"mov r1, %[w]\n\t"
			"mov r2, %[x]\n\t"
			"smc #0\n\t"
			: :[v]"r" (gpa), [w]"r" (hpa), [x]"r" (pgd):);
	
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
