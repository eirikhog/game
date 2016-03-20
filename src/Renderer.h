#pragma once

#include "Platform.h"
#include "Math.h"
#include "Assets.h"

struct RenderContext;

RenderContext *RenderInit(GameAssets *assets, MemorySegment memory);
void RenderStart(RenderContext *ctx, v2i windowSize);
void RenderEnd(RenderContext *ctx);

void DrawImage(RenderContext *ctx, Rect2Di r, u32 id);
void DrawSolidRect(RenderContext *ctx, Rect2Di r, Color c);
void DrawRect(RenderContext *ctx, Rect2Di r, Color c);
void DrawText(RenderContext *ctx, const char *str, v2i position, Color c);

// Diagnostic functions
u32 RenderedObjects(RenderContext *ctx);
