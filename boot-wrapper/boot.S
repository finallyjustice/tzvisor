/*
 * boot.S - simple register setup code for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */

.equ Mode_USR, 0x10
.equ Mode_FIQ, 0x11
.equ Mode_IRQ, 0x12
.equ Mode_SVC, 0x13
.equ Mode_ABT, 0x17
.equ Mode_UND, 0x1B
.equ Mode_SYS, 0x1F
.equ Mode_MON, 0x16
.equ I_Bit,    0x80
.equ F_Bit,    0x40

.equ Len_FIQ_Stack,  64
.equ Len_IRQ_Stack,  64
.equ Len_ABT_Stack,  64
.equ Len_UND_Stack,  64
.equ Len_SVC_Stack,  512
.equ Len_USR_Stack,  512

.equ Offset_FIQ_Stack, 0
.equ Offset_IRQ_Stack, Offset_FIQ_Stack + Len_FIQ_Stack
.equ Offset_ABT_Stack, Offset_IRQ_Stack + Len_IRQ_Stack
.equ Offset_UND_Stack, Offset_ABT_Stack + Len_ABT_Stack
.equ Offset_SVC_Stack, Offset_UND_Stack + Len_UND_Stack
.equ Offset_USR_Stack, Offset_SVC_Stack + Len_SVC_Stack

	.syntax	unified
	.arch_extension sec
	.arch_extension virt
	.text

.macro enter_hyp
	@ We assume we're entered in Secure Supervisor mode. To
	@ get to Hyp mode we have to pass through Monitor mode
	@ and NS-Supervisor mode. Note that there is no way to
	@ return to the Secure world once we've done this.
	@
	@ This will trash r10 and r11.
	ldr	r10, =vectors
	mcr	p15, 0, r10, c12, c0, 1		@ Monitor vector base address
	@ Switch to monitor mode, which will set up the HVBAR and
	@ then return to us in NS-SVC
	smc	#0
	
	@ get into secure world again to setup secure and normal
	smc	#0
	
	bl test_use_print
	
	@ Now we're in NS-SVC, make a Hyp call to get into Hyp mode
#hvc	#0
	@ We will end up here in NS-Hyp.
.endm

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
vectors:
	.word 0	/* reset */
#.word 0	/* undef */
	b undef_handler
	b	2f /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	b	1f
	.word 0 /* irq */
	.word 0 /* fiq */

.align 5
new_vectors:
	.word 0	/* reset */
#.word 0	/* undef */
	b new_undef_handler
	b new_smc_handler /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	b	1f
	.word 0 /* irq */
	.word 0 /* fiq */

undef_handler:
	b test_undef

new_undef_handler:
	b test_new_undef

new_smc_handler:
	ldr r0, =0x1c090000
	mov r1, '['
	str r1, [r0]
	mov r1, 'T'
	str r1, [r0]
	mov r1, 'Z'
	str r1, [r0]
	mov r1, ']'
	str r1, [r0]
	mov r1, ':'
	str r1, [r0]
	mov r1, ' '
	str r1, [r0]
	mov r1, 'M'
	str r1, [r0]
	mov r1, 'S'
	str r1, [r0]
	mov r1, 'G'
	str r1, [r0]
	mov r1, '\n'
	str r1, [r0]
	mov r1, '\r'
	str r1, [r0]
#b test_print
#b .

@@@@@@@@@@@@@@@@
	@ go to secure mode and setup my code
	MRC p15, 0, r0, c1, c1, 0
	AND r0, r0, 0xFFFFFFFE
	MCR p15, 0, r0, c1, c1, 0

	b my_entry
	b .
@@@@@@@@@@@@@@@@

	movs pc, lr

.global my_entry
my_entry:
	ldr     r0, =stacktop
		
	CPS     #Mode_MON
	LDR     sp, =stacktop

	MSR     CPSR_c, #Mode_FIQ | I_Bit | F_Bit /* No interrupts*/
	SUB     sp, r0, #Offset_FIQ_Stack
	
	MSR     CPSR_c, #Mode_IRQ | I_Bit | F_Bit /* No interrupts */
	SUB     sp, r0, #Offset_IRQ_Stack
	
	MSR     CPSR_c, #Mode_ABT | I_Bit | F_Bit /* No interrupts */
	SUB     sp, r0, #Offset_ABT_Stack
	
	MSR     CPSR_c, #Mode_UND | I_Bit | F_Bit  
	SUB     sp, r0, #Offset_UND_Stack   
	
	MSR     CPSR_c, #Mode_SVC | I_Bit | F_Bit /* No interrupts */
	SUB     sp, r0, #Offset_SVC_Stack

	bl bootmain	
	b .

/* Return directly back to the caller without leaving Hyp mode: */
1:	mrs	lr, elr_hyp
	mov	pc, lr

/* In monitor mode, set up HVBAR and SCR then return to caller in NS-SVC. */
2:
	ldr	r10, =new_vectors
	mcr	p15, 0, r10, c12, c0, 1		@ Monitor vector base address

	@ Set up HVBAR
	mrc	p15, 0, r10, c1, c1, 0		@ SCR
	@ Set SCR.NS=1 (needed for setting HVBAR and also returning to NS state)
	@        .IRQ,FIQ,EA=0 (don't take aborts/exceptions to Monitor mode)
	@        .FW,AW=1 (CPSR.A,F modifiable in NS state)
	@        .nET=0 (early termination OK)
	@        .SCD=1 (SMC in NS mode is UNDEF, so accidental SMCs don't
	@                cause us to leap back into this code confusingly)
	@        .HCE=1 (HVC does Hyp call)
	bic	r10, r10, #0x07f
#ldr	r11, =0x1b1
	ldr	r11, =0x131
	orr	r10, r10, r11
	mcr	p15, 0, r11, c1, c1, 0
	isb
	ldr	r11, =vectors
	mcr	p15, 4, r11, c12, c0, 0		@ set HVBAR
		
	@ ...and return to calling code in NS state
	movs	pc, lr


	.globl	start
start:
#ifdef SMP
#ifdef VEXPRESS
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
#endif
	@
	@ CPU initialisation
	@
	mrc	p15, 0, r4, c0, c0, 5		@ MPIDR (ARMv7 only)
	and	r4, r4, #15			@ CPU number

	@
	@ Hypervisor / TrustZone initialization
	@

	@ Set all interrupts to be non-secure
	ldr	r0, =0x2c001000			@ Dist GIC base
	ldr	r1, [r0, #0x04]			@ Type Register
	cmp	r4, #0
	andeq	r1, r1, #0x1f
	movne	r1, #0
	add	r2, r0, #0x080			@ Security Register 0
	mvn	r3, #0
2:	str	r3, [r2]
	sub	r1, r1, #1
	add	r2, r2, #4			@ Next security register
	cmp	r1, #-1
	bne	2b

	@ Set GIC priority mask bit [7] = 1
	ldr	r0, =0x2c002000			@ CPU GIC base
	mov	r1, #0x80
	str	r1, [r0, #0x4]			@ GIC ICCPMR

	@ Set NSACR to allow coprocessor access from non-secure
	mrc	p15, 0, r0, c1, c1, 2
	ldr	r1, =0x43fff
	orr	r0, r0, r1
	mcr	p15, 0, r0, c1, c1, 2

	@ Check CPU nr again
	mrc	p15, 0, r0, c0, c0, 5		@ MPIDR (ARMv7 only)
	bfc	r0, #24, #8			@ CPU number, taking multicluster into account
	cmp	r0, #0				@ primary CPU?
	beq	2f

	@
	@ Secondary CPUs (following the RealView SMP booting protocol)
	@
	@@enter_hyp

	@@ldr	r1, =fs_start - 0x100
	@@adr	r2, 1f
	@@ldmia	r2, {r3 - r7}			@ move the code to a location
	@@stmia	r1, {r3 - r7}			@ less likely to be overridden
#ifdef VEXPRESS
	@@ldr	r0, =0x1c010030			@ VE SYS_FLAGS register
#else
	@@ldr	r0, =0x10000030			@ RealView SYS_FLAGS register
#endif
	@@mov	pc, r1				@ branch to the relocated code
1:
#ifdef VEXPRESS
	@@wfe
#endif
	@@ldr	r1, [r0]
	@@cmp	r1, #0
	@@beq	1b
	@@mov	pc, r1				@ branch to the given address
#endif

2:
	@
	@ UART initialisation (38400 8N1)
	@
#ifdef MACH_MPS
	ldr	r0, =0x1f005000			@ UART3 base (MPS)
#elif defined (VEXPRESS)
	ldr	r0, =0x1c090000			@ UART base (Versatile Express)
#else
	ldr	r0, =0x10009000			@ UART base (RealView/EB)
#endif
	mov	r1, #0x10			@ ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			@ cr
	str	r1, [r0, #0x30]

	@ Now we've got rid of the secondary CPUs, set up a stack
	@ for CPU 0 so we can write most of this in C.
	ldr     sp, =stacktop
	
	enter_hyp
	@ And call the C entrypoint
	bl      c_start
	@ Never reached
1:	b 1b

	@
	@ Function for C code to make semihosting calls:
	@
	.globl __semi_call
__semi_call:
#if defined(MACH_MPS)
	@ M profile semihosting is via bpkt
	bkpt    0xab
#elif defined(__thumb__)
	@ Otherwise, different SVC numbers for ARM or Thumb mode
	svc    0xab
#else
	svc     0x123456
#endif
	mov pc, lr

.globl __boot_kernel
__boot_kernel:
	mov	r4, r0
	stmfd	sp!, {r1-r3}
	ldmia	sp, {r0-r3}

#enter_hyp
	hvc #0
	bx	r4
.type __boot_kernel, %function

	@
	@ Data
	@
	/* The kernel boot command line for builtin kernels is defined in the Make system */
	.globl kernel_cmd
	.globl kernel_cmd_end
kernel_cmd:
#ifdef KCMD
	.asciz KCMD
#endif
kernel_cmd_end:


	.ltorg
	.section .init, "ax"
	.code 32

	Mode_MON            =   0x16
	Mode_SVP            =   0x13
	NS_BIT              =   0x1

.global monitor
.align 5
monitor:
	@ Monitor
	NOP     @ Reset      - not used by Monitor
	NOP     @ Undef      - not used by Monitor
	B       SMC_Handler
	NOP     @ Prefetch   - can by used by Monitor
	NOP     @ Data abort - can by used by Monitor
	NOP     @ RESERVED
	NOP     @ IRQ        - can by used by Monitor
	NOP     @ FIQ        - can by used by Monitor

@ ------------------------------------------------------------
@ SMC Handler
@
@ - Detect which world executed SMC
@ - Saves state to appropriate stack
@ - Restores other worlds state
@ - Switches world
@ - Performs exception return
@ ------------------------------------------------------------
.global SMC_Handler
SMC_Handler:
	PUSH   {r0-r3}                       @ R0-r3 contain args to be passed between worlds
										 @ Temporarily stack, so can be used as scratch regs
										 
	@ Which world have we come from
	@ ------------------------------
	MRC     p15, 0, r0, c1, c1, 0        @ Read Secure Configuration Register data
	TST     r0, #NS_BIT                  @ Is the NS bit set?
	EOR     r0, r0, #NS_BIT              @ Toggle NS bit
	MCR     p15, 0, r0, c1, c1, 0        @ Write Secure Configuration Register data

	@ Load save to pointer
	@ ---------------------
	LDREQ   r0, =S_STACK_SP             @ If NS bit set, was in Normal world.  So restore Secure state
	LDRNE   r0, =NS_STACK_SP
	LDR     r2, [r0]

	@ Load restore from pointer
	@ --------------------------
	LDREQ   r1, =NS_STACK_SP
	LDRNE   r1, =S_STACK_SP
	LDR     r3, [r1]

	@ r2  <-- save to
	@ r3  <-- restore from
 
	@ Save general purpose registers, SPSR and LR
	@ --------------------------------------------
	STMFD   r2!, {r4-r12}               @ Save r4 to r12
	@ ADD SUPPORT FOR SPs
	MRS     r4, spsr                    @ Also get a copy of the SPSR
	STMFD   r2!, {r4, lr}               @ Save original SPSR and LR
	
	STR     r2, [r0]                    @ Save updated pointer back, r0 and r2 now free
	
	@ Restore other world registers, SPSR and LR
	@ ---------------------------------------------
	LDMFD   r3!, {r0, lr}               @ Get SPSR and LR from
	@ ADD SUPPORT FOR SPs
	MSR     spsr_cxsf, r0               @ Restore SPSR
	LDMFD   r3!, {r4-r12}               @ Restore registers r4 to r12

	STR     r3, [r1]                    @ Save updated pointer back, r1 and r3 now free
	
	@ Clear local monitor
	@ --------------------
	CLREX                               @ Not strictly required in this example, as not using LDR EX/STREX
										@ However, architecturally should execute CLREX on a context switch

	@ Now restore args (r0-r3)
	@ -------------------------
	POP     {r0-r3}
	
	
	@ Perform exception return
	@ -------------------------
	MOVS    pc, lr

@ ------------------------------------------------------------
@ Monitor Initialization
@
@ This is called the first time the Secure world wishes to
@ move to the Normal world.
@ ------------------------------------------------------------

.global monitorInit
monitorInit:
	@ Install Secure Monitor
	@ -----------------------
	LDR r1, =ns_image                    /* R1 is used !!!!*/
	STR r0, [r1]
	LDR r0, =monitor                 @ Get address of Monitors vector table
	MCR p15, 0, r0, c12, c0, 1       @ Write Monitor Vector Base Address Register

	@ Save Secure state
	@ ------------------
	LDR     r0, =S_STACK_LIMIT          @ Get address of Secure state stack
	STMFD   r0!, {r4-r12}               @ Save general purpose registers
	@ ADD support for SPs
	MRS     r1, cpsr                    @ Also get a copy of the CPSR
	STMFD   r0!, {r1, lr}               @ Save CPSR and LR

	@ Switch to Monitor mode
	@ -----------------------
	CPS     #Mode_MON                   @ Move to Monitor mode after saving Secure state

	@ Save Secure state stack pointer
	@ --------------------------------
	LDR     r1, =S_STACK_SP              @ Get address of global
	STR     r0, [r1]                     @ Save pointer


	@ Set up initial NS state stack pointer
	@ --------------------------------------
	LDR     r0, =NS_STACK_SP             @ Get address of global
	LDR     r1, =NS_STACK_LIMIT          @ Get top of Normal state stack (assuming FD model)
	STR     r1, [r0]                     @ Save pointer


	@ Set up exception return information
	@ ------------------------------------
	@IMPORT  ns_image
	 
	LDR     lr, ns_image              @ ns_image
	MSR     spsr_cxsf, #Mode_SVP         @ Set SPSR to be SVC mode

	@ Switch to Normal world
	@ -----------------------
	MRC     p15, 0, r4, c1, c1, 0        @ Read Secure Configuration Register data
	ORR     r4, #NS_BIT                  @ Set NS bit
	MCR     p15, 0, r4, c1, c1, 0        @ Write Secure Configuration Register data
	 
	 
	@ Clear general purpose registers
	@ --------------------------------
	MOV     r0,  #0
	MOV     r1,  #0
	MOV     r2,  #0
	MOV     r3,  #0
	MOV     r4,  #0
	MOV     r5,  #0
	MOV     r6,  #0
	MOV     r7,  #0
	MOV     r8,  #0
	MOV     r9,  #0
	MOV     r10, #0
	MOV     r11, #0
	MOV     r12, #0
 
	MOVS    pc, lr

@ ------------------------------------------------------------
@ Space reserved for stacks
@ ------------------------------------------------------------
NS_STACK_BASE:
	.word     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
NS_STACK_LIMIT:

S_STACK_BASE:
	.word     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
S_STACK_LIMIT:

NS_STACK_SP:
	.word     0
	
S_STACK_SP:
	.word     0

ns_image :
	.word     0

@.end
@ ------------------------------------------------------------
@ End
@ ------------------------------------------------------------


UndefHandler:
	b UndefHandler

SWIHandler:
	b SWIHandler

PAbortHandler:
	b PAbortHandler

DAbortHandler:
	b DAbortHandler

IRQHandler:
	b IRQHandler

FIQHandler:
	b FIQHandler
