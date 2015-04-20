#include <string.h>

static void *__memmove_down(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;

	while (__n--)
		*d++ = *s++;

	return __dest;
}

static void *__memmove_up(void *__dest, __const void *__src, size_t __n)
{
	unsigned char *d = (unsigned char *)__dest + __n - 1, *s = (unsigned char *)__src + __n - 1;

	while (__n--)
		*d-- = *s--;

	return __dest;
}

void *(bw_memcpy)(void *__dest, __const void *__src, size_t __n)
{
	return __memmove_down(__dest, __src, __n);
}

void *(bw_memmove)(void *__dest, __const void *__src, size_t __n)
{
	if(__dest > __src)
		return __memmove_up(__dest, __src, __n);
	else
		return __memmove_down(__dest, __src, __n);
}

void *(bw_memchr)(void const *s, int c, size_t n)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(n && *_s != c) {
		++_s;
		--n;
	}

	if(n)
		return (void *)_s;	/* the C library casts const away */
	else
		return (void *)0;
}

size_t (bw_strlen)(const char *s)
{
	const char *sc = s;

	while (*sc != '\0')
		sc++;
	return sc - s;
}

void *(bw_memset)(void *s, int c, size_t count)
{
	char *xs = s;
	while (count--)
		*xs++ = c;
	return s;
}

int (bw_memcmp)(void const *p1, void const *p2, size_t n)
{
	unsigned char const *_p1 = p1;
	unsigned char const *_p2 = p2;

	while(n--) {
		if(*_p1 < *_p2)
			return -1;
		else if(*_p1 > *_p2)
			return 1;

		++_p1;
		++_p2;
	}

	return 0;
}

int (bw_strcmp)(char const *s1, char const *s2)
{
	while(*s1 && *s2) {
		if(*s1 < *s2)
			return -1;
		else if(*s1 > *s2)
			return 1;

		++s1;
		++s2;
	}

	if(!*s1 && !*s2)
		return 0;
	else if(!*s1)
		return -1;
	else
		return 1;
}

int (bw_strncmp)(char const *s1, char const *s2, size_t n)
{
	while(*s1 && *s2 && n--) {
		if(*s1 < *s2)
			return -1;
		else if(*s1 > *s2)
			return 1;

		++s1;
		++s2;
	}

	if(n == 0 || (!*s1 && !*s2))
		return 0;
	else if(!*s1)
		return -1;
	else
		return 1;
}

char *(bw_strchr)(char const *s, int c)
{
	unsigned char const *_s = (unsigned char const *)s;

	while(*_s && *_s != c)
		++_s;

	if(*_s)
		return (char *)_s;	/* the C library casts const away */
	else
		return (char *)0;
}
