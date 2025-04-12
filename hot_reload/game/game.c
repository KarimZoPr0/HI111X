#include <emscripten.h>
#include <SDL2/SDL.h>
#include "../base/base_inc.h"
#include "../base/base_inc.c"
#include "game.h"

static SDL_Rect rect = {50, 50, 50, 50};


EMSCRIPTEN_KEEPALIVE
void update(GameContext* ctx) {
    // Direct SDL rendering using context's renderer
    SDL_SetRenderDrawColor(ctx->renderer, 80, 100, 100, 255);
    SDL_RenderClear(ctx->renderer);


    SDL_RenderPresent(ctx->renderer);
    return;
}