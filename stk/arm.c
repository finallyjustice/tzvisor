// BSP support routine
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "proc.h"
#include "arm.h"
#include "mmu.h"

void cli (void)
{
    uint val;

    // ok, enable paging using read/modify/write
    asm("MRS %[v], cpsr": [v]"=r" (val)::);
    val |= DIS_INT;
    asm("MSR cpsr_cxsf, %[v]": :[v]"r" (val):);
}

void sti (void)
{
    uint val;

    // ok, enable paging using read/modify/write
    asm("MRS %[v], cpsr": [v]"=r" (val)::);
    val &= ~DIS_INT;
    asm("MSR cpsr_cxsf, %[v]": :[v]"r" (val):);
}

// return the cpsr used for user program
uint spsr_usr ()
{
    uint val;

    // ok, enable paging using read/modify/write
    asm("MRS %[v], cpsr": [v]"=r" (val)::);
    val &= ~MODE_MASK;
    val |= USR_MODE;

    return val;
}

// return whether interrupt is currently enabled
int int_enabled ()
{
    uint val;

    // ok, enable paging using read/modify/write
    asm("MRS %[v], cpsr": [v]"=r" (val)::);

    return !(val & DIS_INT);
}

// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.
void pushcli (void)
{
	int enabled;

	enabled = int_enabled();

	cli();

	////if (cpu->ncli++ == 0) {
	////	cpu->intena = enabled;
	////}
}

void popcli (void)
{
	if (int_enabled()) {
		panic("popcli - interruptible");
	}

	////if (--cpu->ncli < 0) {
	////	cprintf("cpu (%d)->ncli: %d\n", cpu, cpu->ncli);
	////	panic("popcli -- ncli < 0");
	////}

	////if ((cpu->ncli == 0) && cpu->intena) {
		sti();
	////}
}

