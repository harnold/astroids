#include "palette.h"
#include "error.h"
#include "pcx.h"

#include <string.h>

int load_palette(const char *path, struct palette *palette)
{
    const char *ext = strrchr(path, '.');

    if (!ext)
        return error("File '%s' has no known image or palette file format", path);

    if (strcmp(ext, ".pcx") == 0)
        return pcx_load_palette(path, palette);
    else
        return error("File '%s' has no known image or palette file format", path);
}
