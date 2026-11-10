#include "player.h"
#include "map.h"
#include <SDL3/SDL.h>
#include <math.h>

static bool is_free(double x, double y) {
    return x >= 0.0 && y >= 0.0 && x < MAP_WIDTH && y < MAP_HEIGHT
        && WORLD_MAP[(int)y][(int)x] == 0;
}

static void move_with_collision(Player* p, double dx, double dy) {
    constexpr double margin = 0.2;
    double next_x = p->pos_x + dx;
    double next_y = p->pos_y + dy;

    double check_x = next_x + (dx > 0 ? margin : -margin);
    double check_y = next_y + (dy > 0 ? margin : -margin);

    if (is_free(check_x, p->pos_y)) p->pos_x = next_x;
    if (is_free(p->pos_x, check_y)) p->pos_y = next_y;
}

void player_update(Player* p, double dt) {
    const bool* keys = SDL_GetKeyboardState(nullptr);
    constexpr double move_speed = 4.5;
    constexpr double rot_speed = 2.8;

    double move_step = move_speed * dt;
    double rot_step = rot_speed * dt;

    // W & S for forward and backward movement
    if (keys[SDL_SCANCODE_W]) move_with_collision(p, p->dir_x * move_step, p->dir_y * move_step);
    if (keys[SDL_SCANCODE_S]) move_with_collision(p, -p->dir_x * move_step, -p->dir_y * move_step);

    // A & D for strafing left and right
    if (keys[SDL_SCANCODE_A]) move_with_collision(p,  p->dir_y * move_step, -p->dir_x * move_step);
    if (keys[SDL_SCANCODE_D]) move_with_collision(p, -p->dir_y * move_step,  p->dir_x * move_step);

    // Rotate with [, ], Left & Right arrow keys
    double angle = 0.0;
    if (keys[SDL_SCANCODE_LEFTBRACKET]  || keys[SDL_SCANCODE_LEFT])  angle = -rot_step;
    if (keys[SDL_SCANCODE_RIGHTBRACKET] || keys[SDL_SCANCODE_RIGHT]) angle =  rot_step;

    if (angle != 0.0) {
        double cos_a = cos(angle);
        double sin_a = sin(angle);

        double old_dir_x = p->dir_x;
        p->dir_x = p->dir_x * cos_a - p->dir_y * sin_a;
        p->dir_y = old_dir_x * sin_a + p->dir_y * cos_a;

        double old_plane_x = p->plane_x;
        p->plane_x = p->plane_x * cos_a - p->plane_y * sin_a;
        p->plane_y = old_plane_x * sin_a + p->plane_y * cos_a;
    }
}