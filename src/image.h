#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

enum image_blit_mode {
    IMAGE_BLIT_COPY = 0,
    IMAGE_BLIT_MASK = 1,
};

struct image {
    uint8_t *data;
    int width;
    int height;
};

void init_image(struct image *image, int width, int height, uint8_t *data);
void destroy_image(struct image *image);
int load_image(const char *path, struct image *image);
void image_blit(const struct image *restrict src, int src_x, int src_y,
                int src_w, int src_h, struct image *restrict dst,
                int dst_x, int dst_y, unsigned flags);

#endif
