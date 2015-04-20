#include "types.h"
#include "defs.h"
#include "syscall.h"
#include "uart.h"
#include "proc.h"
#include "uart.h"

// User code makes a system call with INT T_SYSCALL. System call number
// in r0. Arguments on the stack, from the user call to the C library
// system call function. The saved user sp points to the first argument.

// Fetch the int at addr from the current process.
// get an int at addr in user space
int fetchint(uint addr, int *ip)
{
	if(addr >= proc->sz || addr+4 > proc->sz) {
		return -1;
	}

	*ip = *(int*)(addr);
	return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int fetchstr(uint addr, char **pp)
{
	char *s, *ep;

	if(addr >= proc->sz) {
		return -1;
	}

	*pp = (char*)addr;
	ep = (char*)proc->sz;

	for(s = *pp; s < ep; s++) {
		if(*s == 0) {
			return s - *pp;
		}
	}
	
	return -1;
}

// Fetch the nth (starting from 0) 32-bit system call argument.
// In our ABI, r0 contains system call index, r1-r4 contain parameters.
// now we support system calls with at most 4 parameters.
int argint(int n, int *ip)
{
	if (n > 3) {
		panic ("too many system call parameters\n");
	}

	*ip = *(&proc->tf->r1 + n);

	return 0;
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size n bytes.  Check that the pointer
// lies within the process address space.
int argptr(int n, char **pp, int size)
{
	int i;

	if(argint(n, &i) < 0) {
		return -1;
	}

	if((uint)i >= proc->sz || (uint)i+size > proc->sz) {
		return -1;
	}

	*pp = (char*)i;
	return 0;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int argstr(int n, char **pp)
{
	int addr;
	
	if(argint(n, &addr) < 0) {
		return -1;
	}

	return fetchstr(addr, pp);
}

int sys_test7(void)
{
	int arg;
	argint(0, &arg);
	cprintf("This is sys_test7: %d\n", arg);
	return 0;
}

int sys_test8(void)
{
	cprintf("This is sys_test8\n");
	return 0;
}

int sys_test9(void)
{
	char *str;
	if(argstr(0, &str) < 0)
	{
		cprintf("obtain param error!\n");
	}
	cprintf("This is sys_test9: %s\n", str);
	return 0;
}

int sys_exit(void)
{
	cprintf("This is sys_exit\n");
	resume_stk(cpu->scheduler);
	cprintf("TMP finish sys_exit\n");
	while(1);
	return 0;
}

int sys_char(void)
{
	int c;
	argint(0, &c);
	uart_send(c);
	return 0;
}

static int (*syscalls[])(void) = {
	[SYS_test7]      sys_test7,
	[SYS_test8]      sys_test8,
	[SYS_test9]      sys_test9,
	[SYS_exit]      sys_exit,
	[SYS_char]      sys_char,    
};

void syscall(void)
{
	int num;
	int ret;

	num = proc->tf->r0;

	ret = syscalls[num]();
	proc->tf->r0 = ret;

	return;
}
