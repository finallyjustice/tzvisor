#ifndef TYPES_INCLUDE
#define TYPES_INCLUDE

typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   uchar;
typedef unsigned int    uint32;
typedef int             int32;
typedef unsigned short  uint16;
typedef short           int16;
typedef unsigned char   uint8;
typedef unsigned long   ulong;
typedef unsigned int    Rendez;

#define __REG(x)    (*((volatile unsigned int *)(x)))

#ifndef NULL
#define NULL ((void*)0)
#endif


#endif
