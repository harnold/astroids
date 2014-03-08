#include "xmalloc.h"
#include "error.h"

#include <errno.h>
#include <stdlib.h>

static struct xmalloc_stats xmalloc_stats;

void *xmalloc(size_t size)
{
    if (size == 0)
        size = 1;

    void *ptr = malloc(size);

    if (!ptr)
        fatal_errno("Out of memory?  malloc() failed to allocate %ul bytes", size);

    xmalloc_stats.xmalloc_calls++;
    xmalloc_stats.allocated_bytes += size;

    return ptr;
}

void xfree(void *ptr)
{
    xmalloc_stats.xfree_calls++;
    free(ptr);
}

void xmalloc_get_stats(struct xmalloc_stats *stats)
{
    *stats = xmalloc_stats;
}
