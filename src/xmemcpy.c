#include "xmemcpy.h"

#include <stdint.h>

#define DWORD_COPY_THRESHOLD    16

#define alignment(p)            ((uintptr_t) (p) & 3)

#define shift_combine_1(a, b)   (((a) >>  8) + ((b) << 24))
#define shift_combine_2(a, b)   (((a) >> 16) + ((b) << 16))
#define shift_combine_3(a, b)   (((a) >> 24) + ((b) <<  8))

/* Note: The following functions were originally targeted at P5 class CPUs, on
 * which the performance of xmemcpy() should be close to optimal for C code.
 * For today's CPUs and compilers, it's better to write simple counter-based
 * copy loops, and let the compiler's vectorizer do its job. */

void *xmemcpy(void *restrict dst, const void *restrict src, size_t n)
{
    const uint8_t *sp = (uint8_t *) src;
    uint8_t *dp = (uint8_t *) dst;

    if (n < DWORD_COPY_THRESHOLD)
        goto byte_copy;

    unsigned int shift = (uintptr_t) (sp - dp) & 3;

    if (shift == 0) {

        size_t n1 = 4 - alignment(dp);
        n -= n1;

        while (n1-- > 0)
            *dp++ = *sp++;

        const uint32_t *dw_sp = (uint32_t *) sp;
        uint32_t *dw_dp = (uint32_t *) dp;

        size_t n2 = n >> 2;
        n &= 3;

        while (n2-- > 0)
            *dw_dp++ = *dw_sp++;

        sp = (uint8_t *) dw_sp;
        dp = (uint8_t *) dw_dp;

    } else {

        size_t n1 = 4 - alignment(dp);
        n -= n1;

        while (n1-- > 0)
            *dp++ = *sp++;

        const uint32_t *dw_sp = (uint32_t *) (sp - shift);
        uint32_t *dw_dp = (uint32_t *) dp;

        size_t n2 = (n >> 2) - 1;
        n -= n2 << 2;

        uint32_t a, b;

        a = *dw_sp++;

        if (shift == 1) {
            while (n2-- > 0) {
                b = *dw_sp++;
                *dw_dp++ = shift_combine_1(a, b);
                a = b;
            }
        } else if (shift == 2) {
            while (n2-- > 0) {
                b = *dw_sp++;
                *dw_dp++ = shift_combine_2(a, b);
                a = b;
            }
        } else {
            while (n2-- > 0) {
                b = *dw_sp++;
                *dw_dp++ = shift_combine_3(a, b);
                a = b;
            }
        }

        sp = (uint8_t *) dw_sp - (4 - shift);
        dp = (uint8_t *) dw_dp;
    }

byte_copy:

    while (n-- > 0)
        *dp++ = *sp++;

    return dst;
}

/* We don't need the shift-and-combine algorithm for xmemmove() and xmemset()
 * because we are using these functions mostly for properly aligned data. */

void *xmemmove(void *dst, const void *src, size_t n)
{
    if ((uint8_t *) src + n <= dst || (uint8_t *) dst + n <= src)
        return xmemcpy(dst, src, n);

    if (src >= dst) {

        const uint8_t *sp = (uint8_t *) src;
        uint8_t *dp = (uint8_t *) dst;

        if (n < DWORD_COPY_THRESHOLD || alignment(sp) != alignment(dp))
            goto forward_byte_copy;

        size_t n1 = 4 - alignment(dp);
        n -= n1;

        while (n1-- > 0)
            *dp++ = *sp++;

        const uint32_t *dw_sp = (uint32_t *) sp;
        uint32_t *dw_dp = (uint32_t *) dp;

        size_t n2 = n >> 2;
        n &= 3;

        while (n2-- > 0)
            *dw_dp++ = *dw_sp++;

        sp = (uint8_t *) dw_sp;
        dp = (uint8_t *) dw_dp;

    forward_byte_copy:

        while (n-- > 0)
            *dp++ = *sp++;

    } else {

        const uint8_t *sp = (uint8_t *) src + n;
        uint8_t *dp = (uint8_t *) dst + n;

        if (n < DWORD_COPY_THRESHOLD || alignment(sp) != alignment(dp))
            goto backward_byte_copy;

        size_t n1 = alignment(dp);
        n -= n1;

        while (n1-- > 0)
            *(--dp) = *(--sp);

        const uint32_t *dw_sp = (uint32_t *) sp;
        uint32_t *dw_dp = (uint32_t *) dp;

        size_t n2 = n >> 2;
        n &= 3;

        while (n2-- > 0)
            *(--dw_dp) = *(--dw_sp);

        sp = (uint8_t *) dw_sp;
        dp = (uint8_t *) dw_dp;

    backward_byte_copy:

        while (n-- > 0)
            *(--dp) = *(--sp);
    }

    return dst;
}

void *xmemset(void *ptr, int c, size_t n)
{
    uint8_t b = (uint8_t) c;
    uint8_t *p = (uint8_t *) ptr;

    if (n < DWORD_COPY_THRESHOLD)
        goto byte_memset;

    size_t n1 = 4 - alignment(p);
    n -= n1;

    while (n1-- > 0)
        *p++ = b;

    uint32_t dw_b = b;
    dw_b |= (dw_b << 8);
    dw_b |= (dw_b << 16);

    uint32_t *dw_p = (uint32_t *) p;

    size_t n2 = n >> 2;
    n &= 3;

    while (n2-- > 0)
        *dw_p++ = dw_b;

    p = (uint8_t *) dw_p;

byte_memset:

    while (n-- > 0)
        *p++ = b;

    return ptr;
}
