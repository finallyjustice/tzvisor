#ifndef __USER_H__
#define __USER_H__

#include "types.h"

// system calls
int sys_exit(void);
int sys_disp(int c);
int sys_test7(int arg);
int sys_test8(void);
int sys_test9(char *str);


char* strcpy(char *s, char *t);
int strcmp(const char *p, const char *q);
uint strlen(char *s);
void* memset(void *dst, int v, uint n);
char* strchr(const char *s, char c);
int atoi(const char *s);
void* memmove(void *vdst, void *vsrc, int n);

void printint(int xx, int base, int sign);
void cprintf(char *fmt, ...);


#endif
