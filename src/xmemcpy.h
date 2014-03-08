#ifndef XMEMCPY_H
#define XMEMCPY_H

#include <stddef.h>

void *xmemcpy(void *restrict dst, const void *restrict src, size_t n);
void *xmemmove(void *dst, const void *src, size_t n);
void *xmemset(void *ptr, int c, size_t n);

#endif
