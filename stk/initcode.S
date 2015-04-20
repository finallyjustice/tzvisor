# Initial process execs /init.

#include "syscall.h"

.globl start

# exec(init, argv)
start:
    LDR     r1, =init
    LDR     r2, =argv
    mov     r0, #SYS_test7
    swi     0x00
	
	mov     r0, #SYS_test8
    swi     0x00
	
	mov     r0, #SYS_test9
    swi     0x00

	b .
	
exit:
    MOV     r0, #SYS_exit
    SWI     0x00
    B       exit

# char init[] = "/init\0";
init:
    .string "/init\0"

# char *argv[] = { init, 0 };
.p2align 2
argv:
    .word init
    .word 0

