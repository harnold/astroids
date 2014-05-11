#include "scene.h"
#include "alloc.h"
#include "image.h"
#include "gfx.h"
#include "sprite.h"

struct damage_rect {
    int x, y, w, h;
    struct elist_node link;
};

DEFINE_ALLOCATOR(damage_rect, struct damage_rect, ALLOC_DEFAULT_BLOB_SIZE);

#define damage_list_for_each(__rect, __list) \
    elist_for_each_elem(__rect, __list, struct damage_rect, link)

#define damage_list_get(__node) \
    elist_get(__node, struct damage_rect, link)

inline static struct damage_rect *create_damage_rect(int x, int y, int w, int h,
                                                     struct elist *list)
{
    struct damage_rect *r = alloc_damage_rect();
    r->x = x; r->y = y; r->w = w; r->h = h;
    elist_insert_back(&r->link, list);
    return r;
}

inline static void delete_damage_rect(struct damage_rect *r)
{
    elist_remove(&r->link);
    free_damage_rect(r);
}

static void clear_damage_list(struct elist *list)
{
    struct elist_node *n, *tmp;

    elist_for_each_node_safe(n, tmp, list)
        delete_damage_rect(damage_list_get(n));
}

void init_scene(struct scene *scene)
{
    scene->background = NULL;
    init_elist(&scene->sprite_list);
    init_elist(&scene->damage_list);
}

void destroy_scene(struct scene *scene)
{
    clear_damage_list(&scene->damage_list);
}

void scene_set_background(struct scene *scene, struct image *image)
{
    scene->background = image;

    gfx_draw_image(scene->background, 0, 0, GFX_NO_CLIPPING);
    clear_damage_list(&scene->damage_list);
}

void scene_add_sprite(struct scene *scene, struct sprite *sprite)
{
    struct elist_node *node = elist_begin(&scene->sprite_list);

    while (node != elist_end(&scene->sprite_list) &&
           sprite_list_get(node)->z >= sprite->z) {
        node = node->next;
    }

    elist_insert(&sprite->link, node);
}

void scene_insert_sprite(struct scene *scene, struct sprite *prev,
                         struct sprite *sprite)
{
    elist_insert(&sprite->link, prev->link.next);
}

void scene_remove_sprite(struct scene *scene, struct sprite *sprite)
{
    elist_remove(&sprite->link);
}

void scene_add_damage_rect(struct scene *scene, int x, int y, int w, int h)
{
    create_damage_rect(x, y, w, h, &scene->damage_list);
}

void scene_update(struct scene *scene, float t, float dt)
{
    struct sprite *sprite;

    sprite_list_for_each(sprite, &scene->sprite_list) {
        sprite_update(sprite, t, dt);
    }
}

void scene_draw(struct scene *scene)
{
    struct damage_rect *r;

    damage_list_for_each(r, &scene->damage_list) {
        gfx_draw_image_section(scene->background, r->x, r->y, r->w, r->h,
                               r->x, r->y, IMAGE_BLIT_COPY);
    }

    clear_damage_list(&scene->damage_list);

    struct sprite *sprite;

    sprite_list_for_each(sprite, &scene->sprite_list) {
        gfx_draw_sprite(sprite, IMAGE_BLIT_MASK);
        create_damage_rect(sprite_get_x(sprite), sprite_get_y(sprite),
                           sprite->width, sprite->height, &scene->damage_list);
    }
}

void scene_cleanup(void)
{
    free_all_damage_rect_allocs();
}
