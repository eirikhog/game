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
    void *permanent;
    uint32 permanentSize;
    void *transient;
    uint32 transientSize;
} game_memory;

enum MouseButtons {
    MOUSE_LEFT = 1,
    MOUSE_RIGHT = 2
};

struct game_input {
    uint32 buttons;
    v2 mouse_position;
    v2 mouse_delta;
    uint32 mouse_buttons;
};

typedef void update_game(platform_api *api, game_memory *memory, game_input *input);
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

