.equ Mode_USR, 0x10
.equ Mode_FIQ, 0x11
.equ Mode_IRQ, 0x12
.equ Mode_SVC, 0x13
.equ Mode_ABT, 0x17
.equ Mode_UND, 0x1B
.equ Mode_HYP, 0x1A
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

.syntax unified
.arch_extension sec
.arch_extension virt

.globl	start
start:
	@
	@ Program architected timer frequency
	@
	mrc	p15, 0, r0, c0, c1, 1		@ CPUID_EXT_PFR1
	lsr	r0, r0, #16
	and	r0, r0, #1					@ Check generic timer support
	beq	1f
	ldr	r0, =24000000				@ 24MHz timer frequency
	mcr	p15, 0, r0, c14, c0, 0		@ CNTFRQ
1:
	@
	@ CPU initialisation
	@
	mrc	p15, 0, r4, c0, c0, 5		@ MPIDR (ARMv7 only)
	and	r4, r4, #15					@ CPU number

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
	add	r2, r2, #4				@ Next security register
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
	
	ldr     r0, =svc_stktop

	CPS     #Mode_MON
	LDR     sp, =svc_stktop

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

	CPS     #Mode_SVC
	
	bl bootmain


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


# during startup, kernel stack uses user address, now switch it to kernel addr
.global jump_stack
jump_stack:
	MOV     r0, sp
#ADD     r0, r0, #0x40000000
	sub     r0, r0, #0x30000000
	MOV     sp, r0
	MOV     pc, lr

.text

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
hyp_vectors:
	.word 0 /* reset */
	.word 0    /* undef */
	.word 0    /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	b   hvc_handler
	.word 0 /* irq */
	.word 0 /* fiq */

/* Return directly back to the caller without leaving Hyp mode: */
hvc_handler:
	mrs lr, elr_hyp
	mov pc, lr

.global setup_hyp_vectors
setup_hyp_vectors:
	cps #Mode_MON
	
	@ Set up HVBAR
	mrc p15, 0, r10, c1, c1, 0      @ SCR
	@ Set SCR.NS=1 (needed for setting HVBAR and also returning to NS state)
	@        .IRQ,FIQ,EA=0 (don't take aborts/exceptions to Monitor mode)
	@        .FW,AW=1 (CPSR.A,F modifiable in NS state)
	@        .nET=0 (early termination OK)
	@        .SCD=1 (SMC in NS mode is UNDEF, so accidental SMCs don't
	@                cause us to leap back into this code confusingly)
	@        .HCE=1 (HVC does Hyp call)
	bic r10, r10, #0x07f	
	ldr r11, =0x131
	orr r10, r10, r11
	mcr p15, 0, r11, c1, c1, 0

	isb
	ldr r11, =hyp_vectors
	mcr p15, 4, r11, c12, c0, 0     @ set HVBAR

	cps #Mode_SVC
	b go_to_kernel

.global setup_hyp_vectors2
setup_hyp_vectors2:
	cps #Mode_MON
	
	@ Set up HVBAR
	mrc p15, 0, r10, c1, c1, 0      @ SCR
	@ Set SCR.NS=1 (needed for setting HVBAR and also returning to NS state)
	@        .IRQ,FIQ,EA=0 (don't take aborts/exceptions to Monitor mode)
	@        .FW,AW=1 (CPSR.A,F modifiable in NS state)
	@        .nET=0 (early termination OK)
	@        .SCD=1 (SMC in NS mode is UNDEF, so accidental SMCs don't
	@                cause us to leap back into this code confusingly)
	@        .HCE=1 (HVC does Hyp call)
	bic r10, r10, #0x07f	
	ldr r11, =0x131
	orr r10, r10, r11
	mcr p15, 0, r11, c1, c1, 0

	isb
	ldr r11, =hyp_vectors
	mcr p15, 4, r11, c12, c0, 0     @ set HVBAR

	MRC p15, 0, r0, c1, c1, 0
	AND r0, r0, 0xFFFFFFFE
	MCR p15, 0, r0, c1, c1, 0
	
	cps #Mode_SVC
	mov pc, lr

.ltorg
.section .init, "ax"
#.code 32

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
	PUSH   {r0-r3}

	@ Which world have we come from
	@ ------------------------------
	MRC     p15, 0, r0, c1, c1, 0       
	and r0, r0, 0x1
	cmp r0, $NS_BIT
	beq call_secure_world	@ if NS bit is 1, change from normal to secure world
	bne call_normal_world	@ if NS bit is 0, change from secure to normal world

.global call_secure_world
call_secure_world:
	# from normal world
	# change NS first

	MRC     p15, 0, r0, c1, c1, 0
	EOR     r0, r0, #NS_BIT
	MCR     p15, 0, r0, c1, c1, 0   

	@ Load save to pointer
	@ ---------------------
	LDR   r0, =NS_STACK_SP
	LDR     r2, [r0]

	@ Load restore from pointer
	@ --------------------------
	LDR   r1, =S_STACK_SP
	LDR     r3, [r1]

	@ r2  <-- save to
	@ r3  <-- restore from
	
	@ Save general purpose registers, SPSR and LR
	@ --------------------------------------------
	STMFD   r2!, {r4-r12}               @ Save r4 to r12

	cps #Mode_SVC
	stmfd r2!, {r13}
	cps #Mode_MON

	@ ADD SUPPORT FOR SPs
	MRS     r4, spsr                    @ Also get a copy of the SPSR
	STMFD   r2!, {r4, lr}               @ Save original SPSR and LR
	
	STR     r2, [r0]                    @ Save updated pointer back, r0 and r2 now free
	
	@ Restore other world registers, SPSR and LR
	@ ---------------------------------------------
	LDMFD   r3!, {r0, lr}               @ Get SPSR and LR from	

	@ ADD SUPPORT FOR SPs
	MSR     spsr_cxsf, r0               @ Restore SPSR
	
	cps #Mode_SVC
	ldmfd r3!, {r13}
	cps #Mode_MON
	
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
	
.global call_normal_world
call_normal_world:
	# from secure world
	# change NS finally

	@ Load save to pointer
	@ ---------------------
	LDR   r0, =S_STACK_SP             @ If NS bit set, was in Normal world.  So restore Secure state
	LDR     r2, [r0]

	@ Load restore from pointer
	@ --------------------------
	LDR   r1, =NS_STACK_SP
	LDR     r3, [r1]

	@ r2  <-- save to
	@ r3  <-- restore from

	@ Save general purpose registers, SPSR and LR
	@ --------------------------------------------
	STMFD   r2!, {r4-r12}               @ Save r4 to r12

	cps #Mode_SVC
	stmfd r2!, {r13}
	cps #Mode_MON

	@ ADD SUPPORT FOR SPs
	MRS     r4, spsr                    @ Also get a copy of the SPSR
	STMFD   r2!, {r4, lr}               @ Save original SPSR and LR
	
	STR     r2, [r0]                    @ Save updated pointer back, r0 and r2 now free
	
	@ Restore other world registers, SPSR and LR
	@ ---------------------------------------------
	LDMFD   r3!, {r0, lr}               @ Get SPSR and LR from	

	@ ADD SUPPORT FOR SPs
	MSR     spsr_cxsf, r0               @ Restore SPSR
	
	cps #Mode_SVC
	ldmfd r3!, {r13}
	cps #Mode_MON
	
	LDMFD   r3!, {r4-r12}               @ Restore registers r4 to r12

	STR     r3, [r1]                    @ Save updated pointer back, r1 and r3 now free

	@ we change the NS bit finally to backup sp reg in normal svc mode
	MRC     p15, 0, r0, c1, c1, 0
	EOR     r0, r0, #NS_BIT
	MCR     p15, 0, r0, c1, c1, 0   

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

	mov r10, r13 @@@
	stmfd r0!, {r10} @@@

	@ ADD support for SPs
	MRS     r1, cpsr                    @ Also get a copy of the CPSR

	ldr lr, =secure_world @@@

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
