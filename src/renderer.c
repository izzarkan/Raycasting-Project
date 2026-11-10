#include "renderer.h"
#include "map.h"
#include <math.h>

void render_generate_textures(uint32_t textures[NUM_TEXTURES][TEX_SIZE * TEX_SIZE]) {
    for (int y = 0; y < TEX_SIZE; y++) {
        for (int x = 0; x < TEX_SIZE; x++) {
            int idx = y * TEX_SIZE + x;

            // Texture 0: Red Brick
            bool mortar = (y % 16 == 0) || (((y / 16) % 2 == 0) ? (x % 32 == 0) : ((x + 16) % 32 == 0));
            uint8_t brick_var = (uint8_t)(((x * 7 + y * 13) & 0x1F));
            textures[0][idx] = mortar
                ? 0xFF7F8C8D
                : (0xFF000000 | ((170 + brick_var) << 16) | (45 << 8) | 40);

            // Texture 1: Stone Blocks
            bool border = (x == 0 || y == 0 || x == TEX_SIZE - 1 || y == TEX_SIZE - 1);
            uint8_t stone = (uint8_t)(100 + ((x ^ y) & 0x2F));
            textures[1][idx] = border
                ? 0xFF2C3E50
                : (0xFF000000 | ((stone - 20) << 16) | ((stone + 20) << 8) | stone);

            // Texture 2: Blue Tech Panel
            bool grid = (x % 16 == 0 || y % 16 == 0);
            bool core = (x > 20 && x < 44 && y > 20 && y < 44);
            if (grid)      textures[2][idx] = 0xFF1A252F;
            else if (core) textures[2][idx] = 0xFF3498DB;
            else           textures[2][idx] = 0xFF243B55;
        }
    }
}

static void put_pixel(uint32_t* pixels, int x, int y, uint32_t color) {
    if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    pixels[y * SCREEN_WIDTH + x] = color;
}

static void draw_minimap(uint32_t* pixels, const Player* p) {
    constexpr int scale = 10;
    constexpr int offset = 12;

    for (int my = 0; my < MAP_HEIGHT; my++) {
        for (int mx = 0; mx < MAP_WIDTH; mx++) {
            uint32_t color = (WORLD_MAP[my][mx] > 0) ? 0xFFCCCCCC : 0xFF181818;
            for (int py = 0; py < scale; py++) {
                for (int px = 0; px < scale; px++) {
                    int sx = offset + mx * scale + px;
                    int sy = offset + my * scale + py;
                    pixels[sy * SCREEN_WIDTH + sx] = color;
                }
            }
        }
    }

    int px = offset + (int)(p->pos_x * scale);
    int py = offset + (int)(p->pos_y * scale);
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            put_pixel(pixels, px + dx, py + dy, 0xFFFF3333);
        }
    }
    for (int i = 1; i <= 12; i++) {
        int lx = px + (int)(p->dir_x * i);
        int ly = py + (int)(p->dir_y * i);
        put_pixel(pixels, lx, ly, 0xFFFFFF00);
    }
}

void render_draw_scene(
    uint32_t* pixels,
    const Player* p,
    const uint32_t textures[NUM_TEXTURES][TEX_SIZE * TEX_SIZE]
) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        uint32_t bg = (y < SCREEN_HEIGHT / 2) ? 0xFF1A1A20 : 0xFF333339;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            pixels[y * SCREEN_WIDTH + x] = bg;
        }
    }

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        double camera_x = 2.0 * x / (double)SCREEN_WIDTH - 1.0;
        double ray_dir_x = p->dir_x + p->plane_x * camera_x;
        double ray_dir_y = p->dir_y + p->plane_y * camera_x;

        int map_x = (int)p->pos_x;
        int map_y = (int)p->pos_y;

        double delta_dist_x = (ray_dir_x == 0.0) ? 1e30 : fabs(1.0 / ray_dir_x);
        double delta_dist_y = (ray_dir_y == 0.0) ? 1e30 : fabs(1.0 / ray_dir_y);

        double side_dist_x, side_dist_y;
        int step_x, step_y;

        if (ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = (p->pos_x - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0 - p->pos_x) * delta_dist_x;
        }

        if (ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = (p->pos_y - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0 - p->pos_y) * delta_dist_y;
        }

        bool hit = false;
        int side = 0;
        while (!hit) {
            if (side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }
            if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
                // Ray left the map: treat the last cell as a wall hit
                map_x = (map_x < 0) ? 0 : (map_x >= MAP_WIDTH  ? MAP_WIDTH  - 1 : map_x);
                map_y = (map_y < 0) ? 0 : (map_y >= MAP_HEIGHT ? MAP_HEIGHT - 1 : map_y);
                hit = true;
            } else if (WORLD_MAP[map_y][map_x] > 0) {
                hit = true;
            }
        }

        double perp_wall_dist = (side == 0)
            ? (side_dist_x - delta_dist_x)
            : (side_dist_y - delta_dist_y);
        if (perp_wall_dist < 1e-4) perp_wall_dist = 1e-4;

        int line_height = (int)(SCREEN_HEIGHT / perp_wall_dist);
        int draw_start = -line_height / 2 + SCREEN_HEIGHT / 2;
        if (draw_start < 0) draw_start = 0;
        int draw_end = line_height / 2 + SCREEN_HEIGHT / 2;
        if (draw_end >= SCREEN_HEIGHT) draw_end = SCREEN_HEIGHT - 1;

        int tex_num = WORLD_MAP[map_y][map_x] - 1;
        if (tex_num < 0 || tex_num >= NUM_TEXTURES) tex_num = 0;

        double wall_x = (side == 0)
            ? (p->pos_y + perp_wall_dist * ray_dir_y)
            : (p->pos_x + perp_wall_dist * ray_dir_x);
        wall_x -= floor(wall_x);

        int tex_x = (int)(wall_x * (double)TEX_SIZE);
        if ((side == 0 && ray_dir_x > 0) || (side == 1 && ray_dir_y < 0)) {
            tex_x = TEX_SIZE - tex_x - 1;
        }

        double step = 1.0 * TEX_SIZE / line_height;
        double tex_pos = (draw_start - SCREEN_HEIGHT / 2.0 + line_height / 2.0) * step;

        for (int y = draw_start; y <= draw_end; y++) {
            int tex_y = (int)tex_pos & (TEX_SIZE - 1);
            tex_pos += step;

            uint32_t color = textures[tex_num][tex_y * TEX_SIZE + tex_x];
            if (side == 1) {
                color = ((color & 0x00FEFEFE) >> 1) | 0xFF000000;
            }
            pixels[y * SCREEN_WIDTH + x] = color;
        }
    }

    draw_minimap(pixels, p);
}