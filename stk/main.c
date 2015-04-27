#include "versatile_pb.h"
#include "types.h"
#include "memlayout.h"
#include "uart.h"
#include "mmu.h"
#include "proc.h"

#include "semi_loader.h"

// end of stk in memory (model.lds.S)
extern void* end;

// we assume only one CPU at this time
struct cpu  cpus[NCPU];
struct cpu  *cpu;

char *param_base;

extern unsigned int smc_command;  // command for smc call - r0
extern unsigned int smc_param;    // first param for smc call - r1

/* The code to translate virt addr to phys addr
   is currently not used 
*/
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

// convert a guest physical address in VM to host physical address in KVM
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

// register a pal file at address addr
void exec_pal(unsigned int addr)
{
	exec((char *)addr);
}

void sec_func(void)
{
	extern int test_once;
	test_once = 0;
	cprintf("[TZV] This is secure world\n");
	volatile uint * timer1 = P2V_DEV(TIMER1);
	timer1[TIMER_CONTROL] = TIMER_EN|TIMER_PERIODIC|TIMER_32BIT|TIMER_INTEN;
	//set_timer1_secure();
	unsigned int *iccicr_base = (unsigned int *)P2V_DEV(0x2c002000);
	unsigned int orin = *iccicr_base;
	*iccicr_base &= 0x1;
	sti();

	cprintf("[TZV] In timer\n");
	while(1)
	{
		sti();
		if(test_once > 5)
			break;
	}

	cli();
	*iccicr_base = orin;
	timer1[TIMER_CONTROL] = TIMER_EN|TIMER_PERIODIC|TIMER_32BIT;
	

	//while(1);
	//char *param_base = (char *)smc_param; 
	//exec_pal(param_base);
	
	/* backup */
	//userinit();
	//scheduler();
}

void kmain(void)
{
	cprintf("[TZV] Starting kernel...\n");

	// we currently only support 1 cpu core
	cpu = &cpus[0];

	// init memory management in STK
	init_vmm ();
	kpt_freerange (align_up(&end, PT_SZ), INIT_KERNMAP);
	// init buddy algorithm
	kmem_init2((void *)INIT_KERNMAP, (void *)PHYSTOP);	

	// init vector table (page fault, system call, irq, etc)
	trap_init();
	set_timer1_secure();
	gic_init(P2V_DEV(VIC_BASE));
	// we use TIMER1 instead of TIMER0
	timer1_init(100);
	//sti();

	unsigned int *iccicr_base = (unsigned int *)P2V_DEV(0x2c002000);
	*iccicr_base &= 0x1;

	cprintf("[TZV] Booting Linux now...\n");
	//while(1);
	boot_linux();

	// kernel will never return here
	panic("[TZV] We should never come here!");	
}
