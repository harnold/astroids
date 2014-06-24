#include "explosion.h"
#include "res.h"
#include "world.h"

DEFINE_ALLOCATOR(explosion, struct explosion, ALLOC_DEFAULT_BLOB_SIZE);

const struct animation explosion_animation = {
    ANIM_ONCE_FORWARD, EXPLOSION_FPS, 0, EXPLOSION_NUM_FRAMES - 1
};

const struct sprite_class explosion_class = {
    &explosion_image, 32, 32, 10, 16.0f, 16.0f
};

void init_explosion(struct explosion *exp, float x, float y, int z, float time)
{
    exp->x = x;
    exp->y = y;

    init_sprite(&exp->sprite, &explosion_class,
                world_to_screen_x(x),
                world_to_screen_y(y), z, 0);

    sprite_set_animation(&exp->sprite, &explosion_animation, time);
    elist_link(&exp->link, &exp->link);
}

void destroy_explosion(struct explosion *exp)
{
    elist_remove(&exp->link);
    destroy_sprite(&exp->sprite);
}

struct explosion *create_explosion(float x, float y, int z, float time)
{
    struct explosion *exp = alloc_explosion();
    init_explosion(exp, x, y, z, time);
    return exp;
}

void delete_explosion(struct explosion *exp)
{
    destroy_explosion(exp);
    free_explosion(exp);
}

void explosion_cleanup(void)
{
    free_all_explosion_allocs();
}
