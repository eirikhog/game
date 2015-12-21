#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"

typedef struct {
    v2 position;
    v2 size;
} player;

static render_context ctx;

extern "C"
void EXPORT UpdateGame(platform_api *api, game_memory *memory, game_input *input) {
    game_state *state = (game_state *)memory;
    
    if (!state->initialized) {
        // Do initialization.
        ctx = initialize_renderer(api);
        state->initialized = true;
    }

    color c = { 1.0f, 1.0f, 1.0f, 1.0f };
    render_rect(&ctx, 0, 0, 100, 100, c);

    render_start(&ctx);
}


