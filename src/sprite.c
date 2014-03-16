#include "sprite.h"

#include <stdlib.h>

DEFINE_ALLOCATOR(sprite, struct sprite, ALLOC_DEFAULT_BLOB_SIZE);

void init_sprite_class(struct sprite_class *class, struct image *image,
                      int width, int height, int num_frames,
                      float origin_x, float origin_y)
{
    class->image = image;
    class->width = width;
    class->height = height;
    class->num_frames = num_frames;
    class->origin_x = origin_x;
    class->origin_y = origin_y;
}

void init_sprite(struct sprite *sprite, const struct sprite_class *class,
                 float x, float y, int z, int frame)
{
    *((struct sprite_class *) sprite) = *class;
    sprite->x = x;
    sprite->y = y;
    sprite->z = z;
    sprite->frame = frame;
    sprite->anim = NULL;
    sprite->anim_start_time = 0.0f;
    elist_link(&sprite->link, &sprite->link);
}

void destroy_sprite(struct sprite *sprite)
{
    elist_remove(&sprite->link);
}

struct sprite *create_sprite(const struct sprite_class *class, float x, float y,
                             int z, int frame)
{
    struct sprite *sprite = alloc_sprite();
    init_sprite(sprite, class, x, y, z, frame);
    return sprite;
}

void delete_sprite(struct sprite *sprite)
{
    destroy_sprite(sprite);
    free_sprite(sprite);
}

void sprite_set_animation(struct sprite *sprite, const struct animation *anim,
                          float start_time)
{
    sprite->anim = anim;
    sprite->anim_start_time = start_time;
}

void sprite_update(struct sprite *sprite, float t, float dt)
{
    const struct animation *anim = sprite->anim;

    if (anim == NULL)
        return;

    int nframe = (int) ((t - sprite->anim_start_time) * anim->fps);

    switch (anim->type) {

    case ANIM_ROTATE_FORWARD:
        sprite->frame = anim->start_frame +
            nframe % (anim->end_frame - anim->start_frame + 1);
        break;

    case ANIM_ROTATE_BACKWARD:
        sprite->frame = anim->start_frame -
            nframe % (anim->start_frame - anim->end_frame + 1);
        break;

    case ANIM_ONCE_FORWARD:
        sprite->frame = min(anim->start_frame + nframe, anim->end_frame);
        break;

    case ANIM_ONCE_BACKWARD:
        sprite->frame = max(anim->start_frame - nframe, anim->end_frame);
        break;
    }
}

void sprite_cleanup(void)
{
    free_all_sprite_allocs();
}
