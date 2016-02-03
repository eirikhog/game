#pragma once

#include "Common.h"
#include "Memory.h"
#include "Math.h"
#include "Renderer.h"

#define CHUNK_DIM 32
#define TILE_SIZE 32

typedef struct {
    int32 x;
    int32 y;
    uint32 tiles[CHUNK_DIM*CHUNK_DIM];
} WorldChunk;

typedef struct {
    WorldChunk *next;
    WorldChunk *prev;
} LoadedWorldChunk;

typedef struct {
    v2 position;
} Entity;

typedef struct {
    v2 camera;
    v2 screenSize;
} World;

void world_update(World *world, game_input *input, real32 dt);
void world_render(World *world, RenderContext *renderer);
