#pragma once

#include "Common.h"
#include "Memory.h"

#define CHUNK_SIZE 16 
#define TILE_SIZE 32

struct world_chunk {
    int32 chunk_x;
    int32 chunk_y;
};

struct game_world {
    v2 screen_position;
    real32 time;
    memory_segment memory;
};

game_world *create_world(memory_segment *memory);
void world_update(real32 dt);
void world_render(render_context *renderer);
