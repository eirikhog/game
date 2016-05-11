#pragma once

#include "Platform.h"
#include "Renderer.h"
#include "Assets.h"
#include "World.h"
#include "Console.h"

typedef struct {
    bool initialized;
    RenderContext *renderer;
    MemorySegment game_memory;
    MemorySegment renderer_memory;
    GameAssets assets;
    World *world;
    bool32 shutdown;
    bool32 restart;

    r32 elapsedTime;
    ConsoleState console;

    MemoryPool transientMemory;
} GameState;

