#ifndef GAME_H
#define GAME_H

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define PADDLE_W       10
#define PADDLE_H       80
#define BALL_SIZE      10

typedef enum KeyCode KeyCode;
enum KeyCode
{
    KEY_UNKNOWN = 0,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_RETURN,
    KEY_ESCAPE,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,

    KEY_MINUS,
    KEY_EQUALS,
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_BACKSLASH,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,

    KEY_CAPSLOCK,
    KEY_PRINTSCREEN,
    KEY_SCROLLLOCK,
    KEY_PAUSE,
    KEY_INSERT,
    KEY_HOME,
    KEY_PAGEUP,
    KEY_DELETE,
    KEY_END,
    KEY_PAGEDOWN,

    KEY_COUNT
};

typedef enum {
    STATE_WAITING,
    STATE_PLAYING,
    STATE_SCORE,
    STATE_DISCONNECT
} GameState;

typedef struct {
    SDL_Renderer *renderer;

    int          keyboard[KEY_COUNT];

    SDL_Rect     paddle1;
    SDL_Rect     paddle2;
    SDL_Rect     ball;
    int          ballVelX, ballVelY;

    TCPsocket    sock;
    SDLNet_SocketSet set;
    int          connected;

    GameState    gameState;
    int          localReady;
    int          remoteReady;
    int          ballOwner;
    unsigned int matchId;
    int          playerType;
    int          scoreTimer;

    int          is_initialized;
} GameContext;

#endif /* GAME_H */