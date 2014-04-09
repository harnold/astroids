#ifndef SHIP_H
#define SHIP_H

#include "sprite.h"

#include <stdbool.h>

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
