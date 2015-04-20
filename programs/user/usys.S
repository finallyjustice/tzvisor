#include "syscall.h"

#define SYSCALL(name) \
.globl sys_ ## name; \
sys_ ## name: \
	PUSH {r4};\
	MOV r4, r3;\
	MOV r3, r2;\
	MOV r2, r1;\
	MOV r1, r0;\
	MOV r0, #SYS_ ## name;\
	swi 0x00;\
	POP {r4};\
	bx lr;

SYSCALL(exit)
SYSCALL(disp)
SYSCALL(test7)
SYSCALL(test8)
SYSCALL(test9)
