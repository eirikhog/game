#pragma once

#include "Common.h"
#include "Memory.h"
#include "Math.h"
#include "Renderer.h"

#define CHUNK_DIM 16
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
    v2i position;
} Entity;

typedef struct {
    v2i camera;
    v2i screenSize;
    WorldChunk chunks[32];
    v2i mousePos;
    bool32 mouseDrag;
    v2i mouseDragOrigin;
} World;

void WorldCreate(World *world);
void WorldUpdate(World *world, game_input *input, real32 dt);
void WorldRender(World *world, RenderContext *renderer, v2i windowSize);
