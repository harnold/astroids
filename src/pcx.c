#include "pcx.h"
#include "error.h"
#include "image.h"
#include "palette.h"
#include "rgb.h"
#include "xmalloc.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PCX_HEADER_SIZE         128
#define PCX_PALETTE_SIZE        768

#define PCX_MANUFACTURER        0
#define PCX_VERSION             1
#define PCX_ENCODING            2
#define PCX_BITS_PER_PIXEL      3
#define PCX_X1                  4
#define PCX_Y1                  6
#define PCX_X2                  8
#define PCX_Y2                  10
#define PCX_HRES                12
#define PCX_VRES                14
#define PCX_PALETTE             16
#define PCX_RESERVED_1          64
#define PCX_COLOR_PLANES        65
#define PCX_BYTES_PER_LINE      66
#define PCX_PALETTE_TYPE        68
#define PCX_RESERVED_2          70

static inline unsigned int read_le16(uint8_t *p)
{
    return ((unsigned int) p[1] << 8) | p[0];
}

int pcx_load_palette(const char *path, struct palette *palette)
{
    FILE *file;

    if (!(file = fopen(path, "rb"))) {
        error_errno("Opening file '%s' failed", path);
        goto failure;
    }

    if (fseek(file, -PCX_PALETTE_SIZE, SEEK_END) != 0) {
        error_errno("Seeking in file '%s' failed", path);
        goto failure;
    }

    if (fread(palette->data, PCX_PALETTE_SIZE, 1, file) != 1) {
        error_errno("Reading palette from file '%s' failed", path);
        goto failure;
    }

    fclose(file);

    uint8_t *p = palette->data;
    uint8_t *end = p + PCX_PALETTE_SIZE;

    while (p < end)
        *p++ >>= 2;

    return 0;

failure:

    if (file)
        fclose(file);
    return -1;
}

int pcx_load_image(const char *path, struct image *image)
{
    FILE *file;
    uint8_t *data = NULL;

    if (!(file = fopen(path, "rb"))) {
        error_errno("Opening file '%s' failed", path);
        goto failure;
    }

    uint8_t header[PCX_HEADER_SIZE];

    if (fread(header, PCX_HEADER_SIZE, 1, file) != 1) {
        error_errno("Reading PCX header from file '%s' failed", path);
        goto failure;
    }

    int width = read_le16(header + PCX_X2) - read_le16(header + PCX_X1) + 1;
    int height = read_le16(header + PCX_Y2) - read_le16(header + PCX_Y1) + 1;
    int size = width * height;

    data = xmalloc(size);

    uint8_t *p = data;
    int pixel, count;

    for (int i = 0; i < size; ) {
        if ((pixel = fgetc(file)) == EOF) {
            error_errno("Reading image data from file '%s' failed", path);
            goto failure;
        }
        if (pixel < 192) {
            *p++ = (uint8_t) pixel;
            i++;
        } else {
            count = pixel - 192;
            if ((pixel = fgetc(file)) == EOF) {
                error_errno("Reading image data from file '%s' failed", path);
                goto failure;
            }
            while (count-- > 0) {
                *p++ = (uint8_t) pixel;
                i++;
            }
        }
    }

    fclose(file);

    init_image(image, width, height, data);
    return 0;

failure:

    if (file)
        fclose(file);
    if (data)
        xfree(data);
    return -1;
}
