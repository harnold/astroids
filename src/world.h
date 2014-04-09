#ifndef WORLD_H
#define WORLD_H

#include "gfx.h"
#include "game.h"

#define WORLD_MIN_X     0.0f
#define WORLD_MAX_X     4000.0f
#define WORLD_MIN_Y     0.0f
#define WORLD_MAX_Y     3000.0f
#define WORLD_SIZE_X    (WORLD_MAX_X - WORLD_MIN_X)
#define WORLD_SIZE_Y    (WORLD_MAX_Y - WORLD_MIN_Y)
#define WORLD_CENTER_X  (WORLD_MIN_X + WORLD_SIZE_X / 2)
#define WORLD_CENTER_Y  (WORLD_MIN_Y + WORLD_SIZE_Y / 2)

static inline int world_to_screen_x(float x);
static inline int world_to_screen_dx(float dx);
static inline int world_to_screen_y(float y);
static inline int world_to_screen_dy(float dy);
static inline float screen_to_world_x(int x);
static inline float screen_to_world_dx(int dx);
static inline float screen_to_world_y(int y);
static inline float screen_to_world_dy(int dy);

static inline int world_to_screen_x(float x)
{
    return (int) ((x - WORLD_MIN_X) * gfx_mode_info.x_resolution / WORLD_SIZE_X);
}

static inline int world_to_screen_dx(float dx)
{
    return (int) (dx * gfx_mode_info.x_resolution / WORLD_SIZE_X);
}

static inline int world_to_screen_y(float y)
{
    return (int) ((y - WORLD_MIN_Y) * gfx_mode_info.y_resolution / WORLD_SIZE_Y);
}

static inline int world_to_screen_dy(float dy)
{
    return (int) (dy * gfx_mode_info.y_resolution / WORLD_SIZE_Y);
}

static inline float screen_to_world_x(int x)
{
    return (float) (x - WORLD_MIN_X) * WORLD_SIZE_X / gfx_mode_info.x_resolution;
}

static inline float screen_to_world_dx(int dx)
{
    return (float) dx * WORLD_SIZE_X / gfx_mode_info.x_resolution;
}

static inline float screen_to_world_y(int y)
{
    return (float) (y - WORLD_MIN_Y) * WORLD_SIZE_Y / gfx_mode_info.y_resolution;
}

static inline float screen_to_world_dy(int dy)
{
    return (float) dy * WORLD_SIZE_Y / gfx_mode_info.y_resolution;
}

#endif
