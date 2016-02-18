#pragma once

#include "Platform.h"
#include "Math.h"
#include "Assets.h"

struct RenderContext;

RenderContext *RenderInit(GameAssets *assets, MemorySegment memory);
void RenderStart(RenderContext *ctx, v2 windowSize);
void RenderEnd(RenderContext *ctx);

void DrawImage(RenderContext *ctx, Rect2Di r, AssetId id);
void DrawSolidRect(RenderContext *ctx, Rect2Di r, Color c);
void DrawRect(RenderContext *ctx, Rect2Di r, Color c);
