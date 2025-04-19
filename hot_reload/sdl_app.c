#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include "game/game.h"
#include "input.h"
#include "input.c"

GameContext ctx;

typedef void (*update_and_render_func)(GameContext*);
update_and_render_func update_and_render = NULL;

EMSCRIPTEN_KEEPALIVE void set_update_and_render_func(update_and_render_func f) {
    update_and_render = f;
    printf("set_update_and_render_func called! Function pointer is %p\n", f);
}

 void processEvents() {
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
    if (update_and_render) {
        update_and_render(&ctx);
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    /* window / renderer */
    SDL_Window *win = SDL_CreateWindow(
        "Reload", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    ctx.renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);


    memset(&ctx.keyboard, 0, sizeof(int) * KEY_COUNT);

    emscripten_set_main_loop(loop, 0, 1);
    return 0;
}