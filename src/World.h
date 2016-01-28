#pragma once

#include "Common.h"
#include "Memory.h"
#include "Math.h"
#include "Renderer.h"

#define CHUNK_SIZE 24
#define TILE_SIZE 64
#define WORLD_SIZE 1024

typedef struct {
    v2 player;
} GameWorld;

void world_update(GameWorld *world, game_input *input, real32 dt);
void world_render(GameWorld *world, RenderContext *renderer);

