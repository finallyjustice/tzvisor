#ifndef STRING_H
#define STRING_H

#include <stddef.h>

extern void *(bw_memcpy)(void *__dest, __const void *__src, size_t __n);
extern void *(bw_memmove)(void *__dest, __const void *__src, size_t __n);
extern void *(bw_memchr)(void const *s, int c, size_t n);
extern size_t (bw_strlen)(const char *s);
extern void *(bw_memset)(void *s, int c, size_t count);
extern int (bw_memcmp)(void const *p1, void const *p2, size_t n);
extern int (bw_strcmp)(char const *s1, char const *s2);
extern int (bw_strncmp)(char const *s1, char const *s2, size_t n);
extern char *(bw_strchr)(char const *s, int c);

#endif
