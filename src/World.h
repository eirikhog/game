#pragma once

#include "Common.h"
#include "Memory.h"
#include "Math.h"
#include "Renderer.h"
#include "Chunk.h"

enum EntityType {
    EntityType_None,
    EntityType_Unit,
};

struct Entity {
    EntityType type;
    v2f position;
    v2f moveTarget;
    bool32 selected;
};

struct World {
    v2i camera;
    v2i screenSize;
    WorldChunk chunks[CHUNK_COUNT];

    // Input
    v2i mousePos;
    bool32 mouseDrag;
    v2i mouseDragOrigin;
    r32 mouseLeftHoldTime;
    r32 mouseRightHoldTime;
    bool32 cameraMoving;

    u32 entityCount;
    Entity entities[ENTITIES_MAX];
};

void WorldCreate(World *world);
void WorldUpdate(World *world, GameInput *input, r32 dt);
void WorldRender(World *world, RenderContext *renderer, v2i windowSize);
