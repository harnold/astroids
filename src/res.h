#ifndef RES_H
#define RES_H

#include "image.h"
#include "palette.h"

extern struct palette title_palette;
extern struct palette game_palette;
extern struct image background_image;
extern struct image asteroid_big_image;
extern struct image asteroid_mid_image;
extern struct image asteroid_small_image;
extern struct image explosion_image;
extern struct image font_image;
extern struct image game_over_image;
extern struct image hall_of_fame_image;
extern struct image missile_image;
extern struct image game_paused_image;
extern struct image shield_image;
extern struct image ship_image;
extern struct image title_image;

int res_load_palettes(void);
int res_load_images(void);
void res_destroy_images(void);

#endif
