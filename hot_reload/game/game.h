#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

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

typedef struct GameInput GameInput;
struct GameInput
{
    int keyboard[KEY_COUNT];
};

typedef struct Rect Rect;
struct Rect
{
    int x;
    int y;
    int w;
    int h;

    // color
    int r;
    int g;
    int b;
    int a;
};

// Command types
typedef enum {
    CMD_SET_DRAW_COLOR,
    CMD_CLEAR,
    CMD_FILL_RECT,
    CMD_PRESENT
} CommandType;

typedef struct {
    CommandType type;
    union {
        struct {
            int r, g, b, a;
        } color;
        struct {
            int x, y, w, h;
        } rect;
    } data;
} Command;

typedef struct {
    Command* commands;
    int capacity;
    int count;
} CommandQueue;

typedef struct {
    SDL_Rect player;
    int keyboard[KEY_COUNT];
    SDL_Texture* playerTexture;

    CommandQueue cmdQueue;
    SDL_Renderer* renderer;
    int is_initialized;
} GameContext;