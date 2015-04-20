#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "proc.h"
#include "arm.h"
#include "mmu.h"

// disable irq
void cli (void)
{
    uint val;

    // ok, enable paging using read/modify/write
    asm("MRS %[v], cpsr": [v]"=r" (val)::);
    val |= DIS_INT;
    asm("MSR cpsr_cxsf, %[v]": :[v]"r" (val):);
}

// enable irq
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

// Pushcli/popcli are like cli/sti except that they are matched
void pushcli (void)
{
	int enabled;
	enabled = int_enabled();
	cli();
}

void popcli (void)
{
	if (int_enabled()) {
		panic("popcli - interruptible");
	}
	sti();
}

