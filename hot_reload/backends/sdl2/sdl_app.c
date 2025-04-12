#include "../../base/base_inc.h"
#include "../../base/base_inc.c"
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../game/game.h"
#include "input.h"
#include "input.c"

GameContext ctx;

typedef void (*UpdateFunc)(GameContext*);
UpdateFunc updateFunc = NULL;

EMSCRIPTEN_KEEPALIVE void setUpdateFunc(UpdateFunc f) {
    updateFunc = f;
    printf("setUpdateFunc called! Function pointer is %p\n", f);
}

internal void processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_KEYDOWN:
            doKeyDown(&e.key);
            break;
        case SDL_KEYUP:
            doKeyUp(&e.key);
            break;
        }
    }
}

void loop() {
    processEvents();

    if (updateFunc) {
        updateFunc(&ctx);
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Hot Reload", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    // Initialize the renderer and store it in the context
    ctx.renderer = SDL_CreateRenderer(win, -1, 0);

    // Initialize game context
    ctx.player.x = 400;
    ctx.player.y = 300;
    ctx.player.w = 50;
    ctx.player.h = 50;

    memset(&ctx.keyboard, 0, sizeof(int) * KEY_COUNT);

    emscripten_set_main_loop(loop, 0, 1);
    return 0;
}