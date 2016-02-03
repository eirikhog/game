#pragma once

#include "Common.h"
#include "Math.h"

#ifdef _DEBUG
#undef Assert
#define Assert(x) if(!(x)) { (*(int*)(0)) = 0; }
#else
#undef Assert
#define Assert(x)
#endif

#define InvalidCodePath() Assert(0)

#ifdef _WIN32
#  define EXPORT __declspec( dllexport )
#else
#  define EXPORT
#endif

struct game_buffer;
struct platform_api;

typedef struct {
    v2 windowSize;
    platform_api *api;
} platform_state;

typedef struct {
    void *permanent;
    uint32 permanentSize;
    void *transient;
    uint32 transientSize;
} game_memory;

enum MouseButtons {
    MOUSE_LEFT = 1,
    MOUSE_RIGHT = 2
};

enum InputButtons {
    BUTTON_UP = 1,
    BUTTON_DOWN = 2,
    BUTTON_LEFT = 4,
    BUTTON_RIGHT = 8
};

struct game_input {
    uint32 buttons;
    v2 mouse_position;
    v2 mouse_delta;
    uint32 mouse_buttons;
    v2 joystick;
};

typedef void update_game(platform_state *state, game_memory *memory, game_input *input, real32 dt);
typedef char *read_entire_file(char *filename, uint32 *size);

typedef struct platform_api {
    read_entire_file* ReadEntireFile;
} platform_api;

typedef struct {
    void (*DebugOutput)(char *);
    update_game* UpdateGame;
} game_functions;

typedef struct {
    void *GameMemory;
} program_state;

