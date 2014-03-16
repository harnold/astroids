#ifndef BITS_H
#define BITS_H

#include <stdbool.h>

static inline bool test_bit(unsigned value, unsigned flag)
{
    return (value & flag) != 0;
}

#endif
