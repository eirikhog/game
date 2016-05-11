
#include "Unity.h"

static void
GameInit(GameState *state, PlatformAPI *api, GameMemory *memory) {
    MemorySegment mem_all = {};
    mem_all.size = memory->permanentSize - sizeof(GameState);
    mem_all.base = (u8*)memory->permanent + sizeof(GameState);
    mem_all.used = 0;
    state->game_memory = mem_all;

    MemorySegment transientMemory = {};
    transientMemory.size = memory->transientSize;
    transientMemory.base = (u8*)memory->transient;
    transientMemory.used = 0;
    MemorySegment assetMemory = AllocMemory(&transientMemory, Megabytes(128));
    state->assets = AssetsInit(api, assetMemory);

    // Allocate memory for renderer
    MemorySegment renderer_memory = AllocMemory(&mem_all, Megabytes(16));
    state->renderer_memory = renderer_memory;
    state->renderer = RenderInit(&state->assets, renderer_memory);

    // Create the game world
    MemorySegment world_memory = AllocMemory(&mem_all, sizeof(World));
    state->world = (World*)world_memory.base;

    // Allocate transient memory
    MemorySegment transientWorldMemSegment = AllocMemory(&mem_all, Megabytes(64));
    MemoryPool transientWorldMemory = InitializeAllocator(&transientWorldMemSegment);
    WorldCreate(state->world, transientWorldMemory, &state->console);


    state->console.animationProgress = 0.0f;
    state->console.animationSpeed = 0.05f;

    state->elapsedTime = 0.0f;

    state->restart = false;
    state->shutdown = false;
    state->initialized = true;

    WriteConsole(&state->console, "Game initialized.");

    char *memUsedText = mprintf("Used memory: %d MB", mem_all.used / (1024 * 1024));
    SCOPE_FREE(memUsedText);
    WriteConsole(&state->console, memUsedText);

    char *memText = mprintf("Free memory: %d MB", (mem_all.size - mem_all.used) / (1024 * 1024));
    SCOPE_FREE(memText);
    WriteConsole(&state->console, memText);

    char *memTotalText = mprintf("Total memory: %d MB", mem_all.size / (1024 * 1024));
    SCOPE_FREE(memTotalText);
    WriteConsole(&state->console, memTotalText);
}

void DrawElapsedTime(GameState *state, RenderContext *ctx, v2i windowSize, r32 frameTime) {

    char *output = mprintf("%.2f seconds elapsed.", state->elapsedTime);
    SCOPE_FREE(output);
    DrawText(ctx, output, { windowSize.x - 300 , 0 }, Color(1.0f, 1.0f, 1.0f));

    char *lastFrame = mprintf("Frame time: %.2f", frameTime);
    SCOPE_FREE(lastFrame);
    DrawText(ctx, lastFrame, { windowSize.x - 300, 18}, Color(1.0f, 1.0f, 1.0f));
}

extern "C"
void EXPORT UpdateGame(PlatformState *platformState, GameMemory *memory, GameInput *input, r32 dt) {

    GameState *state = (GameState *)memory->permanent;    
    if (!state->initialized || state->restart) {
        GameInit(state, platformState->api, memory);
    }

    // TODO: Get rid of this ad-hoc loading of glew function pointers.
    if (platformState->libReloaded) {
        glewInit();
        WriteConsole(&state->console, "Loading game library.");
        platformState->libReloaded = false;
    }

    state->elapsedTime += dt;

    state->world->screenSize = platformState->windowSize;

    // Updating
    UpdateConsole(state, input);
    WorldUpdate(state->world, input, dt);

    // Rendering
    RenderStart(state->renderer, platformState->windowSize);
    WorldRender(state->world, state->renderer, platformState->windowSize);

    DrawElapsedTime(state, state->renderer, platformState->windowSize, platformState->lastFrameTime);
    DrawConsole(&state->console, state->renderer, state->world->screenSize);

    RenderEnd(state->renderer);

    if (state->shutdown) {
        platformState->shutdownRequested = true;
    }
}


