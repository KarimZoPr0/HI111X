#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "game.h"

const int speed = 5;

EMSCRIPTEN_KEEPALIVE
void update_and_render(GameContext *ctx)
{
  if(ctx->is_initialized == 0)
  {
    ctx->playerTexture = IMG_LoadTexture(ctx->renderer, "gfx/player.png");
    SDL_QueryTexture(ctx->playerTexture, NULL, NULL, &ctx->player.w, &ctx->player.h);

    ctx->player.x = (WINDOW_WIDTH  - ctx->player.w) / 2;
    ctx->player.y = (WINDOW_HEIGHT - ctx->player.h) / 2 ;

    ctx->is_initialized = 1;
  }

    if (ctx->keyboard[KEY_UP] || ctx->keyboard[KEY_W])    ctx->player.y -= speed;
    if (ctx->keyboard[KEY_DOWN] || ctx->keyboard[KEY_S])  ctx->player.y += speed;
    if (ctx->keyboard[KEY_LEFT] || ctx->keyboard[KEY_A])  ctx->player.x -= speed;
    if (ctx->keyboard[KEY_RIGHT] || ctx->keyboard[KEY_D]) ctx->player.x += speed;

    if (ctx->player.x < 0)                    ctx->player.x = 0;
    if (ctx->player.y < 0)                    ctx->player.y = 0;
    if (ctx->player.x + ctx->player.w > 640)  ctx->player.x = 640 - ctx->player.w;
    if (ctx->player.y + ctx->player.h > 480)  ctx->player.y = 480 - ctx->player.h;

    SDL_SetRenderDrawColor(ctx->renderer, 255, 100, 100, 255);
    SDL_RenderClear(ctx->renderer);

    SDL_RenderCopy(ctx->renderer, ctx->playerTexture, NULL, &ctx->player);

    SDL_Rect rect = {200, 100, 50, 50};
    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 255, 255);
    SDL_RenderFillRect(ctx->renderer, &rect);

    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 0, 255);
    SDL_RenderDrawLine(ctx->renderer, 0, 200, 640,200);

    SDL_RenderPresent(ctx->renderer);
}
