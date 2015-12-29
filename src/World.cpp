#include "World.h"

// TODO: Remove dependency
#include <math.h>

static int32
coordinateToChunk(real32 value) {
    return (int32)floor(value / (CHUNK_SIZE * TILE_SIZE));
}

game_world *
create_world(memory_segment *memory) {
    game_world *world = PUSH_STRUCT(memory, game_world);
    uint32 chunks_dim = (uint32)sqrt(WORLD_SIZE);

    // Create some dummy chunks
    for (int32 i = 0; i < WORLD_SIZE; i++) {
        world_chunk *current = &world->chunks[i];
        current->id = i;
        current->chunk_x = (i % chunks_dim) - chunks_dim / 2;
        current->chunk_y = (i / chunks_dim) - chunks_dim / 2;
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                if (y == 0 || x == 0 || y == CHUNK_SIZE - 1 || x == CHUNK_SIZE - 1) {
                    current->tiles[y * CHUNK_SIZE + x] = TILE_SOLID;
                } else {
                    current->tiles[y * CHUNK_SIZE + x] = TILE_NONE;
                }
            }
        }
    }

    return world;
}

static world_chunk *
get_chunk(world_chunk *chunks, int32 x, int32 y) {
    for (int i = 0; i < WORLD_SIZE; ++i) {
        world_chunk *current = chunks + i;
        if (current->chunk_x == x && current->chunk_y == y) {
            return current;
        }
    }

    return NULL;
}

void world_update(game_world *world, game_input *input, real32 dt) {
    if (input->mouse_buttons & MOUSE_LEFT) {
        world->screen_position += input->mouse_delta;
    }
}

void world_render(game_world *world, render_context *renderer) {

    const int32 screen_width = 1920;
    const int32 screen_height = 1080;
    v2 center = world->screen_position;
    
    // Find which chunks we should display
    int32 num_chunks_x = 1 + screen_width / (TILE_SIZE * CHUNK_SIZE);
    int32 num_chunks_y = 1 + screen_height / (TILE_SIZE * CHUNK_SIZE);

    int32 max_x = coordinateToChunk(center.x + screen_width / 2);
    int32 min_x = coordinateToChunk(center.x - screen_width / 2);
    int32 max_y = coordinateToChunk(center.y + screen_height / 2);
    int32 min_y = coordinateToChunk(center.y - screen_height / 2);

    // Get all chunks and paint the relevant ones
    uint32 rendered_chunks = 0;
    for (int32 j = min_y; j <= max_y; ++j) {
        for (int32 i = min_x; i <= max_x; ++i) {
            world_chunk *chunk = get_chunk(world->chunks, i, j);
            if (chunk) {
                rendered_chunks++;
                v2 base = { -center.x + screen_width/2 + chunk->chunk_x * CHUNK_SIZE * TILE_SIZE,
                            -center.y + screen_height/2 + chunk->chunk_y * CHUNK_SIZE * TILE_SIZE };
                for (int y = 0; y < CHUNK_SIZE; ++y) {
                    for (int x = 0; x < CHUNK_SIZE; ++x) {
                        switch (chunk->tiles[y*CHUNK_SIZE + x]) {
                            case TILE_NONE: {
                                render_rect(renderer, (int32)(base.x + x * TILE_SIZE), (int32)(base.y + y * TILE_SIZE), TILE_SIZE, TILE_SIZE, { 1.0f, 1.0f, 1.0f });
                            }break;
                            case TILE_SOLID: {
                                render_rect(renderer, (int32)(base.x + x * TILE_SIZE), (int32)(base.y + y * TILE_SIZE), TILE_SIZE, TILE_SIZE, { 0.5f, 0.0f, 0.0f });
                            }break;
                            default:
                                InvalidCodePath();
                                break;
                        }
                    }
                }
            }
        }
    }
}


