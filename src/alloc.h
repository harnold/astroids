#ifndef ALLOC_H
#define ALLOC_H

#include "compat.h"

#include <stddef.h>
#include <stdint.h>

struct alloc_blob {
    struct alloc_blob *next;
    size_t size;
    size_t bytes_left;
    size_t offset;
    uint8_t data[];
};

struct allocator {
    size_t blob_size;
    size_t data_offset;
    size_t bytes_per_entry;
    struct alloc_blob *blobs;
    void *free_entries;
};

void *_allocate_entry(struct allocator *alloc);
void _free_entry(struct allocator *alloc, void *entry);
void _free_all(struct allocator *alloc);

#define ALLOC_DEFAULT_BLOB_SIZE         1000

#define _alloc_min_align(type) \
    (alignof(type) < alignof(void *) ? alignof(void *) : alignof(type))

#define DECLARE_ALLOCATOR(name, type) \
    type *alloc_##name(void); \
    void free_##name(type *); \
    void free_all_##name##_allocs(void);

#define _DEFINE_ALLOCATOR(name, type, alignment, blobsize) \
    static struct allocator name##_allocator = { \
        .blob_size = blobsize, \
        .data_offset = (offsetof(struct alloc_blob, data) + alignment - 1) & ~(alignment - 1), \
        .bytes_per_entry = (sizeof(type) + alignment - 1) & ~(alignment - 1), \
        .blobs = NULL, \
        .free_entries = NULL \
    }; \
    \
    type *alloc_##name(void) \
    { return (type *) _allocate_entry(&name##_allocator); } \
    \
    void free_##name(type *entry) \
    { _free_entry(&name##_allocator, entry); } \
    \
    void free_all_##name##_allocs(void) \
    { _free_all(&name##_allocator); }

#define DEFINE_ALLOCATOR(name, type, blobsize) \
    _DEFINE_ALLOCATOR(name, type, _alloc_min_align(type), blobsize)

#endif
