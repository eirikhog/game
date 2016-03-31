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
    bool32 dragSelect;
    Rect2Di dragTarget;
    bool32 setMovePos;
    v2i movePos;
};

void WorldCreate(World *world);
void WorldUpdate(World *world, GameInput *input, r32 dt);
void WorldRender(World *world, RenderContext *renderer, v2i windowSize);
