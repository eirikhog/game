#pragma once

#include "Platform.h"
#include "Math.h"
#include "Assets.h"

struct render_context;

typedef struct {
    real32 r;
    real32 g;
    real32 b;
    real32 a;
} color;

typedef struct {
    v3 vertices[4];
    v3 colors[4];
} render_sprite;

render_context *render_init(game_assets *assets, memory_segment memory);
void render_rect(render_context *ctx, int32 x, int32 y, int32 width, int32 height, color c);

void render_start(render_context *ctx);
void render_end(render_context *ctx);
