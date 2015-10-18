#include "game.h"
#include "asteroid.h"
#include "compat.h"
#include "explosion.h"
#include "keyb.h"
#include "missile.h"
#include "res.h"
#include "scene.h"
#include "ship.h"
#include "sprite.h"
#include "timer.h"
#include "vga.h"
#include "world.h"

#include <math.h>

#define SCORE_LAYER             0
#define SHIP_LAYER              1
#define MISSILE_LAYER           2
#define EXPLOSIONS_LAYER        3
#define ASTEROIDS_LAYER         4

#define ENERGY_COLOR            253
#define ENERGY_X_POS            50
#define ENERGY_Y_POS            50
#define ENERGY_WIDTH            800
#define ENERGY_HEIGHT           100

#define SCORE_DIGITS            6
#define SCORE_X_POS             3350
#define SCORE_Y_POS             50

#define GAME_PAUSED_X           ((gfx_mode_info.x_resolution - game_paused_image.width) / 2)
#define GAME_PAUSED_Y           ((gfx_mode_info.y_resolution - game_paused_image.height) / 2)
#define GAME_OVER_X             ((gfx_mode_info.x_resolution - game_over_image.width) / 2)
#define GAME_OVER_Y             ((gfx_mode_info.y_resolution - game_over_image.height) / 2)
#define GAME_OVER_SPEED         3

#define ASTEROIDS_MAX           50
#define ASTEROID_MIN_DELAY      0.1f
#define ASTEROID_FRAGMENTS      4

#define PLAYER_NAME_MAX         16

#define TIME_PER_LEVEL          10
#define POINTS_PER_HIT          100
#define ENERGY_REFILL_SCORE     10000

#define NUM_FINAL_EXPLOSIONS    200
#define FINAL_EXPLOSIONS_DELAY  0.05f
#define DESTROYED_SHIP_TURN     (FLOAT_2PI)

struct gfx_mode_info gfx_mode_info;

static const struct sprite_class font_class = {
    &font_image, 8, 8, 37, 0.0f, 0.0f
};

static struct {
    char name[PLAYER_NAME_MAX];
    unsigned int score;
    unsigned int energy_refill_score;
    unsigned int level;
    struct scene scene;
    struct ship player_ship;
    struct elist asteroids;
    unsigned int num_asteroids;
    float last_asteroid_time;
    struct elist missiles;
    struct elist explosions;
    int num_final_explosions;
    float next_explosion_time;
} game;

static void update_score_display(void)
{
    char score_text[SCORE_DIGITS + 1];

    score_text[SCORE_DIGITS] = '\0';

    for (int i = SCORE_DIGITS - 1, shifter = 1; i >= 0; i--, shifter *= 10)
        score_text[i] = (game.score / shifter) % 10 + '0';

    int x = world_to_screen_x(SCORE_X_POS);
    int y = world_to_screen_y(SCORE_Y_POS);

    gfx_draw_text(&font_class, x, y, score_text, IMAGE_BLIT_MASK | GFX_NO_CLIPPING);
    scene_add_damage_rect(&game.scene, x, y, SCORE_DIGITS * font_class.width, font_class.height);
}

static void update_energy_display(void)
{
    unsigned x = world_to_screen_x(ENERGY_X_POS);
    unsigned y = world_to_screen_y(ENERGY_Y_POS);
    unsigned w_full = world_to_screen_dx(ENERGY_WIDTH);
    unsigned w = world_to_screen_dx(game.player_ship.energy * ENERGY_WIDTH);
    unsigned h = world_to_screen_dy(ENERGY_HEIGHT);

    gfx_draw_rect(x, y, w, h, ENERGY_COLOR, GFX_NO_CLIPPING);
    scene_add_damage_rect(&game.scene, x, y, w, h);
}

static void control_player_ship(float time, float dt)
{
    struct ship *ship = &game.player_ship;

    if (key_pressed(KEY_LEFT))
        ship_turn(ship, -dt * SHIP_TURN_PER_SEC);
    else if (key_pressed(KEY_RIGHT))
        ship_turn(ship, dt * SHIP_TURN_PER_SEC);

    if (key_pressed(KEY_UP))
        ship_set_power(ship, ship->engine_power + SHIP_ENGINE_POWER_INC);
    else
        ship_set_power(ship, 0);

    if (key_pressed(KEY_SPACE) && time > ship->last_shot + SHIP_FIRE_INTERVAL) {

        float dir = ship_get_visible_direction(ship);
        float vx = MISSILE_SPEED * sin(dir);
        float vy = MISSILE_SPEED * cos(dir);

        struct missile *mis = create_missile(ship->x, ship->y, vx, vy,
                                             MISSILE_LAYER);

        elist_insert_back(&mis->link, &game.missiles);
        scene_add_sprite(&game.scene, &mis->sprite);
        ship->last_shot = time;
    }
}

static void update_player_ship(float dt)
{
    struct ship *ship = &game.player_ship;

    float dv = dt * (ship->engine_power / SHIP_MASS);
    float dir = ship_get_visible_direction(ship);

    ship->vx += dv * sin(dir);
    ship->vy += -dv * cos(dir);
    ship->x += dt * ship->vx;
    ship->y += dt * ship->vy;

    if ((ship->vx < 0 && ship->x < WORLD_MIN_X) ||
        (ship->vx > 0 && ship->x > WORLD_MAX_X)) {
        ship->vx = -ship->vx;
    }

    if ((ship->vy < 0 && ship->y < WORLD_MIN_Y) ||
        (ship->vy > 0 && ship->y > WORLD_MAX_Y)) {
        ship->vy = -ship->vy;
    }

    ship_update_sprite(ship);
}

static void blow_up_asteroid(struct asteroid *ast, float time, struct elist *list)
{
    if (ast->type == BIG_ASTEROID || ast->type == MID_ASTEROID) {

        enum asteroid_type new_type =
            ast->type == BIG_ASTEROID ? MID_ASTEROID : SMALL_ASTEROID;

        float base_dir = frand() * FLOAT_2PI;

        for (int i = 0; i < ASTEROID_FRAGMENTS; i++) {

            float d = base_dir + i * FLOAT_2PI / 4.0f + (frand() - 0.5f) * FLOAT_2PI / 12.0f;
            float v = ASTEROID_MIN_SPEED + frand() * (ASTEROID_MAX_SPEED - ASTEROID_MIN_SPEED);
            float vx = v * sin(d);
            float vy = -v * cos(d);

            struct asteroid *new_ast = create_asteroid(
                new_type, ast->x, ast->y, vx, vy, ASTEROIDS_LAYER, time);

            elist_insert_back(&new_ast->link, list);
            scene_add_sprite(&game.scene, &new_ast->sprite);
            ++game.num_asteroids;
        }
    }

    struct explosion *exp = create_explosion(ast->x, ast->y, EXPLOSIONS_LAYER, time);

    elist_insert_back(&exp->link, &game.explosions);
    scene_add_sprite(&game.scene, &exp->sprite);

    delete_asteroid(ast);
    --game.num_asteroids;
}

static void create_asteroids(float time)
{
    if (game.num_asteroids >= ASTEROIDS_MAX ||
        game.num_asteroids >= game.level ||
        time < game.last_asteroid_time + ASTEROID_MIN_DELAY) {
        return;
    }

    float d = frand() * FLOAT_2PI;
    float v = ASTEROID_MIN_SPEED + frand() * (ASTEROID_MAX_SPEED - ASTEROID_MIN_SPEED);
    float vx = v * sin(d);
    float vy = -v * cos(d);

    float x;
    float y;

    if (vy > 0) {
        y = WORLD_MIN_Y -
            screen_to_world_dy(asteroid_classes[BIG_ASTEROID].height / 2) + 1;
    } else {
        y = WORLD_MAX_Y +
            screen_to_world_dy(asteroid_classes[BIG_ASTEROID].height / 2) - 1;
    }

    if (vx > 0)
        x = WORLD_MIN_X + frand() * WORLD_SIZE_X / 2;
    else
        x = WORLD_MAX_X - frand() * WORLD_SIZE_X / 2;

    struct asteroid *ast = create_asteroid(BIG_ASTEROID, x, y, vx, vy,
                                           ASTEROIDS_LAYER, time);

    elist_insert_back(&ast->link, &game.asteroids);
    scene_add_sprite(&game.scene, &ast->sprite);
    ++game.num_asteroids;
    game.last_asteroid_time = time;
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

static void update_missiles(float dt)
{
    struct elist_node *node, *tmp;

    elist_for_each_node_safe(node, tmp, &game.missiles) {

        struct missile *mis = missile_list_get(node);

        mis->x += dt * mis->vx;
        mis->y += -dt * mis->vy;

        mis->sprite.x = world_to_screen_x(mis->x);
        mis->sprite.y = world_to_screen_y(mis->y);

        if ((mis->x < WORLD_MIN_X && mis->vx < 0) ||
            (mis->x > WORLD_MAX_X && mis->vx > 0) ||
            (mis->y < WORLD_MIN_Y && mis->vy < 0) ||
            (mis->y > WORLD_MAX_Y && mis->vy > 0)) {

            if (!gfx_sprite_visible(&mis->sprite))
                delete_missile(mis);
        }
    }
}

static void delete_missiles(void)
{
    struct elist_node *node, *tmp;

    elist_for_each_node_safe(node, tmp, &game.missiles)
        delete_missile(missile_list_get(node));
}

static void update_explosions(float time, float dt)
{
    struct elist_node *node, *tmp;

    elist_for_each_node_safe(node, tmp, &game.explosions) {

        struct explosion *exp = explosion_list_get(node);

        if (time - exp->sprite.anim_start_time > EXPLOSION_DURATION)
            delete_explosion(exp);
    }
}

static void delete_explosions(void)
{
    struct elist_node *node, *tmp;

    elist_for_each_node_safe(node, tmp, &game.explosions)
        delete_explosion(explosion_list_get(node));
}

static void test_missile_asteroid_collisions(float time)
{
    struct elist new_asteroids;
    init_elist(&new_asteroids);

    struct elist_node *ast_node, *ast_tmp;

    elist_for_each_node_safe(ast_node, ast_tmp, &game.asteroids) {

        struct asteroid *ast = asteroid_list_get(ast_node);
        struct elist_node *mis_node, *mis_tmp;

        bool asteroid_hit = false;

        elist_for_each_node_safe(mis_node, mis_tmp, &game.missiles) {

            struct missile *mis = missile_list_get(mis_node);

            if (sprite_test_collision(
                    &mis->sprite, MISSILE_COLLISION_RADIUS,
                    &ast->sprite, asteroid_classes[ast->type].width / 2)) {

                delete_missile(mis);
                asteroid_hit = true;
            }
        }

        if (asteroid_hit) {
            game.score += POINTS_PER_HIT << (unsigned) ast->type;
            blow_up_asteroid(ast, time, &new_asteroids);
        }
    }

    elist_splice(elist_begin(&new_asteroids), elist_end(&new_asteroids),
                 elist_end(&game.asteroids));
}

static void test_asteroid_ship_collisions(float dt)
{
    int num_collisions = 0;

    struct asteroid *ast;

    asteroid_list_for_each(ast, &game.asteroids) {
        if (sprite_test_collision(
                &game.player_ship.ship_sprite, SHIP_COLLISION_RADIUS,
                &ast->sprite, asteroid_classes[ast->type].width / 2))
            ++num_collisions;
    }

    if (num_collisions > 0) {

        ship_set_shield(&game.player_ship, true);
        game.player_ship.energy -= dt * num_collisions;

        if (game.player_ship.energy < 0)
            game.player_ship.energy = 0;

    } else {
        ship_set_shield(&game.player_ship, false);
    }
}

static void test_collisions(float time, float dt)
{
    test_missile_asteroid_collisions(time);
    test_asteroid_ship_collisions(dt);
}

static void create_final_explosions(float time)
{
    if (time < game.next_explosion_time)
        return;

    struct ship *ship = &game.player_ship;

    float x = ship->x + (frand() - 0.5f) * screen_to_world_dx(ship->ship_sprite.width);
    float y = ship->y + (frand() - 0.5f) * screen_to_world_dx(ship->ship_sprite.height);

    struct explosion *exp = create_explosion(x, y, SHIP_LAYER, time);

    elist_insert_back(&exp->link, &game.explosions);
    scene_add_sprite(&game.scene, &exp->sprite);

    ++game.num_final_explosions;
    game.next_explosion_time += FINAL_EXPLOSIONS_DELAY;
}

static void game_start(void)
{
    game.score = 0;
    game.energy_refill_score = ENERGY_REFILL_SCORE;
    game.level = 1;

    init_scene(&game.scene);
    scene_set_background(&game.scene, &background_image);

    init_ship(&game.player_ship, WORLD_CENTER_X, WORLD_CENTER_Y, SHIP_LAYER);
    scene_add_sprite(&game.scene, &game.player_ship.ship_sprite);

    game.num_asteroids = 0;
    game.last_asteroid_time = 0;
    init_elist(&game.asteroids);
    init_elist(&game.missiles);
    init_elist(&game.explosions);

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
    delete_missiles();
    delete_explosions();
    destroy_scene(&game.scene);
}

static unsigned int game_loop(void)
{
    bool quit = false;
    bool paused = false;
    bool destroyed = false;

    const float start_time = timer_get_time();
    float time = start_time;
    float level_time = 0;

    while (!quit) {

        if (key_pressed(KEY_ESC))
            break;

        if (paused) {
            while (key_pressed(KEY_P)) { /* wait */ }
            while (!key_pressed(KEY_ESC) && !key_pressed(KEY_P)) { /* wait */ }
            while (key_pressed(KEY_ESC) || key_pressed(KEY_P)) { /* wait */ }
            paused = false;
            timer_get_time_delta();
        }

        if (key_pressed(KEY_P)) {
            paused = true;
        }

        float dt = timer_get_time_delta();
        time += dt;

        level_time += dt;

        if (level_time >= TIME_PER_LEVEL) {
            ++game.level;
            level_time = 0;
        }

        if (game.score > game.energy_refill_score) {
            game.player_ship.energy = 1.0f;
            game.energy_refill_score += ENERGY_REFILL_SCORE;
        }

        if (!destroyed) {

            if (game.player_ship.energy >= SHIP_MIN_ENERGY) {
                control_player_ship(time, dt);
                test_collisions(time, dt);
                create_asteroids(time);
            } else {
                game.num_final_explosions = 0;
                game.next_explosion_time = time + FINAL_EXPLOSIONS_DELAY;
                ship_set_shield(&game.player_ship, false);
                ship_set_power(&game.player_ship, 0);
                destroyed = true;
            }

        } else {

            ship_turn(&game.player_ship, dt * DESTROYED_SHIP_TURN);

            if (game.num_final_explosions < NUM_FINAL_EXPLOSIONS)
                create_final_explosions(time);

            else if (elist_empty(&game.explosions))
                quit = true;
        }

        update_asteroids(dt);
        update_missiles(dt);
        update_explosions(time, dt);
        update_player_ship(dt);

        scene_update(&game.scene, time, dt);
        scene_draw(&game.scene);
        update_energy_display();
        update_score_display();

        if (paused) {
            scene_overlay_image(&game.scene, &game_paused_image,
                                GAME_PAUSED_X, GAME_PAUSED_Y,
                                IMAGE_BLIT_MASK);
        }

        gfx_draw_back_buffer();
    }

    if (destroyed) {

        for (int line = -game_over_image.height; line < GAME_OVER_Y; line += GAME_OVER_SPEED) {
            gfx_draw_image(&background_image, 0, 0, IMAGE_BLIT_COPY);
            gfx_draw_image(&game_over_image, GAME_OVER_X, line, IMAGE_BLIT_MASK);
            gfx_draw_back_buffer();
        }

        while (key_pressed(KEY_ESC) || key_pressed(KEY_SPACE)) { /* wait */ }
        while (!key_pressed(KEY_ESC) && !key_pressed(KEY_SPACE)) { /* wait */ }

        return game.score;

    } else {
        return 0;
    }
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
    missile_cleanup();
    explosion_cleanup();
    scene_cleanup();
    sprite_cleanup();
}

void game_run(void)
{
    while (1) {

        if (!run_intro())
            return;

        game_start();
        game_loop();
        game_end();
    }
}
