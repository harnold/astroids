#include "vga.h"
#include "dpmi.h"
#include "palette.h"

#include <conio.h>
#include <i86.h>
#include <string.h>

#define VGA_INT                 0x10
#define VGA_DAC_READ_ADDRESS    0x3C7
#define VGA_DAC_WRITE_ADDRESS   0x3C8
#define VGA_DAC_DATA            0x3C9
#define VGA_INPUT_STATUS_1      0x3DA
#define VGA_VIDEO_BUFFER_SEG    0xA000

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

void vga_set_rgb(int index, uint8_t r, uint8_t g, uint8_t b)
{
    outp(VGA_DAC_WRITE_ADDRESS, index);
    outp(VGA_DAC_DATA, r);
    outp(VGA_DAC_DATA, g);
    outp(VGA_DAC_DATA, b);
}

void vga_get_rgb(int index, uint8_t *r, uint8_t *g, uint8_t *b)
{
    outp(VGA_DAC_READ_ADDRESS, index);
    *r = inp(VGA_DAC_DATA);
    *g = inp(VGA_DAC_DATA);
    *b = inp(VGA_DAC_DATA);
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
    uint8_t r, g, b;

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

void vga_set_black_palette(void)
{
    outp(VGA_DAC_WRITE_ADDRESS, 0);
    for (int i = 0; i < VGA_NUM_COLORS; i++) {
        outp(VGA_DAC_DATA, 0);
        outp(VGA_DAC_DATA, 0);
        outp(VGA_DAC_DATA, 0);
    }
}

uint8_t *vga_video_buffer(void)
{
    return dpmi_ptr_to_rm_segment(VGA_VIDEO_BUFFER_SEG);
}

void vga_wait_for_retrace(void)
{
    while ((inp(VGA_INPUT_STATUS_1) & 8) != 0)
        ;
    while ((inp(VGA_INPUT_STATUS_1) & 8) == 0)
        ;
}
