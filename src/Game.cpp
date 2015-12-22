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

    render_start(&ctx);


    color cornflower_blue = { 0.392156862745098f, 0.5843137254901961f, 0.9294117647058824f };
    render_rect(&ctx, 0, 0, screen_size.x, screen_size.y, cornflower_blue);

    color c1 = { 1.0f, 1.0f, 1.0f, 1.0f };
    render_rect(&ctx, (int32)pos.x, (int32)pos.y, 100, 100, c1);

    //color c2 = { 0.0f, 1.0f, 0.0f, 1.0f };
    //render_rect(&ctx, (int32)pos.x + 75, (int32)pos.y + 75, 60, 60, c2);

    render_end(&ctx);
}


