#ifndef MISSILE_H
#define MISSILE_H

#include "alloc.h"
#include "elist.h"
#include "sprite.h"

#define MISSILE_SPEED                   10000
#define MISSILE_COLLISION_RADIUS        4

struct missile {
    float x;
    float y;
    float vx;
    float vy;
    struct sprite sprite;
    struct elist_node link;
};

DECLARE_ALLOCATOR(missile, struct missile);

#define missile_list_for_each(__missile, __list) \
    elist_for_each_elem((__missile), (__list), struct missile, link)

#define missile_list_get(__node) \
    elist_get(__node, struct missile, link)

void init_missile(struct missile *mis, float x, float y, float vx, float vy, int z);
void destroy_missile(struct missile *mis);
struct missile *create_missile(float x, float y, float vx, float vy, int z);
void delete_missile(struct missile *mis);
void missile_cleanup();

#endif
