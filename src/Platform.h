#pragma once

#include <stdint.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef float real32;
typedef float real64;

#ifdef _DEBUG
#undef Assert
#define Assert(x) if(!(x)) { (*(int*)(0)) = 0; }
#else
#undef Assert
#define Assert(x)
#endif

#ifdef _WIN32
#  define EXPORT __declspec( dllexport )
#else
#  define EXPORT
#endif

struct game_input;
struct game_buffer;
struct platform_api;

typedef struct {
    void *transient;
    uint32 transientSize;
    // Permanent game data (save files, etc?)
    void *permanent;
    uint32 permanentSize;
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

typedef struct {
    uint32 width;
    uint32 height;
    uint32 pitch;
    void *data;
} bitmap;




