#include "types.h"
#include "mmu.h"
#include "versatile_pb.h"
#include "memlayout.h"

#include "semihosting.h"
#include "semi_loader.h"

unsigned int hpa_global;        // from smc call r0
unsigned int gpa_global;        // from smc call r1
unsigned int vttbr_low_global;  // from smc call r3
unsigned int vttbr_low_reg;     // read with instruction

extern void kmain(void);
extern void jump_stack();

// kernel page table, reserved in the kernel.ld
extern uint32   _kernel_pgtbl;
extern uint32   _user_pgtbl;

uint32 *kernel_pgtbl = &_kernel_pgtbl;
uint32 *user_pgtbl = &_user_pgtbl;

struct loader_info loader;

void early_uart_send(unsigned int c)
{
	__REG(UART0) = c;
	if(c == '\n')
		__REG(UART0) = '\r';
}

void early_printstr(char *fmt)
{
	int i, c;
	char *s;
	
	for(i = 0; (c = fmt[i] & 0xff) != 0; i++)
	{
		if(c != '%')
		{
			early_uart_send(c);
			continue;
		}
		
		c = fmt[++i] & 0xff;
		if(c == 0)
			break;
	}
}

// setup the boot page table: dev_mem whether it is device memory
void set_bootpgtbl (uint32 virt, uint32 phy, uint len, int dev_mem )
{
	uint32  pde;
	int     idx;

	// convert all the parameters to indexes
	virt >>= PDE_SHIFT;
	phy  >>= PDE_SHIFT;
	len  >>= PDE_SHIFT;

	for (idx = 0; idx < len; idx++)
	{
		pde = (phy << PDE_SHIFT);

		if (!dev_mem) {
			// normal memory, make it kernel-only, cachable, bufferable
			pde |= (AP_KO << 10) | PE_CACHE | PE_BUF | KPDE_TYPE;
		} else {
			// device memory, make it non-cachable and non-bufferable
			pde |= (AP_KO << 10) | KPDE_TYPE;
		}

		// use different page table for user/kernel space
		if (virt < NUM_UPDE) {
			user_pgtbl[virt] = pde;
		} else {
			kernel_pgtbl[virt] = pde;
		}

		virt++;
		phy++;
	}
}

void _flush_all (void)
{
	uint val = 0;

	// flush all TLB
	asm("MCR p15, 0, %[r], c8, c7, 0" : :[r]"r" (val):);

	// invalid entire data and instruction cache
	//asm ("MCR p15,0,%[r],c7,c10,0": :[r]"r" (val):);
	//asm ("MCR p15,0,%[r],c7,c11,0": :[r]"r" (val):);
}

void load_pgtlb (uint32* kern_pgtbl, uint32* user_pgtbl)
{
	uint    val;

	// set domain access control: all domain will be checked for permission
	val = 0x55555555;
	asm("MCR p15, 0, %[v], c3, c0, 0": :[v]"r" (val):);

	// set the page table base registers. We use two page tables: TTBR0
	// for user space and TTBR1 for kernel space
	// set the TTBCR (Translation Table Base Control Register)
	val = 32 - UADDR_BITS;
	asm("MCR p15, 0, %[v], c2, c0, 2": :[v]"r" (val):);

	// set the kernel page table
	val = (uint)kernel_pgtbl | 0x00;
	asm("MCR p15, 0, %[v], c2, c0, 1": :[v]"r" (val):);

	// set the user page table
	//val = (uint)user_pgtbl | 0x00;
	//asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (val):);

	// ok, enable paging using read/modify/write
	// SCTLR (System Control Register), last bit is MMU
	asm("MRC p15, 0, %[r], c1, c0, 0": [r]"=r" (val)::);

	val |= 0x80300D; // enable MMU, cache, write buffer, high vector tbl,
					 // disable subpage
	asm("MCR p15, 0, %[r], c1, c0, 0": :[r]"r" (val):);
	
	_flush_all();
}

void go_to_kernel(void)
{
	early_printstr("[TZV] Boot Linux...\n");
	boot_kernel(&loader, 0, -1, loader.fdt_start, 0);
	
	early_printstr("[TZV] We should never come here!\n");
	while(1);
}

void secure_world(void)
{
	while(1)
	{
		sec_func();
		asm volatile(
			".arch_extension sec\n\t"
			"smc #0\n\t");
	}
}

void boot_linux(void)
{
	monitorInit(go_to_kernel);
}

void bootmain(void)
{
	load_kernel(&loader);
	setup_hyp_vectors();
	
	set_bootpgtbl(PHY_START, PHY_START, INIT_KERN_SZ, 0);
	set_bootpgtbl(0x80000000, 0x80000000, 0x80000000, 0); // map the whole 2GB physical memory
														  // virtual address space is the same as physical address space

    set_bootpgtbl(DEVBASE1, DEVBASE1, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV: add to prevent crash on _puts
    set_bootpgtbl(KERNBASE+DEVBASE1, DEVBASE1, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV
    set_bootpgtbl(KERNBASE+DEVBASE2, DEVBASE2, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV

    load_pgtlb (kernel_pgtbl, user_pgtbl);
    jump_stack ();
    
    // We can now call normal kernel functions at high memory
    //clear_bss ();
	
	early_printstr("[TZV] Finish MMU\n");
	
	kmain();	
}
