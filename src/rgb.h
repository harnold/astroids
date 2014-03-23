#ifndef RGB_H
#define RGB_H

#include <stdint.h>

typedef uint32_t rgb_t;

static inline rgb_t make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
}

static inline uint8_t rgb_r(rgb_t rgb)
{
    return (uint8_t) (rgb >> 16);
}

static inline uint8_t rgb_g(rgb_t rgb)
{
    return (uint8_t) (rgb >> 8);
}

static inline uint8_t rgb_b(rgb_t rgb)
{
    return (uint8_t) rgb;
}

#endif
