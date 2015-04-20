#include "versatile_pb.h"
#include "types.h"
#include "memlayout.h"
#include "uart.h"

#include "semi_loader.h"

extern unsigned int hpa_global;        // from smc call r0
extern unsigned int gpa_global;        // from smc call r1
extern unsigned int vttbr_low_global;  // from smc call r3
extern unsigned int vttbr_low_reg;     // read with instruction

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

void sec_func(void)
{
	cprintf("[TZV] This is secure world\n");

	cprintf("[TZV] hpa_global:       0x%x\n", hpa_global);
	cprintf("[TZV] gpa_global:       0x%x\n", gpa_global);
	cprintf("[TZV] vttbr_low_global: 0x%x\n", vttbr_low_global);
	cprintf("[TZV] vttbr_low_reg:    0x%x\n", vttbr_low_reg);

	if(vttbr_low_global == 0x0)
		vttbr_low_global = vttbr_low_reg;

	char *data1 = (char *)hpa_global;
	char *data2 = (char *)gpa_to_hpa(gpa_global, vttbr_low_global);
	cprintf("[TZV] data: %s, %s\n", data1, data2);
}

void kmain(void)
{
	boot_linux();
	while(1);
}
