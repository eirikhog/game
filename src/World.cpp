#include "World.h"
#include "Math.h"
#include "Platform.h"

static v2 MousePosition = {};

void world_update(GameWorld *world, game_input *input, real32 dt) {

}

void world_render(GameWorld *world, RenderContext *ctx) {

    const uint32 width = 1920;
    const uint32 height = 1080;


    render_rect(ctx, 0, 0, width, height - 180, { 0.486f, 0.678f, 0.965f });
    render_rect(ctx, 0, height - 180, width, 180, { 0.447f, 0.694f, 0.369f });

    render_image(ctx, 240, 240, 100, 100, 2);
}


