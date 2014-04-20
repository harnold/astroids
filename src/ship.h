#ifndef SHIP_H
#define SHIP_H

#include "sprite.h"

#include <stdbool.h>

#define SHIP_MIN_ENERGY         0.01f
#define SHIP_TURN_PER_SEC       (FLOAT_2PI)
#define SHIP_MAX_ENGINE_POWER   5000000.0f
#define SHIP_ENGINE_POWER_INC   100000.0f
#define SHIP_MASS               1000.0f

struct ship {
    float energy;
    float x;
    float y;
    float v;
    float dir;
    float engine_power;
    struct sprite ship_sprite;
    struct sprite shield_sprite;
};

void init_ship(struct ship *ship, float x, float y, int z);
void destroy_ship(struct ship *ship);
void ship_set_direction(struct ship *ship, float dir);
void ship_turn(struct ship *ship, float r);

#endif
