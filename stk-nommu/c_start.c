/*
 * Copyright (c) 2012 Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Linaro Limited nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 */

/* This file just contains a small glue function which fishes the
 * location of kernel etc out of linker script defined symbols, and
 * calls semi_loader functions to do the actual work of loading
 * and booting the kernel.
 */

#include <stdint.h>
#include "semihosting.h"
#include "semi_loader.h"

static struct loader_info loader;

volatile unsigned char * const UART0_BASE = (unsigned char *)0x1c090000;

void uart_send(unsigned int c)
{
	*UART0_BASE = c;
	if(c == '\n')
		*UART0_BASE = '\r';
}

void printint(int xx, int base, int sign)
{
	static char digits[] = "0123456789abcdef";
	char buf[16];
	int i;
	unsigned int x;

	if(sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do{
		buf[i++] = digits[x % base];
	}while((x /= base) != 0);

	if(sign)
		buf[i++] = '-';

	while(--i >= 0)
		uart_send(buf[i]);
}

void cprintf(char *fmt, ...)
{
	int i, c;
	unsigned int *argp;
	char *s;

	argp = (unsigned int*)(void*)(&fmt + 1);
	for(i = 0; (c = fmt[i] & 0xff) != 0; i++)
	{
		if(c != '%')
		{
			uart_send(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if(c == 0)
			break;
		switch(c)
		{
		case 'd':
			printint(*argp++, 10, 1);
			break;
		case 'x':
		case 'p':
			printint(*argp++, 16, 0);
			break;
		case 's':
			if((s = (char*)*argp++) == 0)
				s = "(null)";
			for(; *s; s++)
				uart_send(*s);
			break;
		case '%':
			uart_send('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			uart_send('%');
			uart_send(c);
			break;
		}
	}
}

void go_to_kernel(void)
{
	load_kernel(&loader);
	boot_kernel(&loader, 0, -1, loader.fdt_start, 0);
}

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

static inline unsigned long * pud_offset(unsigned long * pgd, unsigned long address)
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

unsigned int hpa_global;		// from smc call r0
unsigned int gpa_global;		// from smc call r1
unsigned int vttbr_low_global;	// from smc call r3
unsigned int vttbr_low_reg;		// read with instruction

void secure_world(void)
{
	while(1)
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
		asm volatile(
			".arch_extension sec\n\t"
			"smc #0\n\t");
	};
}

void bootmain(void)
{
	monitorInit(go_to_kernel);	
	while(1);
}
