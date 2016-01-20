#pragma once

#include "Common.h"
#include "Memory.h"
#include "Math.h"
#include "Renderer.h"

#define CHUNK_SIZE 24
#define TILE_SIZE 64
#define WORLD_SIZE 1024

enum TileType {
    TILE_NONE,
    TILE_SOLID,
    TILE_COUNT
};

struct world_chunk {
    uint32 id;
    int32 chunk_x;
    int32 chunk_y;

    TileType tiles[CHUNK_SIZE*CHUNK_SIZE];
};

struct world_coordinates {
    v2 position;
};

struct unit {
    v2 position;
};

typedef struct {
    v2 screen_position;
    real32 time;
    MemorySegment memory;
    world_chunk chunks[WORLD_SIZE];

    unit player;
} GameWorld;

GameWorld *create_world(MemorySegment *memory);
void world_update(GameWorld *world, game_input *input, real32 dt);
void world_render(GameWorld *world, RenderContext *renderer);

