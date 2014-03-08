#include "alloc.h"
#include "xmalloc.h"

#include <stdlib.h>

void *_allocate_entry(struct allocator *alloc)
{
    if (alloc->free_entries != NULL) {

        void **p = (void **) alloc->free_entries;
        alloc->free_entries = *p;
        return (void *) p;

    } else {

        size_t size = alloc->bytes_per_entry;
        struct alloc_blob *blob = alloc->blobs;

        if (blob == NULL || blob->bytes_left < size) {

            blob = xmalloc(alloc->blob_size);
            blob->next = alloc->blobs;
            blob->size = alloc->blob_size;
            blob->bytes_left = blob->size - alloc->data_offset;
            blob->offset = alloc->data_offset - offsetof(struct alloc_blob, data);

            alloc->blobs = blob;
        }

        void *p = blob->data + blob->offset;
        blob->bytes_left -= size;
        blob->offset += size;
        return p;
    }
}

void _free_entry(struct allocator *alloc, void *entry)
{
    void **p = (void **) entry;
    *p = alloc->free_entries;
    alloc->free_entries = (void *) p;
}

void _free_all(struct allocator *alloc)
{
    struct alloc_blob *blob = alloc->blobs;

    while (blob != NULL) {
        struct alloc_blob *next = blob->next;
        xfree(blob);
        blob = next;
    }

    alloc->blobs = NULL;
    alloc->free_entries = NULL;
}
