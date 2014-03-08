#ifndef PALETTE_H
#define PALETTE_H

#include <stdint.h>

struct palette {
    uint8_t data[768];
};

int load_palette(const char *path, struct palette *palette);

#endif
