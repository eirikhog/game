#pragma once

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

    // TODO: Move these to chunk
    u32 entityCount;
    Entity entities[ENTITIES_MAX];
};

void WorldCreate(World *world);
void WorldUpdate(World *world, GameInput *input, r32 dt);
void WorldRender(World *world, RenderContext *renderer, v2i windowSize);
