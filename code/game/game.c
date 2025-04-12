//
// Created by Abdik on 2025-03-23.
//

// [h] third-party
#include <SDL.h>

#include "game.h"

shared_function void GameUpdateAndRender(App* app)
{
    if (app->is_initialized == 0)
    {

    }
    SDL_SetRenderDrawColor(app->renderer, 200, 0, 255, 255);
    SDL_RenderClear(app->renderer);

    SDL_RenderPresent(app->renderer);
}