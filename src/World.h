#pragma once

struct World {
    v2i camera;
    v2i screenSize;
    WorldChunk chunks[CHUNK_COUNT];

    // Transient memory
    MemorySegment transientMemory;

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

    // Entities which are loaded
    u32 loadedEntitiesCount;
    Entity loadedEntities[4096]; // TODO: Determine a safe number...
};

struct MoveWaypoint {
    v2f position;
    MoveWaypoint *next;
};

struct PathfinderTile {
    v2i position;
    u32 weight;
};

void WorldCreate(World *world, MemorySegment transient);
void WorldUpdate(World *world, GameInput *input, r32 dt);
void WorldRender(World *world, RenderContext *renderer, v2i windowSize);
