#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"

/* ------------------------------------------------------------------ */
/*  Constants                                                         */
/* ------------------------------------------------------------------ */
static const float BALL_SPEED_FACTOR = 2.0f;
static const int   PADDLE_SPEED      = 5;

/* Palette (SDL_Color has r‑g‑b‑a) ---------------------------------- */
static const SDL_Color PLAYER1_COLOR   = {100, 128, 255, 255};
static const SDL_Color PLAYER2_COLOR   = {255, 128,   0, 255};
static const SDL_Color BALL_COLOR      = {255,   0,   0, 255};
static const SDL_Color READY_COLOR     = {  0, 255,   0, 255};
static const SDL_Color NOT_READY_COLOR = {255,   0,   0, 255};
static const SDL_Color BG_COLOR        = {0,  80, 100, 255};

/* ------------------------------------------------------------------ */
/*  Helpers                                                           */
/* ------------------------------------------------------------------ */
static inline void apply_ball_speed(GameContext *ctx)
{
    int dirX = (ctx->ballVelX >= 0) ? 1 : -1;
    int dirY = (ctx->ballVelY >= 0) ? 1 : -1;
    ctx->ballVelX = (int)(BALL_SPEED_FACTOR * dirX);
    ctx->ballVelY = (int)(BALL_SPEED_FACTOR * dirY);
}

/* ------------------------------------------------------------------ */
/*  Lightweight TCP packet                                            */
/* ------------------------------------------------------------------ */
typedef struct {
    Uint16 paddleY;
    Uint8  readyState;
    Uint8  gameState;
    Uint32 matchId;
    Sint16 ballX, ballY;
    Sint8  velX,  velY;
} GamePacket;

/* ------------------------------------------------------------------ */
/*  Networking                                                        */
/* ------------------------------------------------------------------ */
static void net_try_connect(GameContext *ctx)
{
    if (ctx->connected) return;

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, "127.0.0.1", 1234) != 0) return;

    ctx->sock = SDLNet_TCP_Open(&ip);
    if (!ctx->sock) return;

    ctx->set = SDLNet_AllocSocketSet(1);
    SDLNet_AddSocket(ctx->set, (SDLNet_GenericSocket)ctx->sock);
    ctx->connected = 1;

    if (ctx->matchId == 0) {
    	ctx->matchId = ((unsigned)rand() << 16) ^ (unsigned)rand();
    }
}

static void net_exchange(GameContext *ctx)
{
    if (!ctx->connected) return;

    // –––––– 1) SEND: build and reliably send one full GamePacket ––––––
    GamePacket out = {
        .paddleY    = (ctx->playerType == 1) ? ctx->paddle1.y : ctx->paddle2.y,
        .readyState = ctx->localReady,
        .gameState  = ctx->gameState,
        .matchId    = ctx->matchId,
        .ballX      = ctx->ball.x,
        .ballY      = ctx->ball.y,
        .velX       = (Sint8)ctx->ballVelX,
        .velY       = (Sint8)ctx->ballVelY
    };
    {
        Uint8 *send_ptr   = (Uint8*)&out;
        int    bytes_left = sizeof(out);
        while (bytes_left > 0) {
            int sent = SDLNet_TCP_Send(ctx->sock, send_ptr, bytes_left);
            if (sent <= 0) goto disconnect;  // error or closed
            send_ptr   += sent;
            bytes_left -= sent;
        }
    }

    // –––––– 2) RECEIVE: poll once, then drain every full packet ––––––
    if (SDLNet_CheckSockets(ctx->set, 0) > 0 &&
        SDLNet_SocketReady(ctx->sock))
    {
        do {
            // Read exactly sizeof(GamePacket) bytes (handle partial reads)
            GamePacket in;
            Uint8 *recv_ptr   = (Uint8*)&in;
            int    bytes_left = sizeof(in);
            while (bytes_left > 0) {
                int n = SDLNet_TCP_Recv(ctx->sock, recv_ptr, bytes_left);
                if (n <= 0) goto disconnect;  // error or closed
                recv_ptr   += n;
                bytes_left -= n;
            }

            // Determine host/client
            bool isHost = (ctx->matchId >= in.matchId);
            ctx->playerType = isHost ? 1 : 2;

            // Update the *other* paddle & ready flag
            SDL_Rect *remote = isHost ? &ctx->paddle2 : &ctx->paddle1;
            remote->y         = in.paddleY;
            ctx->remoteReady = in.readyState;

            if (isHost) {
                // Host drives start & ball simulation
                ctx->ballOwner = 1;
                if (ctx->gameState == STATE_WAITING &&
                    ctx->localReady && ctx->remoteReady)
                {
                    ctx->gameState = STATE_PLAYING;
                    ctx->ballVelX  = 1;
                    ctx->ballVelY  = 1;
                    apply_ball_speed(ctx);
                }
            } else {
                // Client just mirrors host’s state
                ctx->ballOwner = 0;
                ctx->ball.x    = in.ballX;
                ctx->ball.y    = in.ballY;
                ctx->ballVelX  = in.velX;
                ctx->ballVelY  = in.velY;
                ctx->gameState = (GameState)in.gameState;
            }

        } while (SDLNet_SocketReady(ctx->sock));  // loop until no more data
    }

    return;

disconnect:
    SDLNet_TCP_Close(ctx->sock);
    SDLNet_FreeSocketSet(ctx->set);
    ctx->connected = 0;
    ctx->gameState = STATE_DISCONNECT;
}


/* ------------------------------------------------------------------ */
/*  Input                                                             */
/* ------------------------------------------------------------------ */
static void handle_input(GameContext *ctx)
{
    int dy = 0;
    if (ctx->keyboard[KEY_UP] || ctx->keyboard[KEY_W])   dy -= PADDLE_SPEED;
    if (ctx->keyboard[KEY_DOWN] || ctx->keyboard[KEY_S]) dy += PADDLE_SPEED;

    if (dy) {
        SDL_Rect *p = (ctx->playerType == 1) ? &ctx->paddle1 : &ctx->paddle2;
        p->y += dy;
        p->y = SDL_clamp(p->y, 0, WINDOW_HEIGHT - PADDLE_H);
    }

    if (ctx->keyboard[KEY_SPACE] && ctx->gameState == STATE_WAITING) {
		ctx->localReady ^= 1;
        ctx->keyboard[KEY_SPACE] = 0;
    }
}

/* ------------------------------------------------------------------ */
/*  Physics                                                           */
/* ------------------------------------------------------------------ */
static void update_physics(GameContext *ctx)
{
    if (ctx->gameState == STATE_PLAYING && ctx->ballOwner) {
        ctx->ball.x += ctx->ballVelX;
        ctx->ball.y += ctx->ballVelY;

        if (ctx->ball.y <= 0 || ctx->ball.y + BALL_SIZE >= WINDOW_HEIGHT)
            ctx->ballVelY = -ctx->ballVelY;

        const SDL_Rect *p1 = &ctx->paddle1;
        const SDL_Rect *p2 = &ctx->paddle2;

        if (ctx->ballVelX < 0 && ctx->ball.x <= p1->x + PADDLE_W &&
            ctx->ball.y + BALL_SIZE >= p1->y && ctx->ball.y <= p1->y + PADDLE_H)
            ctx->ballVelX = -ctx->ballVelX;

        if (ctx->ballVelX > 0 && ctx->ball.x + BALL_SIZE >= p2->x &&
            ctx->ball.y + BALL_SIZE >= p2->y && ctx->ball.y <= p2->y + PADDLE_H)
            ctx->ballVelX = -ctx->ballVelX;

        if (ctx->ball.x < -BALL_SIZE || ctx->ball.x > WINDOW_WIDTH) {
            ctx->gameState  = STATE_SCORE;
            ctx->scoreTimer = 60 * 10;
            ctx->ball.x     = (WINDOW_WIDTH  - BALL_SIZE)/2;
            ctx->ball.y     = (WINDOW_HEIGHT - BALL_SIZE)/2;
            ctx->ballVelX   = -ctx->ballVelX;
        }
    }
    else if (ctx->gameState == STATE_SCORE && ctx->ballOwner) {
        if (--ctx->scoreTimer <= 0) {
            ctx->gameState = STATE_PLAYING;
            apply_ball_speed(ctx);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Rendering                                                         */
/* ------------------------------------------------------------------ */
static inline void set_draw_color(SDL_Renderer *r, SDL_Color c)
{
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

static void render_game(GameContext *ctx)
{
    SDL_Renderer *r = ctx->renderer;
    set_draw_color(r, BG_COLOR);
    SDL_RenderClear(r);

    set_draw_color(r, PLAYER1_COLOR);
    SDL_RenderFillRect(r, &ctx->paddle1);
    set_draw_color(r, PLAYER2_COLOR);
    SDL_RenderFillRect(r, &ctx->paddle2);
    set_draw_color(r, BALL_COLOR);
    SDL_RenderFillRect(r, &ctx->ball);

    SDL_Rect local = {10, 10, 15, 15}, remote = {WINDOW_WIDTH - 25, 10, 15, 15};
    if (ctx->playerType == 2) { local.x = WINDOW_WIDTH - 25; remote.x = 10; }
    set_draw_color(r, ctx->localReady  ? READY_COLOR : NOT_READY_COLOR);
    SDL_RenderFillRect(r, &local);
    set_draw_color(r, ctx->remoteReady ? READY_COLOR : NOT_READY_COLOR);
    SDL_RenderFillRect(r, &remote);

    SDL_Rect divider = {WINDOW_WIDTH/2 - 1, 0, 2, WINDOW_HEIGHT};
    set_draw_color(r, (SDL_Color){255,255,255,128});
    SDL_RenderFillRect(r, &divider);

    SDL_Rect stateRect = {(WINDOW_WIDTH - 20)/2, 10, 20, 20};
    switch (ctx->gameState) {
        case STATE_WAITING:    set_draw_color(r, (SDL_Color){255,165,0,255}); break;
        case STATE_PLAYING:    set_draw_color(r, (SDL_Color){  0,255,0,255}); break;
        case STATE_SCORE:      set_draw_color(r, (SDL_Color){255,255,0,255}); break;
        case STATE_DISCONNECT: set_draw_color(r, (SDL_Color){255,  0,0,255}); break;
    }
    SDL_RenderFillRect(r, &stateRect);

    SDL_Rect who = {(WINDOW_WIDTH - 40)/2, 35, 40, 15};
    set_draw_color(r, ctx->playerType == 1 ? PLAYER1_COLOR : PLAYER2_COLOR);
    SDL_RenderFillRect(r, &who);

    SDL_RenderPresent(r);
}

/* ------------------------------------------------------------------ */
/*  Tick	 				                                          */
/* ------------------------------------------------------------------ */
EMSCRIPTEN_KEEPALIVE
void update_and_render(GameContext *ctx)
{
    if (!ctx->is_initialized) {
		srand((unsigned)time(NULL));

        ctx->paddle1 = (SDL_Rect){ 20, (WINDOW_HEIGHT-PADDLE_H)/2, PADDLE_W, PADDLE_H };
        ctx->paddle2 = (SDL_Rect){ WINDOW_WIDTH-20-PADDLE_W,
                                   (WINDOW_HEIGHT-PADDLE_H)/2,
                                   PADDLE_W, PADDLE_H };
        ctx->ball    = (SDL_Rect){ (WINDOW_WIDTH-BALL_SIZE)/2,
                                   (WINDOW_HEIGHT-BALL_SIZE)/2,
                                   BALL_SIZE, BALL_SIZE };
        ctx->gameState   = STATE_WAITING;
        ctx->localReady  = ctx->remoteReady = 0;
        ctx->ballVelX    = ctx->ballVelY = 0;
        ctx->matchId     = 0;
        ctx->playerType  = 0;
        ctx->scoreTimer  = 0;

        ctx->is_initialized = 1;
    }

    switch (ctx->gameState) {
        case STATE_WAITING:
            net_try_connect(ctx);
            handle_input(ctx);
            net_exchange(ctx);
            break;
        case STATE_PLAYING:
        case STATE_SCORE:
            handle_input(ctx);
            net_exchange(ctx);
            update_physics(ctx);
            break;
        case STATE_DISCONNECT:
            net_try_connect(ctx);
            if (ctx->connected) {
                ctx->gameState  = STATE_WAITING;
                ctx->localReady = ctx->remoteReady = 0;
            }
            break;
    }

    render_game(ctx);
}
