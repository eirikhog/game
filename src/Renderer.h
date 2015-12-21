#pragma once

#include "Platform.h"

typedef struct {
    bool initialized;
} render_context;

typedef struct {
    real32 r;
    real32 g;
    real32 b;
    real32 a;
} color;

typedef struct {
    real32 *vertices;
    uint32 verticesCount;
    real32 *colors;
    uint32 colorsCount;
} render_object;

render_context initialize_renderer(platform_api *api);
void render_rect(render_context *ctx, int32 x, int32 y, int32 width, int32 height, color c);

void render_start(render_context *ctx);

