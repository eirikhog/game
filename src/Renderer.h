#pragma once

#include "Platform.h"
#include "Math.h"
#include "Assets.h"

struct RenderContext;

typedef struct {
    real32 r;
    real32 g;
    real32 b;
    real32 a;
} Color;

RenderContext *render_init(game_assets *assets, memory_segment memory);
void render_rect(RenderContext *ctx, int32 x, int32 y, int32 width, int32 height, Color c);
void render_rect(RenderContext *ctx, v2 pos, v2 size, Color c);

void render_start(RenderContext *ctx);
void render_end(RenderContext *ctx);
