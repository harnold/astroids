#include "res.h"
#include "error.h"

#include <stddef.h>

struct palette title_palette;
struct palette game_palette;
struct image background_image;
struct image asteroid_big_image;
struct image asteroid_mid_image;
struct image asteroid_small_image;
struct image explosion_image;
struct image font_image;
struct image game_over_image;
struct image hall_of_fame_image;
struct image missile_image;
struct image game_paused_image;
struct image shield_image;
struct image ship_image;
struct image title_image;

struct palette_info {
    struct palette *palette;
    const char *path;
};

struct image_info {
    struct image *image;
    const char *path;
};

static const struct palette_info palette_infos[] = {
    { &title_palette, "gfx\\title.pcx" },
    { &game_palette, "gfx\\ship.pcx" },
    { NULL, NULL },
};

static const struct image_info image_infos[] = {
    { &background_image, "gfx\\backgrnd.pcx" },
    { &asteroid_big_image, "gfx\\astbig.pcx" },
    { &asteroid_mid_image, "gfx\\astmid.pcx" },
    { &asteroid_small_image, "gfx\\astsmall.pcx" },
    { &explosion_image, "gfx\\explode.pcx" },
    { &font_image, "gfx\\font.pcx" },
    { &game_over_image, "gfx\\gameover.pcx" },
    { &hall_of_fame_image, "gfx\\hall.pcx" },
    { &missile_image, "gfx\\missile.pcx" },
    { &game_paused_image, "gfx\\pause.pcx" },
    { &shield_image, "gfx\\shield.pcx" },
    { &ship_image, "gfx\\ship.pcx" },
    { &title_image, "gfx\\title.pcx" },
    { NULL, NULL },
};

int res_load_palettes(void)
{
    const struct palette_info *info;

    for (info = palette_infos; info->palette != NULL; info++) {
        if (load_palette(info->path, info->palette) != 0)
            return error("Loading palette from file %s failed", info->path);
    }

    return 0;
}

int res_load_images(void)
{
    const struct image_info *info;

    for (info = image_infos; info->image != NULL; info++) {
        if (load_image(info->path, info->image) != 0)
            return error("Loading image from file %s failed", info->path);
    }

    return 0;
}

void res_destroy_images(void)
{
    const struct image_info *info;

    for (info = image_infos; info->image != NULL; info++) {
        if (info->image->data != NULL) {
            destroy_image(info->image);
            info->image->data = NULL;
        }
    }
}
