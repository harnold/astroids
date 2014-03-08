#ifndef VGA_H
#define VGA_H

#include "rgb.h"

#define VGA_NUM_COLORS          256
#define VGA_NUM_COLOR_LEVELS    64

enum vga_mode {
    VGA_TEXT_MODE               = 0x03,
    VGA_MODE_320x200_256        = 0x13
};

struct palette;

void vga_get_mode(int *mode);
void vga_set_mode(int mode);
void vga_set_color(int index, rgb_t rgb);
void vga_get_color(int index, rgb_t *rgb);
void vga_set_palette_data(int start, int count, const uint8_t *data);
void vga_set_palette(struct palette *pal);
void vga_wait_for_retrace(void);

#endif
