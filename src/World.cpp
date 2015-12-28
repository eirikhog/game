#include "World.h"

game_world *create_world(memory_segment *memory) {
    game_world *world = PUSH_STRUCT(memory, game_world);
    return world;
}

void world_update(real32 dt) {
}

void world_render(render_context *renderer) {

    // Get the chunks which should be rendered.
}


