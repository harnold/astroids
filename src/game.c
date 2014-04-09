#include "game.h"
#include "asteroid.h"
#include "compat.h"
#include "keyb.h"
#include "res.h"
#include "scene.h"
#include "sprite.h"
#include "timer.h"
#include "vga.h"
#include "world.h"

#include <math.h>

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
#define ASTEROID_MIN_SPEED      200
#define ASTEROID_MAX_SPEED      3000

#define PLAYER_NAME_MAX         16

#define TIME_PER_LEVEL          10

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

static void create_asteroids(float time)
{
    if (game.num_asteroids >= ASTEROIDS_MAX)
        return;

    if (game.num_asteroids >= game.level)
        return;

    float d = frand() * FLOAT_2PI;
    float v = ASTEROID_MIN_SPEED + frand() * (ASTEROID_MAX_SPEED - ASTEROID_MIN_SPEED);
    float vx = v * sin(d);
    float vy = v * cos(d);

    float x;
    float y;

    if (vy > 0) {
        y = WORLD_MIN_Y -
            screen_to_world_dy(asteroid_classes[BIG_ASTEROID].height / 2);
    } else {
        y = WORLD_MAX_Y +
            screen_to_world_dy(asteroid_classes[BIG_ASTEROID].height / 2);
    }

    if (vx > 0) {
        x = WORLD_MIN_X + frand() * WORLD_SIZE_X / 2;
    } else {
        x = WORLD_MAX_X - frand() * WORLD_SIZE_X / 2;
    }

    struct asteroid *ast = create_asteroid(BIG_ASTEROID, x, y, vx, vy,
                                           ASTEROIDS_LAYER, time);

    elist_insert_back(&ast->link, &game.asteroids);
    scene_add_sprite(&game.scene, &ast->sprite);
    ++game.num_asteroids;
}

static void update_asteroids(float dt)
{
    struct elist_node *node, *tmp;

    elist_for_each_node_safe(node, tmp, &game.asteroids) {

        struct asteroid *ast = asteroid_list_get(node);

        ast->x += dt * ast->vx;
        ast->y += dt * ast->vy;

        ast->sprite.x = world_to_screen_x(ast->x);
        ast->sprite.y = world_to_screen_y(ast->y);

        if ((ast->x < WORLD_MIN_X && ast->vx < 0) ||
            (ast->x > WORLD_MAX_X && ast->vx > 0) ||
            (ast->y < WORLD_MIN_Y && ast->vy < 0) ||
            (ast->y > WORLD_MAX_Y && ast->vy > 0)) {

            if (!gfx_sprite_visible(&ast->sprite)) {
                delete_asteroid(ast);
                --game.num_asteroids;
            }
        }
    }
}

static void delete_asteroids(void)
{
    struct elist_node *node, *tmp;

    elist_for_each_node_safe(node, tmp, &game.asteroids)
        delete_asteroid(asteroid_list_get(node));
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
    delete_asteroids();
    destroy_scene(&game.scene);
}

static unsigned int game_loop(void)
{
    bool quit = false;

    const float start_time = timer_get_time();
    float time = start_time;
    float level_time = 0;

    while (!quit) {

        if (key_pressed(KEY_ESC))
            break;

        float dt = timer_get_time_delta();
        time += dt;

        level_time += dt;

        if (level_time >= TIME_PER_LEVEL) {
            ++game.level;
            level_time = 0;
        }

        update_asteroids(dt);
        create_asteroids(time);

        scene_update(&game.scene, time, dt);
        scene_draw(&game.scene);
        update_energy_display();
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
