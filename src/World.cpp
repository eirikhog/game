#include "World.h"
#include "Math.h"
#include "Platform.h"

static v2 MousePosition = {};
static v2 PlayerPosition = { 5, 5 };
static v2 PlayerSpeed = {};
static v2 PlayerAcceleration = {};

WorldChunk* get_chunk(World *world, int x, int y) {
    for (int i = 0; i < 16; ++i) {
        if (world->chunks[i].x == x && world->chunks[i].y == y) {
            return &(world->chunks[i]);
        }
    }

    return 0;
}

uint32 get_tile(World *world, real32 x, real32 y) {
    int32 chunkX = floor(x / CHUNK_DIM);
    int32 chunkY = floor(y / CHUNK_DIM);
    WorldChunk *chunk = get_chunk(world, chunkX, chunkY);
    if (chunk) {
        int32 localX = (int32)x % CHUNK_DIM;
        int32 localY = (int32)y % CHUNK_DIM;
        return chunk->tiles[localX + localY * CHUNK_DIM];
    }

    // TODO: This should never happen!
    Assert(0);
    return 0;
}

v2 get_tile_from_screen_position(World *world, v2 screenPos) {
    v2 camera = world->camera;
    v2 worldPosition = { screenPos.x - world->screenSize.x / 2 - world->camera.x,
                         screenPos.y - world->screenSize.y / 2 - world->camera.y };
    v2 result = { (int32)floor(worldPosition.x / TILE_SIZE), (int32)floor(worldPosition.y / TILE_SIZE) };
    return result;
}

void set_tile(World *world, real32 x, real32 y, uint32 tileValue) {
    int32 chunkX = floor(x / CHUNK_DIM);
    int32 chunkY = floor(y / CHUNK_DIM);
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
    }
    else {
        // TODO: This should never happen!
        Assert(0);
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
            world->chunks[i].tiles[j] = ATLAS_STONE;
        }
    }

#if 0
    // Create the first chunks
    int loadChunks[4][2] = { { -1, -1 }, { -1, 0 }, { 0, -1 }, { 0, 0 } };
    for (int i = 0; i < 2; ++i) {
        WorldChunk *chunk = get_chunk(world, loadChunks[i][0], loadChunks[i][1]);
        for (int tile = 0; tile < CHUNK_DIM * CHUNK_DIM; ++tile) {
            chunk->tiles[tile] = ATLAS_DIRT;
        }
    }
#endif

    set_tile(world, 0, -1, ATLAS_DIRT);
    set_tile(world, -1, 0, ATLAS_DIRT);
    set_tile(world, 0, 0, ATLAS_DIRT);
    set_tile(world, 0, 1, ATLAS_DIRT);
}

void world_update(World *world, game_input *input, real32 dt) {
    MousePosition = input->mouse_position;
    const real32 gravity = 20.0f;
    const real32 height = 1080.0f;

    v2 cameraMove = { input->joystick.x, -input->joystick.y };
    world->camera += cameraMove * 5.0f;

    bool32 isMoving = 0;
    PlayerAcceleration.x = 50 * input->joystick.x;
    if (input->buttons & BUTTON_RIGHT) {
        PlayerAcceleration.x = 50;
    } else if (input->buttons & BUTTON_LEFT) {
        PlayerAcceleration.x = -50;
    } else if (PlayerPosition.y == height - 180 - 100 && input->joystick.x < 0.1f && input->joystick.x > -0.1f) {
        PlayerAcceleration.x += PlayerSpeed.x * -15.f;
    }

    if (PlayerPosition.y == height - 180 - 100 && input->buttons & BUTTON_UP) {
        PlayerAcceleration.y = 0;
        PlayerSpeed.y -= 20;
    }

    PlayerSpeed += PlayerAcceleration * dt;
    if (PlayerSpeed.x > 20) {
        PlayerSpeed.x = 20;
    } else if (PlayerSpeed.x < -20) {
        PlayerSpeed.x = -20;
    }

    PlayerSpeed.y += dt * gravity;
    PlayerPosition += PlayerSpeed;
    if (PlayerPosition.y >= height - 180 - 100) {
        PlayerPosition.y = height - 180 - 100;
        PlayerSpeed.y = 0;
        PlayerAcceleration.y = 0;
    }

}

inline v2 chunk_coords_to_screen_coords(v2 camera, v2 screenSize, int x, int y) {
    const int32 chunkSideLength = (TILE_SIZE * CHUNK_DIM);
    v2 result = { camera.x + screenSize.x / 2.0f + x * chunkSideLength, camera.y + screenSize.y / 2.0f + y * chunkSideLength };
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

    render_rect(ctx, 0, 0, (int32)screenSize.x, (int32)screenSize.y, { 0.486f, 0.678f, 0.965f });

    uint32 texture = ATLAS_STONE;

    v2 center = world->camera;
    for (int32 y = floor((center.y / chunkSideLength) - chunksY / 2); y <= (center.y / chunkSideLength) + chunksY / 2; ++y) {
        for (int32 x = floor((center.x / chunkSideLength) - chunksX / 2); x <= (center.x / chunkSideLength) + chunksX / 2; ++x) {
            WorldChunk *chunk = get_chunk(world, x, y);
            v2 screenPos = chunk_coords_to_screen_coords(center, screenSize, x, y);
            // Render all tiles
            int tileIndexX = 0, tileIndexY = 0;
            for (int32 tileY = screenPos.y; tileY < screenPos.y + chunkSideLength; tileY += TILE_SIZE, ++tileIndexY) {
                tileIndexX = 0;
                for (int tileX = screenPos.x; tileX < screenPos.x + chunkSideLength; tileX +=  TILE_SIZE, ++tileIndexX) {
                    texture = chunk->tiles[tileIndexX + tileIndexY * CHUNK_DIM];
                    render_image(ctx, tileX, tileY, TILE_SIZE, TILE_SIZE, texture);
                }
            }
        }
    }

    //render_rect(ctx, 0, 0, width, height - 180, { 0.486f, 0.678f, 0.965f });
    //render_rect(ctx, 0, height - 180, width, 180, { 0.447f, 0.694f, 0.369f });

    //render_image(ctx, PlayerPosition.x, PlayerPosition.y, 100, 100, 2);
    render_rect(ctx, screenSize.x / 2, screenSize.y / 2, 1, 16, { 1.0f, 0.0f, 0.0f });
    render_rect(ctx, screenSize.x / 2, screenSize.y / 2, 16, 1, { 0.0f, 0.0f, 1.0f });

    render_rect(ctx, MousePosition.x - TILE_SIZE / 2, MousePosition.y-TILE_SIZE/2, TILE_SIZE, TILE_SIZE, white);
    
    v2 tilePos = get_tile_from_screen_position(world, MousePosition);
    set_tile(world, tilePos.x, tilePos.y, ATLAS_DIRT);
}
