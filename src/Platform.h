#pragma once

#include "Common.h"

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

struct game_input;
struct game_buffer;
struct platform_api;

typedef struct {
    void *permanent;
    uint32 permanentSize;
    void *transient;
    uint32 transientSize;
} game_memory;

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

