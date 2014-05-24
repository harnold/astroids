#include "missile.h"
#include "res.h"
#include "world.h"

DEFINE_ALLOCATOR(missile, struct missile, ALLOC_DEFAULT_BLOB_SIZE);

const struct sprite_class missile_class = {
    &missile_image, 8, 8, 1, 4.0f, 4.0f
};

void init_missile(struct missile *mis, float x, float y, float vx, float vy, int z)
{
    mis->x = x;
    mis->y = y;
    mis->vx = vx;
    mis->vy = vy;

    init_sprite(&mis->sprite, &missile_class,
                world_to_screen_x(x),
                world_to_screen_y(y), z, 0);

    elist_link(&mis->link, &mis->link);
}

void destroy_missile(struct missile *mis)
{
    elist_remove(&mis->link);
    destroy_sprite(&mis->sprite);
}

struct missile *create_missile(float x, float y, float vx, float vy, int z)
{
    struct missile *mis = alloc_missile();
    init_missile(mis, x, y, vx, vy, z);
    return mis;
}

void delete_missile(struct missile *mis)
{
    destroy_missile(mis);
    free_missile(mis);
}

void missile_cleanup()
{
    free_all_missile_allocs();
}
