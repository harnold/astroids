#include "vga.h"
#include "palette.h"

#include <conio.h>
#include <i86.h>
#include <string.h>

#define VGA_INT                 0x10
#define VGA_DAC_READ_ADDRESS    0x3C7
#define VGA_DAC_WRITE_ADDRESS   0x3C8
#define VGA_DAC_DATA            0x3C9
#define VGA_INPUT_STATUS_1      0x3DA

void vga_get_mode(int *mode)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.w.ax = 0x0F00;
    int386(VGA_INT, &regs, &regs);
    *mode = regs.h.al;
}

void vga_set_mode(int mode)
{
    union REGS regs;

    memset(&regs, 0, sizeof(regs));
    regs.h.al = (uint8_t) mode;
    int386(VGA_INT, &regs, &regs);
}

void vga_set_color(int index, rgb_t rgb)
{
    outp(VGA_DAC_WRITE_ADDRESS, index);
    outp(VGA_DAC_DATA, rgb_r(rgb));
    outp(VGA_DAC_DATA, rgb_g(rgb));
    outp(VGA_DAC_DATA, rgb_b(rgb));
}

void vga_get_color(int index, rgb_t *rgb)
{
    unsigned int r, g, b;

    outp(VGA_DAC_READ_ADDRESS, index);
    r = inp(VGA_DAC_DATA);
    g = inp(VGA_DAC_DATA);
    b = inp(VGA_DAC_DATA);
    *rgb = make_rgb(r, g, b);
}

void vga_set_palette_data(int start, int count, const uint8_t *data)
{
    const uint8_t *p = data;

    outp(VGA_DAC_WRITE_ADDRESS, start);
    for (int i = 0; i < count; i++) {
        outp(VGA_DAC_DATA, *p++);
        outp(VGA_DAC_DATA, *p++);
        outp(VGA_DAC_DATA, *p++);
    }
}

void vga_set_palette(struct palette *pal)
{
    vga_set_palette_data(0, VGA_NUM_COLORS, pal->data);
}

void vga_wait_for_retrace(void)
{
    while ((inp(VGA_INPUT_STATUS_1) & 8) != 0)
        ;
    while ((inp(VGA_INPUT_STATUS_1) & 8) == 0)
        ;
}
