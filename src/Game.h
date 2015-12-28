#pragma once

#include "Platform.h"
#include "Renderer.h"
#include "Assets.h"

struct game_world;

typedef struct {
    bool initialized;
    render_context *renderer;

    memory_segment game_memory;
    memory_segment renderer_memory;

    game_assets assets;
    game_world *world;
} game_state;


