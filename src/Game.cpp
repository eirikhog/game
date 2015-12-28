#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"
#include "World.h"

static void
initialize(game_state *state, platform_api *api, game_memory *memory) {
    memory_segment mem_all = {};
    mem_all.size = memory->permanentSize - sizeof(game_state);
    mem_all.base = (uint8*)memory->permanent + sizeof(game_state);
    mem_all.used = 0;
    state->game_memory = mem_all;


    memory_segment mem_transient = {};
    mem_transient.size = memory->transientSize;
    mem_transient.base = (uint8*)memory->transient;
    mem_transient.used = 0;
    state->assets = assets_initialize(api, mem_transient);

    // Allocate 4 mb for renderer
    memory_segment renderer_memory = allocate_memory(&mem_all, 4 * 1024 * 1024);
    state->renderer_memory = renderer_memory;
    state->renderer = render_init(&state->assets, renderer_memory);

    state->world = create_world(&state->game_memory);

    state->initialized = true;
}

extern "C"
void EXPORT UpdateGame(platform_api *api, game_memory *memory, game_input *input) {

    game_state *state = (game_state *)memory->permanent;    
    if (!state->initialized) {
        initialize(state, api, memory);
    }

    // Updating
    world_update(0.0f);

    // Rendering
    render_start(state->renderer);
    world_render(state->renderer);
    render_end(state->renderer);
}


