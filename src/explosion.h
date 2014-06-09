#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "alloc.h"
#include "elist.h"
#include "sprite.h"

extern const struct animation explosion_animation;
extern const float explosion_duration;

struct explosion {
    float x;
    float y;
    struct sprite sprite;
    struct elist_node link;
};

DECLARE_ALLOCATOR(explosion, struct explosion);

#define explosion_list_for_each(__explosion, __list) \
    elist_for_each_elem((__explosion), (__list), struct explosion, link)

#define explosion_list_get(__node) \
    elist_get((__node), struct explosion, link)

void init_explosion(struct explosion *exp, float x, float y, int z, float time);
void destroy_explosion(struct explosion *exp);
struct explosion *create_explosion(float x, float y, int z, float time);
void delete_explosion(struct explosion *exp);
void explosion_cleanup(void);

#endif
