#include "World.h"
#include "Math.h"
#include "Platform.h"

WorldChunk* get_chunk(World *world, int x, int y) {
    for (int i = 0; i < 16; ++i) {
        if (world->chunks[i].x == x && world->chunks[i].y == y) {
            return &(world->chunks[i]);
        }
    }

    return 0;
}

uint32 get_tile(World *world, real32 x, real32 y) {
    int32 chunkX = (int32)floor(x / CHUNK_DIM);
    int32 chunkY = (int32)floor(y / CHUNK_DIM);
    WorldChunk *chunk = get_chunk(world, chunkX, chunkY);
    if (chunk) {
        int32 localX = (int32)x % CHUNK_DIM;
        int32 localY = (int32)y % CHUNK_DIM;
        return chunk->tiles[localX + localY * CHUNK_DIM];
    }

    InvalidCodePath();
    return 0;
}

v2 get_tile_from_screen_position(World *world, v2 screenPos) {
    v2 camera = world->camera;
    v2 worldPosition = { screenPos.x - world->screenSize.x / 2 - world->camera.x,
                         screenPos.y - world->screenSize.y / 2 - world->camera.y };
    v2 result = { (real32)floor(worldPosition.x / TILE_SIZE), (real32)floor(worldPosition.y / TILE_SIZE) };
    return result;
}

void set_tile(World *world, real32 x, real32 y, uint32 tileValue) {
    int32 chunkX = (int32)floor(x / CHUNK_DIM);
    int32 chunkY = (int32)floor(y / CHUNK_DIM);
    WorldChunk *chunk = get_chunk(world, chunkX, chunkY);
    if (chunk) {
        int32 localX = (int32)x % CHUNK_DIM;
        if (localX < 0) {
            localX = localX + CHUNK_DIM;
        }
        int32 localY = (int32)y % CHUNK_DIM;
        if (localY < 0) {
            localY += CHUNK_DIM;
        }

        chunk->tiles[localX + localY * CHUNK_DIM] = tileValue;
    } else {
        InvalidCodePath();
    }
}

// Create world from scratch
void world_create(World *world) {
    *world = {};

    // Start at center
    world->camera = { 0.0f, 0.0f };

    for (int i = 0; i < 32; ++i) {
        world->chunks[i].x = (i % 4)-2;
        world->chunks[i].y = (i / 4)-2;
        for (int j = 0; j < CHUNK_DIM * CHUNK_DIM; ++j) {
            world->chunks[i].tiles[j] = ASSET_TEXTURE_STONE;
        }
    }

    set_tile(world, 0, -1, ASSET_TEXTURE_DIRT);
    set_tile(world, -1, 0, ASSET_TEXTURE_DIRT);
    set_tile(world, 0, 0, ASSET_TEXTURE_DIRT);
    set_tile(world, 0, 1, ASSET_TEXTURE_DIRT);
    set_tile(world, 1, 0, ASSET_TEXTURE_DIRT);
}

void world_update(World *world, game_input *input, real32 dt) {

    if (input->mouse_buttons & MOUSE_RIGHT) {
        world->camera -= input->mouse_delta;
    }

    if (input->mouse_buttons & MOUSE_LEFT) {
        v2 tilePos = get_tile_from_screen_position(world, input->mouse_position);
        set_tile(world, tilePos.x, tilePos.y, ASSET_TEXTURE_DIRT);
    }

}

inline v2 chunk_coords_to_screen_coords(v2 camera, v2 screenSize, int x, int y) {
    const int32 chunkSideLength = (TILE_SIZE * CHUNK_DIM);
    v2 result = { camera.x + screenSize.x / 2.0f + x * chunkSideLength, camera.y + screenSize.y / 2.0f + y * chunkSideLength };
    return result;
}

inline v2 get_tile_coordinate(v2 camera, v2 screenSize, v2 position) {
    v2 worldPosition = { position.x - screenSize.x / 2 - camera.x,
                         position.y - screenSize.y / 2 - camera.y };
    v2 tile = { (real32)floor(worldPosition.x / TILE_SIZE), (real32)floor(worldPosition.y / TILE_SIZE) };

    v2 result = { camera.x + screenSize.x / 2 + tile.x * TILE_SIZE, camera.y + screenSize.y / 2 + tile.y * TILE_SIZE };

    return result;
}

void world_render(World *world, RenderContext *ctx, v2 windowSize) {

    const v2 screenSize = windowSize;

    // Render visible chunks
    const int32 chunkSideLength = (TILE_SIZE * CHUNK_DIM);
    int32 chunksX = (int32)(1 + ceil(screenSize.x / (real32)chunkSideLength));
    int32 chunksY = (int32)(1 + ceil(screenSize.y / (real32)chunkSideLength));

    Color white = { 1.0f, 1.0f, 1.0f };
    Color black = { 0.0f, 0.0f, 0.0f };

    //render_rect(ctx, 0, 0, (int32)screenSize.x, (int32)screenSize.y, { 0.486f, 0.678f, 0.965f });

    AssetId texture = ASSET_TEXTURE_DIRT;

    v2 center = world->camera;
    for (int32 y = (int32)floor((center.y / chunkSideLength) - chunksY / 2); y < (center.y / chunkSideLength) + chunksY / 2; ++y) {
        for (int32 x = (int32)floor((center.x / chunkSideLength) - chunksX / 2); x < (center.x / chunkSideLength) + chunksX / 2; ++x) {
            WorldChunk *chunk = get_chunk(world, x, y);
            v2 screenPos = chunk_coords_to_screen_coords(center, screenSize, x, y);

            // Don't draw stuff outside the window.
            int32 startX = screenPos.x < 0 ? (int32)abs((int32)screenPos.x / TILE_SIZE) : 0;
            int32 startY = screenPos.y < 0 ? (int32)abs((int32)screenPos.y / TILE_SIZE) : 0;
            int32 endX = CHUNK_DIM;
            if ((int32)screenPos.x + CHUNK_DIM * TILE_SIZE > world->screenSize.x) {
                int32 overflow = endX * TILE_SIZE - (int32)world->screenSize.x;
                endX = CHUNK_DIM - overflow / TILE_SIZE;
            }
            int32 endY = CHUNK_DIM;
            if ((int32)screenPos.y + CHUNK_DIM * TILE_SIZE > world->screenSize.y) {
                int32 overflow = endY * TILE_SIZE - (int32)world->screenSize.y;
                endY = CHUNK_DIM - overflow / TILE_SIZE;
            }

            for (int32 tileY = startY; tileY < endY; ++tileY) {
                for (int32 tileX = startX; tileX < endX; ++tileX) {
                    v2 offset = { (real32)tileX * TILE_SIZE, (real32)tileY * TILE_SIZE };
                    v2 pos = screenPos + offset;
                    texture = (AssetId)chunk->tiles[tileX + tileY * CHUNK_DIM];
                    render_image(ctx, (int32)pos.x, (int32)pos.y, TILE_SIZE, TILE_SIZE, texture);
                }
            }
        }
    }

    render_rect(ctx, (int32)screenSize.x / 2, (int32)screenSize.y / 2, 1, 16, { 1.0f, 0.0f, 0.0f });
    render_rect(ctx, (int32)screenSize.x / 2, (int32)screenSize.y / 2, 16, 1, { 0.0f, 0.0f, 1.0f });

    //v2 target = get_tile_coordinate(world->camera, screenSize, MousePosition);
    //render_image(ctx, (int32)target.x, (int32)target.y, TILE_SIZE, TILE_SIZE, ASSET_TEXTURE_MARKER);
}
