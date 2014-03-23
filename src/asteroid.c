#include "asteroid.h"
#include "res.h"
#include "world.h"

DEFINE_ALLOCATOR(asteroid, struct asteroid, ALLOC_DEFAULT_BLOB_SIZE);

const struct animation asteroid_animations[] = {
    { ANIM_ROTATE_FORWARD, 60, 0, 19 },
    { ANIM_ROTATE_FORWARD, 60, 0, 19 },
    { ANIM_ROTATE_FORWARD, 60, 0, 19 },
};

const struct sprite_class asteroid_classes[] = {
    { &asteroid_big_image, 48, 48, 20, 24.0f, 24.0f },
    { &asteroid_mid_image, 32, 32, 20, 16.0f, 16.0f },
    { &asteroid_small_image, 16, 16, 20, 8.0f, 8.0f }
};

void init_asteroid(struct asteroid *ast, enum asteroid_type type,
                   float x, float y, float v, float dir, int z, float time)
{
    ast->type = type;
    ast->x = x;
    ast->y = y;
    ast->v = v;
    ast->dir = dir;

    init_sprite(&ast->sprite, &asteroid_classes[type],
                world_to_screen_x(x),
                world_to_screen_y(y), z, 0);

    sprite_set_animation(&ast->sprite, &asteroid_animations[type], time);

    elist_link(&ast->link, &ast->link);
}

void destroy_asteroid(struct asteroid *ast)
{
    elist_remove(&ast->link);
}

struct asteroid *create_asteroid(enum asteroid_type type, float x, float y,
                                 float v, float dir, int z, float time)
{
    struct asteroid *ast = alloc_asteroid();
    init_asteroid(ast, type, x, y, v, dir, z, time);
    return ast;
}

void delete_asteroid(struct asteroid *ast)
{
    destroy_asteroid(ast);
    free_asteroid(ast);
}

void asteroid_set_type(struct asteroid *ast, enum asteroid_type type, float time)
{
    ast->type = type;
    const struct sprite_class *class = &asteroid_classes[type];
    *((struct sprite_class *) &ast->sprite) = *class;
    sprite_set_animation(&ast->sprite, &asteroid_animations[type], time);
}

void asteroid_cleanup(void)
{
    free_all_asteroid_allocs();
}
