#ifndef GAME_H
#define GAME_H

#include "../base/base_inc.h"

typedef struct GameState GameState;
struct GameState
{
    Arena *arena;
    Arena *frame_arena;
};

typedef struct App App;
struct App
{
    Arena *arena;
    SDL_Window *window;
    SDL_Renderer *renderer;
    GameState *game_state;
    B32 is_initialized;
};

shared_function void GameUpdateAndRender(App *app);
#endif //GAME_H
