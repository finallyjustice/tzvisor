#include "versatile_pb.h"
#include "types.h"
#include "memlayout.h"
#include "uart.h"

#include "semi_loader.h"

void sec_func(void)
{
	cprintf("This is the mmu enabled msg!\n");
}

void kmain(void)
{
	cprintf("Hello kmain!\n");
	
	/*int i;
	for(i=0; i<10; i++)
	{
		asm volatile(
			".arch_extension sec\n\t"
			"smc #0\n\t");
		cprintf("Welcome!!! %d\n", i);
	}*/
	//setup_hyp_vectors();
	afternoon();
	while(1);
}
