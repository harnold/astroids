#ifndef RGB_H
#define RGB_H

#include <stdint.h>

typedef uint32_t rgb_t;

static inline rgb_t make_rgb(unsigned int r, unsigned int g, unsigned int b)
{
    return (r << 16) & (g << 8) & b;
}

static inline unsigned int rgb_r(rgb_t rgb)
{
    return (rgb & 0xFF0000) >> 16;
}

static inline unsigned int rgb_g(rgb_t rgb)
{
    return (rgb & 0xFF00) >> 8;
}

static inline unsigned int rgb_b(rgb_t rgb)
{
    return rgb & 0xFF;
}

#endif
