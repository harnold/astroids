#include "ship.h"
#include "compat.h"
#include "res.h"
#include "world.h"

#include <math.h>

#define NUM_DIRECTIONS          12
#define RAD_PER_DIRECTION       (FLOAT_2PI / NUM_DIRECTIONS)
#define MIN_POWER               0.01f

static struct sprite_class ship_class = {
    &ship_image, 32, 32, 2 * NUM_DIRECTIONS, 16.0f, 16.0f
};

static struct sprite_class shield_class = {
    &shield_image, 32, 32, 1, 16.0f, 16.0f
};

void init_ship(struct ship *ship, float x, float y, int z)
{
    ship->energy = 1.0f;
    ship->x = x;
    ship->y = y;
    ship->v = 0;
    ship->dir = 0;
    ship->engine_power = 0;

    init_sprite(&ship->ship_sprite, &ship_class,
                world_to_screen_x(x),
                world_to_screen_y(y), z, 0);

    init_sprite(&ship->shield_sprite, &shield_class,
                world_to_screen_x(x),
                world_to_screen_y(y), z, 0);
}

void destroy_ship(struct ship *ship)
{
    destroy_sprite(&ship->ship_sprite);
    destroy_sprite(&ship->shield_sprite);
}

void ship_set_direction(struct ship *ship, float dir)
{
    ship->dir = dir;

    while (ship->dir < 0)
        ship->dir += FLOAT_2PI;

    unsigned frame =
        (unsigned) (NUM_DIRECTIONS * (ship->dir + (RAD_PER_DIRECTION / 2)) / FLOAT_2PI)
        % NUM_DIRECTIONS;

    ship->ship_sprite.frame = ship->engine_power > 0 ?
        frame + NUM_DIRECTIONS : frame;
}

void ship_turn(struct ship *ship, float r)
{
    ship_set_direction(ship, ship->dir + r);
}
