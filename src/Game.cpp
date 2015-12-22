#include "Game.h"
#include "Platform.h"
#include "Math.h"
#include "Renderer.h"

typedef struct {
    v2 position;
    v2 size;
} player;

static render_context ctx;
static v2 pos;

extern "C"
void EXPORT UpdateGame(platform_api *api, game_memory *memory, game_input *input) {
    game_state *state = (game_state *)memory;
    
    if (!state->initialized) {
        ctx = initialize_renderer(api);
        state->initialized = true;
    }

    pos.x += 2.5f;
    if (pos.x > 1200) pos.x = 0;
    pos.y += 3.13f;
    if (pos.y > 700) pos.y = 0;

    render_start(&ctx);

    color cornflower_blue = { 0.392156862745098f, 0.5843137254901961f, 0.9294117647058824f };
    render_rect(&ctx, 0, 0, 1350, 750, cornflower_blue);

    color c1 = { 1.0f, 1.0f, 1.0f, 1.0f };
    render_rect(&ctx, (int32)pos.x, (int32)pos.y, 100, 100, c1);

    color c2 = { 0.0f, 1.0f, 0.0f, 1.0f };
    render_rect(&ctx, (int32)pos.x + 75, (int32)pos.y + 75, 60, 60, c2);

    render_end(&ctx);
}


