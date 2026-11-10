#include "config.h"
#include "player.h"
#include "renderer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("C23 SDL3 Raycaster", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, nullptr) : nullptr;
    SDL_Texture* screen_texture = renderer ? SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT
    ) : nullptr;
    uint32_t* pixels = calloc((size_t)SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(uint32_t));

    if (!window || !renderer || !screen_texture || !pixels) {
        SDL_Log("Initialization failed: %s", SDL_GetError());
        free(pixels);
        SDL_DestroyTexture(screen_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    static uint32_t textures[NUM_TEXTURES][TEX_SIZE * TEX_SIZE];
    render_generate_textures(textures);

    Player player = {
        .pos_x = 2.5, .pos_y = 5.5,
        .dir_x = 1.0, .dir_y = 0.0,
        .plane_x = 0.0, .plane_y = 0.66
    };

    uint64_t last_ticks = SDL_GetTicksNS();
    bool running = true;
    SDL_Event event;

    while (running) {
        uint64_t current_ticks = SDL_GetTicksNS();
        double dt = (double)(current_ticks - last_ticks) / 1e9;
        if (dt > 0.05) dt = 0.05; // avoid tunnelling through walls after a stall
        last_ticks = current_ticks;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) running = false;
        }
        
        player_update(&player, dt);
        render_draw_scene(pixels, &player, textures);

        SDL_UpdateTexture(screen_texture, nullptr, pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, screen_texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    free(pixels);
    SDL_DestroyTexture(screen_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}