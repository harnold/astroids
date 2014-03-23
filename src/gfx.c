#include "gfx.h"
#include "bits.h"
#include "dpmi.h"
#include "error.h"
#include "image.h"
#include "palette.h"
#include "sprite.h"
#include "timer.h"
#include "vga.h"
#include "xmemcpy.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define REFRESH_RATE_TEST_CYCLES	30
#define MAX_REFRESH_RATE                100.0f

static struct {
    struct gfx_mode_info mode_info;
    int saved_vga_mode;
    struct image front_buffer;
    struct image back_buffer;
    int clip_x;
    int clip_y;
    int clip_w;
    int clip_h;
} gfx;

static void gfx_check_refresh_rate(void)
{
    timer_get_time();

    for (int i = 0; i < REFRESH_RATE_TEST_CYCLES; i++)
        vga_wait_for_retrace();

    float delta = timer_get_time_delta();

    if (delta > REFRESH_RATE_TEST_CYCLES / MAX_REFRESH_RATE) {
        gfx.mode_info.refresh_rate = REFRESH_RATE_TEST_CYCLES / delta;
        gfx.mode_info.vsync_supported = true;
    } else {
        gfx.mode_info.refresh_rate = -1;
        gfx.mode_info.vsync_supported = false;
    }
}

void gfx_init()
{
    vga_get_mode(&gfx.saved_vga_mode);
    gfx.mode_info.mode = gfx.saved_vga_mode;

    vga_set_mode(VGA_MODE_320x200_256);
    gfx.mode_info.mode = VGA_MODE_320x200_256;
    gfx.mode_info.x_resolution = 320;
    gfx.mode_info.y_resolution = 200;
    gfx.mode_info.page_size = 320 * 200;

    init_image(&gfx.front_buffer,
               gfx.mode_info.x_resolution,
               gfx.mode_info.y_resolution,
               vga_video_buffer());

    init_image(&gfx.back_buffer,
               gfx.mode_info.x_resolution,
               gfx.mode_info.y_resolution,
               NULL);

    xmemset(gfx.back_buffer.data, 0, gfx.mode_info.page_size);

    gfx_check_refresh_rate();
    gfx_reset_clip_rect();
}

void gfx_exit(void)
{
    destroy_image(&gfx.back_buffer);
    vga_set_mode(gfx.saved_vga_mode);
}

void gfx_get_mode_info(struct gfx_mode_info *info)
{
    *info = gfx.mode_info;
}

void gfx_get_clip_rect(int *x, int *y, int *w, int *h)
{
    *x = gfx.clip_x;
    *y = gfx.clip_y;
    *w = gfx.clip_w;
    *h = gfx.clip_h;
}

void gfx_set_clip_rect(int x, int y, int w, int h)
{
    gfx.clip_x = x;
    gfx.clip_y = y;
    gfx.clip_w = w;
    gfx.clip_h = h;
}

void gfx_reset_clip_rect(void)
{
    gfx.clip_x = 0;
    gfx.clip_y = 0;
    gfx.clip_w = gfx.mode_info.x_resolution;
    gfx.clip_h = gfx.mode_info.y_resolution;
}

bool gfx_clip(int *x, int *y, int *w, int *h)
{
    int xs = *x;
    int ys = *y;
    int xe = xs + *w - 1;
    int ye = ys + *h - 1;
    int clip_xe = gfx.clip_x + gfx.clip_w - 1;
    int clip_ye = gfx.clip_y + gfx.clip_h - 1;

    /* Completely inside the clipping rectangle? */

    if (xs >= gfx.clip_x && xe <= clip_xe &&
        ys >= gfx.clip_y && ye <= clip_ye)
        return true;

    /* Completely outside the clipping rectangle? */

    if (xs > clip_xe || xe < gfx.clip_x ||
        ys > clip_ye || ye < gfx.clip_y)
        return false;

    /* Partially inside the clipping rectangle. */

    if (xs < gfx.clip_x)
        xs = gfx.clip_x;
    if (ys < gfx.clip_y)
        ys = gfx.clip_y;
    if (xe > clip_xe)
        xe = clip_xe;
    if (ye > clip_ye)
        ye = clip_ye;

    *x = xs;
    *y = ys;
    *w = xe - xs + 1;
    *h = ye - ys + 1;

    return true;
}

void gfx_draw_back_buffer(void)
{
    vga_wait_for_retrace();

    image_blit(&gfx.back_buffer, 0, 0,
               gfx.mode_info.x_resolution,
               gfx.mode_info.y_resolution,
               &gfx.front_buffer, 0, 0,
               IMAGE_BLIT_COPY);
}

void gfx_draw_image_section(const struct image *image, int src_x, int src_y,
                            int src_w, int src_h, int dst_x, int dst_y,
                            unsigned flags)
{
    if (test_bit(flags, GFX_NO_CLIPPING)) {
        image_blit(image, src_x, src_y, src_w, src_h,
                   &gfx.back_buffer, dst_x, dst_y, flags & IMAGE_BLIT_MASK);
    } else {
        int cx = dst_x;
        int cy = dst_y;
        int cw = src_w;
        int ch = src_h;

        if (!gfx_clip(&cx, &cy, &cw, &ch))
            return;

        int dx = (cx - dst_x);
        int dy = (cy - dst_y);

        image_blit(image, src_x + dx, src_y + dy, cw, ch, &gfx.back_buffer,
                   dst_x + dx, dst_y + dy, flags & IMAGE_BLIT_MASK);
    }
}

void gfx_draw_image(const struct image *image, int x, int y, unsigned flags)
{
    gfx_draw_image_section(image, 0, 0, image->width, image->height, x, y, flags);
}

void gfx_draw_sprite(const struct sprite *sprite, unsigned flags)
{
    gfx_draw_image_section(sprite->image, 0, sprite->frame * sprite->height,
                           sprite->width, sprite->height,
                           sprite_get_x(sprite), sprite_get_y(sprite),
                           flags);
}

bool gfx_sprite_visible(const struct sprite *sprite)
{
    int xs = sprite_get_x(sprite);
    int ys = sprite_get_y(sprite);
    int xe = xs + sprite->width - 1;
    int ye = ys + sprite->height - 1;

    int clip_xe = gfx.clip_x + gfx.clip_w - 1;
    int clip_ye = gfx.clip_y + gfx.clip_h - 1;

    return (xs <= clip_xe && xe >= gfx.clip_x &&
            ys <= clip_ye && ye >= gfx.clip_y);
}

void gfx_fade_out(void)
{
    for (int level = 0; level < VGA_NUM_COLOR_LEVELS; level++) {

        vga_wait_for_retrace();

        for (int i = 0; i < VGA_NUM_COLORS; i++) {

            rgb_t c;
            vga_get_color(i, &c);

            uint8_t r = rgb_r(c);
            uint8_t g = rgb_g(c);
            uint8_t b = rgb_b(c);

            if (r > 0)
                r--;
            if (g > 0)
                g--;
            if (b > 0)
                b--;

            vga_set_color(i, make_rgb(r, g, b));
        }
    }
}

void gfx_fade_in(const struct palette *pal)
{
    for (int level = 0; level < VGA_NUM_COLOR_LEVELS; level++) {

        vga_wait_for_retrace();

        const uint8_t *p = pal->data;

        for (int i = 0; i < VGA_NUM_COLORS; i++) {

            rgb_t c;
            vga_get_color(i, &c);

            uint8_t r = rgb_r(c);
            uint8_t g = rgb_g(c);
            uint8_t b = rgb_b(c);

            if (r < *p++)
                r++;
            if (g < *p++)
                g++;
            if (b < *p++)
                b++;

            vga_set_color(i, make_rgb(r, g, b));
        }
    }
}
