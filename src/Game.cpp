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
    GameWorld *world;
} game_state;

static void
initialize(game_state *state, platform_api *api, game_memory *memory) {
    MemorySegment mem_all = {};
    mem_all.size = memory->permanentSize - sizeof(game_state);
    mem_all.base = (uint8*)memory->permanent + sizeof(game_state);
    mem_all.used = 0;
    state->game_memory = mem_all;


    MemorySegment mem_transient = {};
    mem_transient.size = memory->transientSize;
    mem_transient.base = (uint8*)memory->transient;
    mem_transient.used = 0;
    state->assets = assets_initialize(api, mem_transient);

    // Allocate memory for renderer
    MemorySegment renderer_memory = allocate_memory(&mem_all, 128 * 1024 * 1024);
    state->renderer_memory = renderer_memory;
    state->renderer = render_init(&state->assets, renderer_memory);

    MemorySegment world_memory = allocate_memory(&mem_all, 8 * 1024 * 1024);
    state->world = create_world(&world_memory);

    state->initialized = true;
}

extern "C"
void EXPORT UpdateGame(platform_api *api, game_memory *memory, game_input *input) {

    game_state *state = (game_state *)memory->permanent;    
    if (!state->initialized) {
        initialize(state, api, memory);
    }

    // Updating
    world_update(state->world, input, 0.0f);

    // Rendering
    render_start(state->renderer);
    world_render(state->world, state->renderer);
    //render_rect(state->renderer, { 0, 0 }, { 20, 20 }, { 1.0f, 1.0f, 1.0f });
    render_end(state->renderer);
}


