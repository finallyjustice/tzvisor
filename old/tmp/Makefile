# Makefile - build a kernel+filesystem image for stand-alone Linux booting
#
# Copyright (C) 2011 ARM Limited. All rights reserved.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.txt file.


# Include config file (prefer config.mk, fall back to config-default.mk)
ifneq ($(wildcard config.mk),)
include config.mk
else
include config-default.mk
endif

LIBFDTOBJS      = libfdt/fdt.o libfdt/fdt_ro.o libfdt/fdt_wip.o \
				  libfdt/fdt_sw.o libfdt/fdt_rw.o libfdt/fdt_strerror.o
BOOTLOADER	= entry.S
OBJS 		= entry.o start.o semihosting.o string.o semi_loader.o $(LIBFDTOBJS) \
			  main.o uart.o raise.o asm.o \
			  vm.o buddy.o proc.o \
			  trap_asm.o trap.o console.o arm.o\
			  lib/string.o
KERNEL		= uImage

IMAGE		= linux-system.axf
SEMIIMG 	= linux-system-semi.axf
LD_SCRIPT	= model.lds.S


CC		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)ld
OBJDUMP    = $(CROSS_COMPILE)objdump
OBJCOPY    = $(CROSS_COMPILE)objcopy

# These are needed by the underlying kernel make
export CROSS_COMPILE ARCH

# Build all wrappers
#all: $(IMAGE) $(SEMIIMG)
all: $(SEMIIMG)

# Build just the semihosting wrapper
semi: $(SEMIIMG)

clean distclean:
	rm -f $(IMAGE) $(SEMIIMG) \
	model.lds modelsemi.lds $(OBJS) $(KERNEL) *.asm *.sym \
	initcode initcode.out initcode.o

$(KERNEL): $(KERNEL_SRC)/arch/arm/boot/uImage
	cp $< $@

$(IMAGE): $(OBJS) model.lds $(KERNEL) $(FILESYSTEM) Makefile
	$(LD) -o $@ $(OBJS) libgcc.a --script=model.lds
	$(OBJDUMP) -D linux-system.axf > linux-system.asm
	$(OBJDUMP) -t linux-system.axf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > linux-system.sym

#$(SEMIIMG): $(OBJS) modelsemi.lds
#	$(LD) -o $@ $(OBJS) libgcc.a --script=modelsemi.lds
#	$(OBJDUMP) -D linux-system-semi.axf > linux-system-semi.asm
#	$(OBJDUMP) -t linux-system-semi.axf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > linux-system-semi.sym

initcode: initcode.S
	$(CC) -c -o initcode.o initcode.S -nostdinc -I. 
	$(LD) initcode.o -o initcode.out -N -e start -Ttext 0
	$(OBJCOPY) -S -O binary --prefix-symbol="_binary_initcode" initcode.out initcode

$(SEMIIMG): $(OBJS) model.lds.S initcode
	$(LD) -o $@ $(OBJS) libgcc.a -b binary initcode --script=model.lds.S
	$(OBJDUMP) -D linux-system-semi.axf > linux-system-semi.asm
	$(OBJDUMP) -t linux-system-semi.axf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > linux-system-semi.sym

boot.o: $(BOOTLOADER)
	$(CC) $(CPPFLAGS) -DKCMD='$(KCMD)' -c -o $@ $<

%.o: %.c
	$(CC) $(CPPFLAGS) -O2 -ffreestanding -I. -Ilibfdt -c -o $@ $<

model.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -E -P -C -o $@ $<

modelsemi.lds: $(LD_SCRIPT) Makefile
	$(CC) $(CPPFLAGS) -DSEMIHOSTING=1 -E -P -C -o $@ $<

$(KERNEL_SRC)/arch/arm/boot/uImage: force
	$(MAKE) -C $(KERNEL_SRC) -j4 uImage

# Pass any target we don't know about through to the kernel makefile.
# This is a convenience rule so we can say 'make menuconfig' etc here.
# Note that any rules in this file must have a command or be marked as
# .PHONY.
%: force
	$(MAKE) -C $(KERNEL_SRC) $@

force: ;

Makefile: ;

.PHONY: all semi clean distclean config.mk config-default.mk
