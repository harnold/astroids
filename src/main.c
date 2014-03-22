#include "error.h"
#include "game.h"
#include "gfx.h"
#include "keyb.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GAME_TIMER_HZ   120.0f

static void cleanup(void)
{
    game_exit();
    gfx_exit();
    timer_exit();
    keyboard_exit();
}

int main(void)
{
    error_set_log_file(stdout);

    if (atexit(cleanup) != 0)
        return error("Registering cleanup function failed");

    srand((unsigned int) time(NULL));

    if (keyboard_init() != 0)
        return error("Initializing keyboard failed");

    if (timer_init(GAME_TIMER_HZ) != 0)
        return error("Initializing timer failed");

    gfx_init();

    if (game_init() != 0)
        return error("Initializing game failed");

    game_run();

    return EXIT_SUCCESS;
}
