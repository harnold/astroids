#ifndef XMALLOC_H
#define XMALLOC_H

#include <stddef.h>

struct xmalloc_stats {
    int xmalloc_calls;
    int xfree_calls;
    size_t allocated_bytes;
};

void *xmalloc(size_t size);
void xfree(void *ptr);
void xmalloc_get_stats(struct xmalloc_stats *stat);

#endif
