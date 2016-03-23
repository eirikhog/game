#pragma once

#include "Common.h"
#include "Memory.h"
#include "Math.h"
#include "Renderer.h"
#include "Chunk.h"

struct World;

void WorldCreate(World *world);
void WorldUpdate(World *world, GameInput *input, r32 dt);
void WorldRender(World *world, RenderContext *renderer, v2i windowSize);
