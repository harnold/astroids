#include "game.h"
#include "asteroid.h"
#include "gfx.h"
#include "keyb.h"
#include "res.h"
#include "scene.h"
#include "sprite.h"
#include "timer.h"
#include "vga.h"
#include "world.h"

#define SCORE_LAYER             0
#define SHIP_LAYER              1
#define ASTEROIDS_LAYER         2

#define ENERGY_COLOR            253
#define ENERGY_X_POS            50
#define ENERGY_Y_POS            50
#define ENERGY_WIDTH            800
#define ENERGY_HEIGHT           80

#define SCORE_DIGITS            6
#define SCORE_X_POS             3350
#define SCORE_Y_POS             50

#define ASTEROIDS_MAX           50

#define PLAYER_NAME_MAX         16

struct ship {
    float energy;
    float x;
    float y;
    float v;
    float dir;
    struct sprite ship_sprite;
    struct sprite shield_sprite;
};

struct gfx_mode_info gfx_mode_info;

static const struct sprite_class font_class = {
    &font_image, 8, 8, 37, 0.0f, 0.0f
};

static struct {
    char name[PLAYER_NAME_MAX];
    unsigned int score;
    unsigned int level;
    struct scene scene;
    struct ship player_ship;
    struct elist asteroids;
    unsigned int num_asteroids;
    struct sprite score_sprites[SCORE_DIGITS];
} game;

static int frame_of_char(char c)
{
    return (c >= '0' && c <= '9') ? c - '0' :
           (c >= 'A' && c <= 'Z') ? c - 'A' + 10 :
           (c == ' ') ? 36 : -1;
}

static void update_number_display(struct sprite sprites[], int num_digits,
                                  unsigned int value)
{
    for (int i = num_digits - 1, shifter = 1; i >= 0; i--, shifter *= 10)
        sprites[i].frame = frame_of_char('0' + (value / shifter) % 10);
}

static void update_energy_display(void)
{
    unsigned x = world_to_screen_x(ENERGY_X_POS);
    unsigned y = world_to_screen_y(ENERGY_Y_POS);
    unsigned w = world_to_screen_dx(game.player_ship.energy * ENERGY_WIDTH);
    unsigned h = world_to_screen_dx(ENERGY_HEIGHT);

    gfx_draw_rect(x, y, w, h, ENERGY_COLOR, GFX_NO_CLIPPING);
}

static void create_asteroids(void)
{
    if (game.num_asteroids >= ASTEROIDS_MAX)
        return;
}

static void update_asteroids(void)
{
}

static void game_start(void)
{
    game.score = 0;
    game.level = 1;

    game.player_ship.energy = 1.0f;

    game.num_asteroids = 0;
    init_elist(&game.asteroids);

    init_scene(&game.scene);
    scene_set_background(&game.scene, &background_image);

    for (int i = 0; i < SCORE_DIGITS; i++) {
        init_sprite(&game.score_sprites[i], &font_class,
                    world_to_screen_x(SCORE_X_POS) + i * font_class.width,
                    world_to_screen_y(SCORE_Y_POS),
                    SCORE_LAYER, 0.0f);
        scene_add_sprite(&game.scene, &game.score_sprites[i]);
    }

    vga_set_black_palette();
    scene_draw(&game.scene);

    update_energy_display();

    gfx_draw_back_buffer();
    gfx_fade_in(&game_palette);
}

static void game_end(void)
{
    gfx_fade_out();
    destroy_scene(&game.scene);
}

static unsigned int game_loop(void)
{
    bool quit = false;

    const float start_time = timer_get_time();
    float time = start_time;

    while (!quit) {

        if (key_pressed(KEY_ESC))
            break;

        float dt = timer_get_time_delta();
        time += dt;

        scene_update(&game.scene, time, dt);
        scene_draw(&game.scene);
        gfx_draw_back_buffer();
    }

    return game.score;
}

int game_init(void)
{
    if (res_load_palettes() != 0)
        return -1;

    if (res_load_images() != 0) {
        res_destroy_images();
        return -1;
    }

    gfx_get_mode_info(&gfx_mode_info);

    return 0;
}

void game_exit(void)
{
    res_destroy_images();

    asteroid_cleanup();
    scene_cleanup();
    sprite_cleanup();
}

void game_run(void)
{
    game_start();
    game_loop();
    game_end();
}
