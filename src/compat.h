#ifndef COMPAT_H
#define COMPAT_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define array_length(a)         (sizeof(a) / sizeof((a)[0]))

static inline uint32_t hword(uint32_t x) { return x >> 16; }
static inline uint32_t lword(uint32_t x) { return x & 0xFFFF; }

static inline char *stpcpy(char *dst, const char *src)
{
    while (*src != '\0')
        *dst++ = *src++;
    *dst = *src;
    return dst;
}

static inline float frand(void) { return (float) rand() / RAND_MAX; }

#ifndef __unique_identifier

#define __concat1(a, b)                 a##b
#define __concat(a, b)                  __concat1(a, b)
#define __unique_identifier(prefix)     __concat(prefix, __LINE__)

#endif /* __unique_identifier */

#if __STDC_VERSION__ < 201000

#ifndef static_assert

#define _Static_assert(cond, msg) \
        typedef struct { \
            int __static_assert_failure: (cond) ? 1 : -1; \
        } __unique_identifier(__static_assert_);

#define static_assert   _Static_assert

#endif /* static_assert */

#ifndef alignof

#define _Alignof(type)  offsetof(struct { char a; type b; }, b)
#define alignof(type)   _Alignof(type)

#endif /* alignof */

#endif /* C standard < C11 */

#if defined(__WATCOMC__)

#define PACKED_STRUCT   _Packed struct

#else
#error Unsupported compiler
#endif /* Watcom C */

#endif
