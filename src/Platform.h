#ifndef _PLATFORM_H
#define _PLATFORM_H

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
struct PlatformAPI;

typedef struct {
    v2i windowSize;
    PlatformAPI *api;
} PlatformState;

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
    BUTTON_RIGHT = 8,
    BUTTON_CONSOLE = 16,
};

#define KEYBOARD_MAX 32

struct keyboard_state {
    i32 keyCount;
    i32 keyStack[KEYBOARD_MAX]; // max 32 keys per frame
};

struct game_input {
    uint32 buttons;
    v2i mouse_position;
    v2i mouse_delta;
    uint32 mouse_buttons;
    v2f joystick;
    keyboard_state keyboard;
};

typedef void update_game(PlatformState *state, game_memory *memory, game_input *input, real32 dt);
typedef char *read_entire_file(char *filename, uint32 *size);
typedef void write_entire_file(char *filename, void *data, uint32 size);
typedef void RunExternalProgram(const char* command, char *outputBuffer, u32 bufferSize);

typedef struct PlatformAPI {
    read_entire_file* ReadEntireFile;
    write_entire_file* WriteEntireFile;
    RunExternalProgram* RunProgram;
} PlatformAPI;

typedef struct {
    void (*DebugOutput)(char *);
    update_game* UpdateGame;
} game_functions;

typedef struct {
    void *GameMemory;
} program_state;

#endif
