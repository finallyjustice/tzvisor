#include "versatile_pb.h"
#include "types.h"
#include "memlayout.h"
#include "uart.h"
#include "mmu.h"
#include "proc.h"

#include "semi_loader.h"

extern void* end;

struct cpu  cpus[NCPU];
struct cpu  *cpu;

char *param_base;

//extern unsigned int hpa_global;        // from smc call r0
//extern unsigned int gpa_global;        // from smc call r1
//extern unsigned int vttbr_low_global;  // from smc call r3
//extern unsigned int vttbr_low_reg;     // read with instruction

extern unsigned int smc_command;
extern unsigned int smc_param;

#define PGDIR_SHIFT    30
#define PMD_SHIFT      21
#define PTRS_PER_PMD   512
#define PTRS_PER_PTE   512
#define PAGE_MASK      0xfffff000
#define ADDR_MASK      0x00000fff
#define PAGE_SHIFT     12
#define pgd_index(addr)   ((addr) >> PGDIR_SHIFT)
#define pmd_index(addr)   (((addr) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))
#define pte_index(addr)   (((addr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

static inline unsigned long *pud_offset(unsigned long * pgd, unsigned long address)
{
	return (unsigned long *)pgd;
}

static inline unsigned long *pud_page_paddr(unsigned long pud)
{
	return (unsigned long *)(pud & (unsigned long)PAGE_MASK);
}

static inline unsigned long *pmd_page_vaddr(unsigned long pmd)
{
	return (unsigned long *)(pmd & (unsigned long)PAGE_MASK);
}

static inline unsigned long *pmd_offset(unsigned long *pud, unsigned long addr)
{
	return (unsigned long *)pud_page_paddr(*pud) + pmd_index(addr)*2;
}

static inline unsigned long *pte_offset_kernel(unsigned long *pmd, unsigned long addr)
{
	return (unsigned long *)pmd_page_vaddr(*pmd) + pte_index(addr)*2;
}

unsigned long gpa_to_hpa(unsigned long gpa, unsigned long vttbr)
{
	unsigned long *pgd;
	unsigned long *pud;
	unsigned long *pmd;
	unsigned long *pte;

	pgd = (unsigned long *)vttbr + pgd_index(gpa)*2;
	pud = pud_offset(pgd, gpa);
	pmd = pmd_offset(pud, gpa);
	pte = pte_offset_kernel(pmd, gpa);

	return (*pte & PAGE_MASK) | (gpa & ADDR_MASK);
}

void exec_pal(unsigned int addr)
{
	exec((char *)addr);
}

void sec_func(void)
{
	cprintf("[TZV] This is secure world\n");

	//cprintf("[TZV] hpa_global:       0x%x\n", hpa_global);
	//cprintf("[TZV] gpa_global:       0x%x\n", gpa_global);
	//cprintf("[TZV] vttbr_low_global: 0x%x\n", vttbr_low_global);
	//cprintf("[TZV] vttbr_low_reg:    0x%x\n", vttbr_low_reg);

	cprintf("[TZV] smc_command: 0x%x\n", smc_command);
	cprintf("[TZV] smc_param  : 0x%x\n", smc_param);

	char *param_base = (char *)smc_param; 
	exec_pal(param_base);
	
	/* backup */
	//userinit();
	//scheduler();
}

void kmain(void)
{
	cprintf("[TZV] Starting kernel...\n");

	// we currently only support 1 cpu core
	cpu = &cpus[0];

	init_vmm ();
	kpt_freerange (align_up(&end, PT_SZ), INIT_KERNMAP);
	kmem_init2((void *)INIT_KERNMAP, (void *)PHYSTOP);	

	trap_init();

	//userinit();
	//scheduler();

	cprintf("[TZV] Booting Linux now...\n");
	boot_linux();
	panic("[TZV] We should never come here!");	
}
