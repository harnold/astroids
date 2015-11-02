#include "ringbuf.h"
#include "xmemcpy.h"

#define min(a, b)       (((a) <= (b)) ? (a) : (b))

int init_ringbuf(struct ringbuf *buf, void *ptr, size_t capacity)
{
    if (capacity > SIZE_MAX / 2)
        return -1;

    buf->capacity = capacity;
    buf->bytes_filled = 0;
    buf->write_pos = 0;
    buf->data = (uint8_t *) ptr;

    return 0;
}

bool ringbuf_empty(struct ringbuf *buf)
{
    return buf->bytes_filled == 0;
}

void ringbuf_clear(struct ringbuf *buf)
{
    buf->bytes_filled = 0;
}

int ringbuf_push_back(struct ringbuf *buf, const void *src, size_t n)
{
    if (buf->capacity < n)
        return -1;

    size_t n1 = min(buf->capacity - buf->write_pos, n);

    xmemcpy(buf->data + buf->write_pos, src, n1);

    if (n > n1)
        xmemcpy(buf->data, (uint8_t *) src + n1, n - n1);

    buf->bytes_filled = min(buf->bytes_filled + n, buf->capacity);
    buf->write_pos = (buf->write_pos + n) % buf->capacity;
    return 0;
}

int ringbuf_pop_front(struct ringbuf *buf, void *dst, size_t n)
{
    if (buf->bytes_filled < n)
        return -1;

    size_t read_pos = buf->write_pos >= buf->bytes_filled ?
        buf->write_pos - buf->bytes_filled :
        buf->capacity - (buf->bytes_filled - buf->write_pos);

    size_t n1 = min(buf->capacity - read_pos, n);

    xmemcpy(dst, buf->data + read_pos, n1);

    if (n > n1)
        xmemcpy((uint8_t *) dst + n1, buf->data, n - n1);

    buf->bytes_filled -= n;
    return 0;
}
