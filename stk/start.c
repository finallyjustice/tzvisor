#include "types.h"
#include "mmu.h"
#include "versatile_pb.h"
#include "memlayout.h"

extern void kmain(void);
extern void jump_stack();

void early_uart_send(unsigned int c)
{
	__REG(UART0) = c;
	if(c == '\n')
		__REG(UART0) = '\r';
}

void early_printstr(char *s)
{
	int i = 0;
	while(s[i])
	{
		early_uart_send(s[i]);
		i++;
	}
}

// kernel page table, reserved in the kernel.ld
extern uint32   _kernel_pgtbl;
extern uint32   _user_pgtbl;

uint32 *kernel_pgtbl = &_kernel_pgtbl;
uint32 *user_pgtbl = &_user_pgtbl;

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
	val = 32 - UADDR_BITS;
	asm("MCR p15, 0, %[v], c2, c0, 2": :[v]"r" (val):);

	// set the kernel page table
	val = (uint)kernel_pgtbl | 0x00;
	asm("MCR p15, 0, %[v], c2, c0, 1": :[v]"r" (val):);

	// set the user page table
	val = (uint)user_pgtbl | 0x00;
	asm("MCR p15, 0, %[v], c2, c0, 0": :[v]"r" (val):);

	// ok, enable paging using read/modify/write
	asm("MRC p15, 0, %[r], c1, c0, 0": [r]"=r" (val)::);

	val |= 0x80300D; // enable MMU, cache, write buffer, high vector tbl,
					 // disable subpage
	asm("MCR p15, 0, %[r], c1, c0, 0": :[r]"r" (val):);
	
	_flush_all();
}

void bootmain(void)
{
	early_printstr("hello1\n");

	uint32  vectbl;

    // double map the low memory, required to enable paging
    // we do not map all the physical memory
    set_bootpgtbl(PHY_START, PHY_START, INIT_KERN_SZ, 0);
    set_bootpgtbl(KERNBASE+PHY_START, PHY_START, INIT_KERN_SZ, 0);

    // vector table is in the middle of first 1MB (0xF000)
    /////////vectbl = P2V_WO ((VEC_TBL & PDE_MASK) + PHY_START);

    /*if (vectbl <= (uint)&end) {
        _puts("error: vector table overlap and cprintf() is 0x00000\n");
        cprintf ("error: vector table overlaps kernel\n");
    }*/
    // V, P, len, is_mem
    /////////set_bootpgtbl(VEC_TBL, PHY_START, 1 << PDE_SHIFT, 0); // V, P, SZ, ISDEV
    set_bootpgtbl(DEVBASE1, DEVBASE1, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV: add to prevent crash on _puts
    set_bootpgtbl(KERNBASE+DEVBASE1, DEVBASE1, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV
    set_bootpgtbl(KERNBASE+DEVBASE2, DEVBASE2, DEV_MEM_SZ, 1); // V, P, SZ, ISDEV

    load_pgtlb (kernel_pgtbl, user_pgtbl);
    jump_stack ();
    
    // We can now call normal kernel functions at high memory
    //clear_bss ();
	
	__REG(P2V(UART0)) = 'A';	
	early_printstr("hello2\n");
	kmain();
}
