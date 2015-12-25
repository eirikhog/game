#pragma once

#include "Platform.h"
#include "Renderer.h"

typedef struct {
    bool initialized;
    render_context *renderer;

    memory_segment game_memory;
    memory_segment renderer_memory;
} game_state;


