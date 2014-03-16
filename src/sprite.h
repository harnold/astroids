#ifndef SPRITE_H
#define SPRITE_H

#include "alloc.h"
#include "elist.h"

#include <stdint.h>

struct image;

enum animation_type {
    ANIM_ROTATE_FORWARD,
    ANIM_ROTATE_BACKWARD,
    ANIM_ONCE_FORWARD,
    ANIM_ONCE_BACKWARD
};

struct animation {
    enum animation_type type;
    float fps;
    int start_frame;
    int end_frame;
};

struct sprite_class {
    struct image *image;
    int width;
    int height;
    int num_frames;
    float origin_x;
    float origin_y;
};

struct sprite {
    struct image *image;
    int width;
    int height;
    int num_frames;
    float origin_x;
    float origin_y;
    float x;
    float y;
    int z;
    int frame;
    const struct animation *anim;
    float anim_start_time;
    struct elist_node link;
};

DECLARE_ALLOCATOR(sprite, struct sprite);

#define sprite_list_for_each(__sprite, __list) \
    elist_for_each_elem((__sprite), (__list), struct sprite, link)

#define sprite_list_get(__node) \
    elist_get(__node, struct sprite, link)

static inline int sprite_get_x(const struct sprite *sprite);
static inline int sprite_get_y(const struct sprite *sprite);

void init_sprite_class(struct sprite_class *class, struct image *image,
                       int width, int height, int num_frames,
                       float origin_x, float origin_y);
void init_sprite(struct sprite *sprite, const struct sprite_class *class,
                 float x, float y, int z, int frame);
void destroy_sprite(struct sprite *sprite);
struct sprite *create_sprite(const struct sprite_class *class, float x, float y,
                             int z, int frame);
void delete_sprite(struct sprite *sprite);
void sprite_set_animation(struct sprite *sprite, const struct animation *anim,
                          float start_time);
void sprite_update(struct sprite *sprite, float t, float dt);
void sprite_cleanup(void);

static inline int sprite_get_x(const struct sprite *sprite)
{
    return (int) (sprite->x - sprite->origin_x + 0.5f);
}

static inline int sprite_get_y(const struct sprite *sprite)
{
    return (int) (sprite->y - sprite->origin_y + 0.5f);
}

#endif
