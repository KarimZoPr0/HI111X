// [h] third-party
#define SDL_MAIN_HANDLED
#include <SDL.h>

// [h] root
#include "../base/base_inc.h"
#include "../game/game.h"

App *app;

typedef void (*GameUpdateAndRenderFunc)(App *app);
typedef struct
{
    void *library;
    GameUpdateAndRenderFunc update_and_render;
    time_t last_write_time;
} GameCode;

// Returns the last modification time of a file.
time_t get_last_write_time(const char *path)
{
    struct stat attr;
    if (stat(path, &attr) == 0)
    {
        return attr.st_mtime;
    }
    return 0;
}

// Simple file copy (copies binary data from src to dst)
int copy_file(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out)
    {
        fclose(in);
        return -1;
    }
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0)
    {
        fwrite(buffer, 1, bytes, out);
    }
    fclose(in);
    fclose(out);
    return 0;
}

GameCode load_game_code(const char *original_lib, const char *temp_lib)
{
    GameCode code = {0};

    // Copy original DLL to temporary DLL so that the original file is free to be recompiled.
    if (copy_file(original_lib, temp_lib) != 0)
    {
        SDL_Log("Failed to copy %s to %s", original_lib, temp_lib);
        return code;
    }

    code.library = SDL_LoadObject(temp_lib);
    if (code.library == 0)
    {
        return code;
    }
    code.update_and_render = (GameUpdateAndRenderFunc)SDL_LoadFunction(code.library, "GameUpdateAndRender");
    if (!code.update_and_render)
    {
        SDL_Log("SDL_LoadFunction error: %s", SDL_GetError());
    }
    code.last_write_time = get_last_write_time(original_lib);
    return code;
}

void unload_game_code(GameCode *code)
{
    if (code->library)
    {
        SDL_UnloadObject(code->library);
        code->library = NULL;
    }
    code->update_and_render = NULL;
    code->last_write_time = 0;
}

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Hot Reloading Example",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600, SDL_WINDOW_ALWAYS_ON_TOP);
    if (!window)
    {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_Log("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Arena *arena = ArenaAlloc();
    app = push_array(arena, App, 1);
    app->window = window;
    app->renderer = renderer;
    app->game_state = push_array(arena, GameState, 1);
    app->game_state->arena = ArenaAlloc();
    app->game_state->frame_arena = ArenaAlloc();
    app->is_initialized = 0;

    const char *original_lib = "libgame.dll";
    const char *temp_lib = "libgame_temp.dll";

    GameCode game = load_game_code(original_lib, temp_lib);

    B32 running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = 0;
        }

        // Check if the original DLL has been updated (hot reload trigger).
        time_t new_write_time = get_last_write_time(original_lib);
        if (new_write_time > game.last_write_time)
        {
            unload_game_code(&game);
            game = load_game_code(original_lib, temp_lib);
        }

        // Call the game update/render function, passing in our context.
        if (game.update_and_render)
        {
            game.update_and_render(app);
        }
        SDL_Delay(16); // Roughly 60 FPS.
    }

    unload_game_code(&game);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
