/*
 * model.lds.S - simple linker script for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
TARGET(binary)

PHYS_OFFSET = 0xf0000000;
ENTRY_SVC_STACK_SIZE = 0x1000;

SECTIONS
{
	. = 0xf0010000;

	.start_sec : {
		entry.o(.text)
		entry.o(.init)
		start.o(.text .text.*)
		semihosting.o(.text .text.*)
		string.o(.text .text.*)
		semi_loader.o(.text .text.*)
		libfdt/fdt.o(.text .text.*)
		libfdt/fdt_ro.o(.text .text.*)
		libfdt/fdt_wip.o(.text .text.*)
		libfdt/fdt_sw.o(.text .text.*)
		libfdt/fdt_rw.o(.text .text.*)
		libfdt/fdt_strerror.o(.text .text.*)

		entry.o(.rodata .rodata.*)
		start.o(.rodata .rodata.*)
		semihosting.o(.rodata .rodata.*)
		string.o(.rodata .rodata.*)
		semi_loader.o(.rodata .rodata.*)
		libfdt/fdt.o(.rodata .rodata.*)
		libfdt/fdt_ro.o(.rodata .rodata.*)
		libfdt/fdt_wip.o(.rodata .rodata.*)
		libfdt/fdt_sw.o(.rodata .rodata.*)
		libfdt/fdt_rw.o(.rodata .rodata.*)
		libfdt/fdt_strerror.o(.rodata .rodata.*)
		
		entry.o(.data .data.*)
		start.o(.data .data.*)
		semihosting.o(.data .data.*)
		string.o(.data .data.*)
		semi_loader.o(.data .data.*)
		libfdt/fdt.o(.data .data.*)
		libfdt/fdt_ro.o(.data .data.*)
		libfdt/fdt_wip.o(.data .data.*)
		libfdt/fdt_sw.o(.data .data.*)
		libfdt/fdt_rw.o(.data .data.*)
		libfdt/fdt_strerror.o(.data .data.*)
		
		PROVIDE(edata_entry = .);
		 
		entry.o(.bss .bss.* COMMON)
		start.o(.bss .bss.* COMMON)
		semihosting.o(.bss .bss.* COMMON)
		string.o(.bss .bss.* COMMON)
		semi_loader.o(.bss .bss.* COMMON)
		libfdt/fdt.o(.bss .bss.* COMMON)
		libfdt/fdt_ro.o(.bss .bss.* COMMON)
		libfdt/fdt_wip.o(.bss .bss.* COMMON)
		libfdt/fdt_sw.o(.bss .bss.* COMMON)
		libfdt/fdt_rw.o(.bss .bss.* COMMON)
		libfdt/fdt_strerror.o(.bss .bss.* COMMON)

		/*define a stack for the entry*/
		. = ALIGN(0x2000);
		. += ENTRY_SVC_STACK_SIZE;

		PROVIDE (svc_stktop = .);

		/* define the kernel page table, must be 16K and 16K-aligned*/
		. = ALIGN(0x4000);
		PROVIDE (_kernel_pgtbl = .);
		. += 0x8000;

		/* we also need a user page table*/
		PROVIDE (_user_pgtbl = .);
		. += 0x2000;

		PROVIDE(end_entry = .);
	}

	. = 0xf0080000;
	
	.text : AT(0xf0080000){
		*(.text .text.* .gnu.linkonce.t.*)
	}

	PROVIDE(etext = .);   /* Define the 'etext' symbol to this value */

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	/* aligned the data to a (4K) page, so it can be assigned
	   different protection than the code*/
	. = ALIGN(0x1000);

	PROVIDE (data_start = .);
	
	.data : {
		*(.data .data.*)
	}

	PROVIDE (edata = .);

	.bss : {
		*(.bss .bss.* COMMON)
	}
	
	. = ALIGN(0x1000);
	PROVIDE (end = .);
}
