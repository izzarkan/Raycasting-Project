#pragma once
#include "config.h"
#include "player.h"
#include <stdint.h>

void render_generate_textures(uint32_t textures[NUM_TEXTURES][TEX_SIZE * TEX_SIZE]);
void render_draw_scene(
    uint32_t* pixels,
    const Player* p,
    const uint32_t textures[NUM_TEXTURES][TEX_SIZE * TEX_SIZE]
);