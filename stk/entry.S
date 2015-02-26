	.syntax	unified
	.arch_extension sec
	.arch_extension virt
	.text

	.globl	start
start:
	@
	@ Program architected timer frequency
	@
	mrc	p15, 0, r0, c0, c1, 1		@ CPUID_EXT_PFR1
	lsr	r0, r0, #16
	and	r0, r0, #1			@ Check generic timer support
	beq	1f
	ldr	r0, =24000000			@ 24MHz timer frequency
	mcr	p15, 0, r0, c14, c0, 0		@ CNTFRQ
1:
	@
	@ CPU initialisation
	@
	mrc	p15, 0, r4, c0, c0, 5		@ MPIDR (ARMv7 only)
	and	r4, r4, #15			@ CPU number

2:
	@
	@ UART initialisation (38400 8N1)
	@
	ldr	r0, =0x1c090000			@ UART base (Versatile Express)
	mov	r1, #0x10			@ ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			@ cr
	str	r1, [r0, #0x30]

	@ Now we've got rid of the secondary CPUs, set up a stack
	@ for CPU 0 so we can write most of this in C.
	#ldr     sp, =stacktop
	ldr     sp, =svc_stktop

	@ And call the C entrypoint
	bl      bootmain
	@ Never reached
1:	b 1b

#@
#@ Function for C code to make semihosting calls:
#@
#.globl __semi_call
#__semi_call:
#	svc     0x123456
#	mov pc, lr

# during startup, kernel stack uses user address, now switch it to kernel addr
.global jump_stack
jump_stack:
	MOV     r0, sp
	ADD     r0, r0, #0x10000000
	MOV     sp, r0
	MOV     pc, lr
