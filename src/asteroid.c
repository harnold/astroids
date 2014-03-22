#include "asteroid.h"
#include "res.h"
#include "sprite.h"
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
                   float x, float y, float vx, float vy, int z, float time)
{
    ast->type = type;
    ast->world_x = x;
    ast->world_y = y;
    ast->world_vx = vx;
    ast->world_vy = vy;

    struct sprite *spr = create_sprite(&asteroid_classes[type],
                                       world_to_screen_x(x),
                                       world_to_screen_y(y), z, 0);

    sprite_set_animation(spr, &asteroid_animations[type], time);

    ast->sprite = spr;
    elist_link(&ast->link, &ast->link);
}

void destroy_asteroid(struct asteroid *ast)
{
    elist_remove(&ast->link);
    delete_sprite(ast->sprite);
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
    *((struct sprite_class *) ast->sprite) = *class;
    sprite_set_animation(ast->sprite, &asteroid_animations[type], time);
}

void asteroid_cleanup(void)
{
    free_all_asteroid_allocs();
}
