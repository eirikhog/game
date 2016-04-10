#pragma once

struct ConsoleState;

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

    // TODO: Better way to handle this?
    ConsoleState *console;
};

struct MoveWaypoint {
    v2f position;
    MoveWaypoint *next;
};

struct PathfinderTile {
    v2i position;
    u32 weight;
    bool32 visited;
    r32 lineDist;
};

