#ifndef ASTEROID_H
#define ASTEROID_H

#include "alloc.h"
#include "elist.h"
#include "sprite.h"

#include <stdbool.h>

struct animation;
struct sprite_class;
struct sprite;

extern const struct animation asteroid_animations[];
extern const struct sprite_class asteroid_classes[];

enum asteroid_type {
    BIG_ASTEROID,
    MID_ASTEROID,
    SMALL_ASTEROID
};

struct asteroid {
    enum asteroid_type type;
    float x;
    float y;
    float v;
    float dir;
    struct sprite sprite;
    struct elist_node link;
};

DECLARE_ALLOCATOR(asteroid, struct asteroid);

#define asteroid_list_for_each(__asteroid, __list) \
    elist_for_each_elem((__asteroid), (__list), struct asteroid, link)

#define asteroid_list_get(__node) \
    elist_get(__node, struct asteroid, link)

void init_asteroid(struct asteroid *ast, enum asteroid_type type,
                   float x, float y, float v, float dir, int z, float time);
void destroy_asteroid(struct asteroid *ast);
struct asteroid *create_asteroid(enum asteroid_type type, float x, float y,
                                 float v, float dir, int z, float time);
void delete_asteroid(struct asteroid *ast);
void asteroid_set_type(struct asteroid *ast, enum asteroid_type type, float time);
void asteroid_cleanup(void);

#endif
