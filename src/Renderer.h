#pragma once

#include "Platform.h"
#include "Math.h"
#include "Assets.h"

// TODO: Remove global defines.
#define ATLAS_WHITE 0 
#define ATLAS_STONE 1
#define ATLAS_DIRT 2 

struct RenderContext;

typedef struct {
    real32 r;
    real32 g;
    real32 b;
    real32 a;
} Color;

RenderContext *render_init(GameAssets *assets, MemorySegment memory);
void render_rect(RenderContext *ctx, int32 x, int32 y, int32 width, int32 height, Color c);
void render_image(RenderContext *ctx, int32 x, int32 y, int32 width, int32 height, uint32 id);
void render_rect(RenderContext *ctx, v2 pos, v2 size, Color c);

void render_start(RenderContext *ctx);
void render_end(RenderContext *ctx);
