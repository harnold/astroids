#ifndef SHIP_H
#define SHIP_H

#include "sprite.h"

#include <stdbool.h>

#define SHIP_MIN_ENERGY         0.01f
#define SHIP_TURN_PER_SEC       (FLOAT_2PI)
#define SHIP_MAX_ENGINE_POWER   500000.0f
#define SHIP_ENGINE_POWER_INC   20000.0f
#define SHIP_MASS               100.0f
#define SHIP_SHIELD_FLICKER     3
#define SHIP_COLLISION_RADIUS   12

struct ship {
    float energy;
    float x;
    float y;
    float vx;
    float vy;
    float dir;
    float engine_power;
    short shield_flicker;
    unsigned shield_active:1;
    unsigned shield_visible:1;
    struct sprite ship_sprite;
    struct sprite shield_sprite;
};

void init_ship(struct ship *ship, float x, float y, int z);
void destroy_ship(struct ship *ship);
void ship_set_direction(struct ship *ship, float dir);
void ship_turn(struct ship *ship, float r);
void ship_set_power(struct ship *ship, float power);
void ship_set_shield(struct ship *ship, bool active);
void ship_update_sprite(struct ship *ship);

#endif
