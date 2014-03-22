#ifndef BITVECT_H
#define BITVECT_H

#include <stdbool.h>
#include <stdint.h>

static inline void vbitvect_set_bit(volatile uint8_t *v, unsigned bit)
{
    *(v + bit / 8) |= 1 << (bit % 8);
}

static inline void vbitvect_clear_bit(volatile uint8_t *v, unsigned bit)
{
    *(v + bit / 8) &= ~(1 << (bit % 8));
}

static inline bool vbitvect_test_bit(volatile uint8_t *v, unsigned bit)
{
    return (*(v + bit / 8) & (1 << (bit % 8))) != 0;
}

#endif
