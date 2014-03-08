#ifndef PCX_H
#define PCX_H

struct palette;
struct image;

int pcx_load_palette(const char *path, struct palette *palette);
int pcx_load_image(const char *path, struct image *image);

#endif
