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

struct PlatformAPI;

struct PlatformState {
    v2i windowSize;
    PlatformAPI *api;
    bool32 libReloaded;
    r32 lastFrameTime;
    bool32 shutdownRequested;
};

struct GameMemory {
    void *permanent;
    u32 permanentSize;
    void *transient;
    u32 transientSize;
};

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

struct KeyboardState {
    i32 keyCount;
    i32 keyStack[KEYBOARD_MAX]; // max 32 keys per frame
};

struct GameInput {
    u32 buttons;
    v2i mouse_position;
    v2i mouse_delta;
    u32 mouse_buttons;
    v2f joystick;
    KeyboardState keyboard;
};

// TODO: These are platform specific. Probably have to do this define in the implementation header?
typedef void *FileHandle;

typedef void update_game(PlatformState *state, GameMemory *memory, GameInput *input, r32 dt);
typedef char *read_entire_file(char *filename, u32 *size);
typedef void write_entire_file(char *filename, void *data, u32 size);
typedef void RunExternalProgram(const char* command, char *outputBuffer, u32 bufferSize);

// File functions... (Do we need asynch versions?)
typedef FileHandle file_open(char *filename);
typedef bool32 file_close(FileHandle);
typedef u32 file_write(FileHandle, u32 size, void *source);
typedef u32 file_read(FileHandle, u32 size, void *dest);
typedef bool32 file_move(char *src, char *dest);
typedef bool32 file_delete(char *filename);
typedef bool32 file_exists(char *filename);

typedef struct PlatformAPI {
    read_entire_file* ReadEntireFile;
    write_entire_file* WriteEntireFile;
    RunExternalProgram* RunProgram;
    file_open *FileOpen;
    file_close *FileClose;
    file_write *FileWrite;
    file_read *FileRead;
    file_move *FileMove;
    file_delete *FileDelete;
    file_exists *FileExists;
} PlatformAPI;

typedef struct {
    void (*DebugOutput)(char *);
    update_game* UpdateGame;
} GameFunctions;

typedef struct {
    void *GameMemory;
} ProgramState;

#endif
