#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

typedef struct {
    bool initialized;
    RenderContext *renderer;
    MemorySegment game_memory;
    MemorySegment renderer_memory;
    GameAssets assets;
    World *world;
} game_state;

static void
game_init(game_state *state, platform_api *api, game_memory *memory) {
    MemorySegment mem_all = {};
    mem_all.size = memory->permanentSize - sizeof(game_state);
    mem_all.base = (uint8*)memory->permanent + sizeof(game_state);
    mem_all.used = 0;
    state->game_memory = mem_all;

    MemorySegment mem_transient = {};
    mem_transient.size = memory->transientSize;
    mem_transient.base = (uint8*)memory->transient;
    mem_transient.used = 0;
    state->assets = assets_init(api, mem_transient);

    // Allocate memory for renderer
    MemorySegment renderer_memory = allocate_memory(&mem_all, 16 * 1024 * 1024);
    state->renderer_memory = renderer_memory;
    state->renderer = render_init(&state->assets, renderer_memory);

    MemorySegment world_memory = allocate_memory(&mem_all, sizeof(World));
    state->world = (World*)world_memory.base;
    world_create(state->world);

    state->initialized = true;
}

extern "C"
void EXPORT UpdateGame(platform_state *platformState, game_memory *memory, game_input *input, real32 dt) {

    game_state *state = (game_state *)memory->permanent;    
    if (!state->initialized) {
        game_init(state, platformState->api, memory);
    }

    state->world->screenSize = platformState->windowSize;

    // Updating
    world_update(state->world, input, dt);

    // Rendering
    render_start(state->renderer, platformState->windowSize);
    world_render(state->world, state->renderer, platformState->windowSize);
    render_end(state->renderer);
}


