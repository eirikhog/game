#pragma once

#include "Platform.h"
#include "Math.h"

struct render_entry;

typedef struct {
    bool initialized;
    bool rendering;
    render_entry *sprites;
} render_context;

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

render_context initialize_renderer(platform_api *api);
void render_rect(render_context *ctx, int32 x, int32 y, int32 width, int32 height, color c);

void render_start(render_context *ctx);
void render_end(render_context *ctx);