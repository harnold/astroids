#ifndef SCENE_H
#define SCENE_H

#include "elist.h"

struct image;
struct sprite;

struct scene {
    struct image *background;
    struct elist sprite_list;
    struct elist damage_list;
};

void init_scene(struct scene *scene);
void destroy_scene(struct scene *scene);
void scene_set_background(struct scene *scene, struct image *image);
void scene_add_sprite(struct scene *scene, struct sprite *sprite);
void scene_remove_sprite(struct scene *scene, struct sprite *sprite);
void scene_update(struct scene *scene, float t, float dt);
void scene_draw(struct scene *scene);
void scene_cleanup(void);

#endif
