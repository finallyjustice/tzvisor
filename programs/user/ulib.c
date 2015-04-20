#include "types.h"
#include "user.h"

char*
strcpy(char *s, char *t)
{
    char *os;
    
    os = s;
    while((*s++ = *t++) != 0)
        ;
    return os;
}

int
strcmp(const char *p, const char *q)
{
    while(*p && *p == *q)
        p++, q++;
    return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
    int n;
    
    for(n = 0; s[n]; n++)
        ;
    return n;
}

void*
memset(void *dst, int v, uint n)
{
	uint8	*p;
	uint8	c;
	uint32	val;
	uint32	*p4;

	p   = dst;
	c   = v & 0xff;
	val = (c << 24) | (c << 16) | (c << 8) | c;

	// set bytes before whole uint32
	for (; (n > 0) && ((uint)p % 4); n--, p++){
		*p = c;
	}

	// set memory 4 bytes a time
	p4 = (uint*)p;

	for (; n >= 4; n -= 4, p4++) {
		*p4 = val;
	}

	// set leftover one byte a time
	p = (uint8*)p4;

	for (; n > 0; n--, p++) {
		*p = c;
	}

	return dst;
}

char*
strchr(const char *s, char c)
{
    for(; *s; s++)
        if(*s == c)
            return (char*)s;
    return 0;
}

int
atoi(const char *s)
{
    int n;
    
    n = 0;
    while('0' <= *s && *s <= '9')
        n = n*10 + *s++ - '0';
    return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
    char *dst, *src;
    
    dst = vdst;
    src = vsrc;
    while(n-- > 0)
        *dst++ = *src++;
    return vdst;
}
/*
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
		sys_disp(buf[i]);
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
			sys_disp(c);
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
				sys_disp(*s);
			break;
		case '%':
			sys_disp('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			sys_disp('%');
			sys_disp(c);
			break;
		}
	}
}*/
