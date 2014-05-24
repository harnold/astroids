#include "asteroid.h"
#include "res.h"
#include "world.h"

DEFINE_ALLOCATOR(asteroid, struct asteroid, ALLOC_DEFAULT_BLOB_SIZE);

#define ASTEROID_FORWARD    0
#define ASTEROID_BACKWARD   (SMALL_ASTEROID + 1)

const struct animation asteroid_animations[] = {
    { ANIM_ROTATE_FORWARD, 20, 0, 19 },
    { ANIM_ROTATE_FORWARD, 20, 0, 19 },
    { ANIM_ROTATE_FORWARD, 20, 0, 19 },
    { ANIM_ROTATE_BACKWARD, 20, 19, 0 },
    { ANIM_ROTATE_BACKWARD, 20, 19, 0 },
    { ANIM_ROTATE_BACKWARD, 20, 19, 0 },
};

const struct sprite_class asteroid_classes[] = {
    { &asteroid_big_image, 48, 48, 20, 24.0f, 24.0f },
    { &asteroid_mid_image, 32, 32, 20, 16.0f, 16.0f },
    { &asteroid_small_image, 16, 16, 20, 8.0f, 8.0f }
};

void init_asteroid(struct asteroid *ast, enum asteroid_type type,
                   float x, float y, float vx, float vy, int z, float time)
{
    ast->type = type;
    ast->x = x;
    ast->y = y;
    ast->vx = vx;
    ast->vy = vy;

    init_sprite(&ast->sprite, &asteroid_classes[type],
                world_to_screen_x(x),
                world_to_screen_y(y), z, 0);

    const struct animation *anim;

    if (vy > 0)
        anim = &asteroid_animations[ASTEROID_FORWARD + type];
    else
        anim = &asteroid_animations[ASTEROID_BACKWARD + type];

    sprite_set_animation(&ast->sprite, anim, time);
    elist_link(&ast->link, &ast->link);
}

void destroy_asteroid(struct asteroid *ast)
{
    elist_remove(&ast->link);
    destroy_sprite(&ast->sprite);
}

struct asteroid *create_asteroid(enum asteroid_type type, float x, float y,
                                 float vx, float vy, int z, float time)
{
    struct asteroid *ast = alloc_asteroid();
    init_asteroid(ast, type, x, y, vx, vy, z, time);
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
