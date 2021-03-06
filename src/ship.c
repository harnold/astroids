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

static inline unsigned ship_get_dir_sector(struct ship *ship)
{
    return (unsigned) ((NUM_DIRECTIONS / FLOAT_2PI) *
                       (ship->dir + (RAD_PER_DIRECTION / 2)))
        % NUM_DIRECTIONS;
}

void init_ship(struct ship *ship, float x, float y, int z)
{
    ship->energy = 1.0f;
    ship->x = x;
    ship->y = y;
    ship->vx = 0;
    ship->vy = 0;
    ship->dir = 0;
    ship->engine_power = 0;
    ship->shield_flicker = 0;
    ship->shield_active = false;
    ship->shield_visible = false;

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

float ship_get_visible_direction(struct ship *ship)
{
    return (float) ship_get_dir_sector(ship) * (FLOAT_2PI / NUM_DIRECTIONS);
}

void ship_set_direction(struct ship *ship, float dir)
{
    ship->dir = dir;

    while (ship->dir < 0)
        ship->dir += FLOAT_2PI;
}

void ship_turn(struct ship *ship, float r)
{
    ship_set_direction(ship, ship->dir + r);
}

void ship_set_power(struct ship *ship, float power)
{
    ship->engine_power = confine_float(power, 0, SHIP_MAX_ENGINE_POWER);
}

void ship_update_sprite(struct ship *ship)
{
    unsigned frame = ship_get_dir_sector(ship);

    ship->ship_sprite.frame = ship->engine_power >= MIN_POWER ?
        frame + NUM_DIRECTIONS : frame;

    ship->ship_sprite.x = world_to_screen_x(ship->x);
    ship->ship_sprite.y = world_to_screen_y(ship->y);

    if (ship->shield_active) {

        ship->shield_sprite.x = ship->ship_sprite.x;
        ship->shield_sprite.y = ship->ship_sprite.y;

        if (ship->shield_flicker == 0) {
            elist_insert(&ship->shield_sprite.link, ship->ship_sprite.link.next);
            ship->shield_visible = true;
        } else if (ship->shield_flicker == 1) {
            elist_remove(&ship->shield_sprite.link);
            ship->shield_visible = false;
        }

        if (++ship->shield_flicker >= SHIP_SHIELD_FLICKER)
            ship->shield_flicker = 0;
    }
}

void ship_set_shield(struct ship *ship, bool active)
{
    if (active == ship->shield_active)
        return;

    ship->shield_active = active ? true : false;

    if (active) {
        ship->shield_flicker = 0;
    } else if (ship->shield_visible) {
        elist_remove(&ship->shield_sprite.link);
        ship->shield_visible = false;
    }
}
