#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct ringbuf {
    size_t capacity;
    size_t bytes_filled;
    size_t write_pos;
    uint8_t *data;
};

int init_ringbuf(struct ringbuf *buf, void *ptr, size_t capacity);
bool ringbuf_empty(struct ringbuf *buf);
void ringbuf_clear(struct ringbuf *buf);
int ringbuf_push_back(struct ringbuf *buf, const void *restrict src, size_t n);
int ringbuf_pop_front(struct ringbuf *buf, void *restrict dst, size_t n);

#endif
