#include <emscripten.h>
#include <SDL2/SDL.h>
#include "game.h"

// A global variable in the side module
static int speed = 10;

EMSCRIPTEN_KEEPALIVE
void update_and_render(GameContext* ctx) {
    if (!ctx) return;

    // Example: modifying state based on keyboard input
    if (ctx->keyboard[KEY_UP] || ctx->keyboard[KEY_W]) ctx->player.y -= speed;
    if (ctx->keyboard[KEY_DOWN] || ctx->keyboard[KEY_S]) ctx->player.y += speed;
    if (ctx->keyboard[KEY_LEFT] || ctx->keyboard[KEY_A]) ctx->player.x -= speed;
    if (ctx->keyboard[KEY_RIGHT] || ctx->keyboard[KEY_D]) ctx->player.x += speed;

    if (ctx->player.x < 0) {
        ctx->player.x = 0;
    } else if (ctx->player.x + ctx->player.w > 640) {
        ctx->player.x = 640 - ctx->player.w;
    }
    if (ctx->player.y < 0) {
        ctx->player.y = 0;
    } else if (ctx->player.y + ctx->player.h > 480) {
        ctx->player.y = 480 - ctx->player.h;
    }

    // Use a stack-allocated SDL_Rect
    SDL_Rect rect = {200, 50, 50, 50};

    SDL_SetRenderDrawColor(ctx->renderer, 255, 100, 100, 255);
    SDL_RenderClear(ctx->renderer);

    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 255, 255);
    SDL_RenderFillRect(ctx->renderer, &rect);
    SDL_RenderPresent(ctx->renderer);
}
