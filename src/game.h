#ifndef GAME_H
#define GAME_H

#include "gfx.h"

extern struct gfx_mode_info gfx_mode_info;

int game_init(void);
void game_exit(void);
void game_run(void);

#endif
