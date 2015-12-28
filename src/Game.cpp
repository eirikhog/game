#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"

extern "C"
void EXPORT UpdateGame(platform_api *api, game_memory *memory, game_input *input) {

    game_state *state = (game_state *)memory->permanent;    
    if (!state->initialized) {
        memory_segment mem_all = {};
        mem_all.size = memory->permanentSize - sizeof(game_state);
        mem_all.base = (uint8*)memory->permanent + sizeof(game_state);
        mem_all.used = 0;
        state->game_memory = mem_all;

        state->assets = assets_initialize(api);

        // Allocate 4 mb for renderer
        memory_segment renderer_memory = allocate_memory(&mem_all, 4 * 1024 * 1024);
        state->renderer_memory = renderer_memory;
        state->renderer = render_init(&state->assets, renderer_memory);

        state->initialized = true;
    }

    static v2 pos = { 50, 50 };
    static v2 acc = { 5, 7 };
    const v2 screen_size = { 1920.0f, 1080.0f };
    pos.x += acc.x;
    if (pos.x + 100 > screen_size.x || pos.x < 0) {
        acc.x *= -1;
    }

    pos.y += acc.y;
    if (pos.y + 100 > screen_size.y || pos.y < 0) {
        acc.y *= -1;
    }

    render_start(state->renderer);

    color cornflower_blue = { 0.392156862745098f, 0.5843137254901961f, 0.9294117647058824f };
    render_rect(state->renderer, 0, 0, (int32)screen_size.x, (int32)screen_size.y, cornflower_blue);

    color c1 = { 1.0f, 1.0f, 1.0f, 1.0f };
    render_rect(state->renderer, (int32)pos.x, (int32)pos.y, 100, 100, c1);

    color c2 = { 0.0f, 1.0f, 0.0f, 1.0f };
    render_rect(state->renderer, (int32)pos.x + 75, (int32)pos.y + 75, 60, 60, c2);

    render_end(state->renderer);
}


