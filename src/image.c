#include "image.h"
#include "error.h"
#include "pcx.h"
#include "xmalloc.h"
#include "xmemcpy.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void init_image(struct image *image, int width, int height, uint8_t *data)
{
    image->width = width;
    image->height = height;

    if (data != NULL)
        image->data = data;
    else
        image->data = xmalloc(width * height);
}

void destroy_image(struct image *image)
{
    xfree(image->data);
}

int load_image(const char *path, struct image *image)
{
    const char *ext = strrchr(path, '.');

    if (!ext)
        return error("File '%s' has no known image file format", path);

    if (strcmp(ext, ".pcx") == 0)
        return pcx_load_image(path, image);
    else
        return error("File '%s' has no known image file format", path);
}

static void image_blit_copy(const uint8_t *restrict src, int src_stride,
                            int src_w, int src_h, uint8_t *restrict dst,
                            int dst_stride)
{
    for (int i = 0; i < src_h; i++) {
        xmemcpy(dst, src, src_w);
        src += src_stride;
        dst += dst_stride;
    }
}

static void image_blit_mask(const uint8_t *restrict src, int src_stride,
                            int src_w, int src_h, uint8_t *restrict dst,
                            int dst_stride)
{
    int src_off = src_stride - src_w;
    int dst_off = dst_stride - src_w;

    for (int i = 0; i < src_h; i++) {

        for (int j = 0; j < src_w; j++) {
            if (*src != 0) {
                *dst++ = *src++;
            } else {
                src++;
                dst++;
            }
        }

        src += src_off;
        dst += dst_off;
    }
}

void image_blit(const struct image *restrict src, int src_x, int src_y,
                int src_w, int src_h, struct image *restrict dst,
                int dst_x, int dst_y, unsigned flags)
{
    int src_stride = src->width;
    int dst_stride = dst->width;

    uint8_t *sp = src->data + src_stride * src_y + src_x;
    uint8_t *dp = dst->data + dst_stride * dst_y + dst_x;

    if ((flags & IMAGE_BLIT_MASK) == 1)
        image_blit_mask(sp, src_stride, src_w, src_h, dp, dst_stride);
    else
        image_blit_copy(sp, src_stride, src_w, src_h, dp, dst_stride);
}
