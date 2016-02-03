#include "World.h"
#include "Math.h"
#include "Platform.h"

static v2 MousePosition = {};
static v2 PlayerPosition = { 5, 5 };
static v2 PlayerSpeed = {};
static v2 PlayerAcceleration = {};

// Create world from scratch
World world_create() {
    World myWorld = {};

    // Start at center
    myWorld.camera = { 0.0f, 0.0f };
    return myWorld;
}

void world_update(World *world, game_input *input, real32 dt) {
    MousePosition = input->mouse_position;
    const real32 gravity = 20.0f;
    const real32 height = 1080.0f;

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

WorldChunk* get_chunk(World *world, int x, int y) {
    return 0;
}

inline v2 chunk_coords_to_screen_coords(v2 camera, v2 screenSize, int x, int y) {
    const int32 chunkSideLength = (32 * CHUNK_DIM);
    v2 result = { camera.x + screenSize.x / 2.0f + x * chunkSideLength, camera.y + screenSize.y / 2.0f + y * chunkSideLength };
    return result;
}

void world_render(World *world, RenderContext *ctx, v2 windowSize) {

    const v2 screenSize = windowSize;

    // Render visible chunks
    const int32 chunkSideLength = (32 * CHUNK_DIM);
    int32 chunksX = 1 + ceil(screenSize.x / (real32)chunkSideLength);
    int32 chunksY = 1 + ceil(screenSize.y / (real32)chunkSideLength);

    Color white = { 1.0f, 1.0f, 1.0f };
    Color black = { 0.0f, 0.0f, 0.0f };

    render_rect(ctx, 0, 0, screenSize.x, screenSize.y, { 0.486f, 0.678f, 0.965f });

    uint32 texture = ATLAS_STONE;

    v2 center = world->camera;
    for (int32 y = (center.y / chunkSideLength) - chunksY / 2; y <= (center.y / chunkSideLength) + chunksY / 2; ++y) {
        for (int32 x = (center.x / chunkSideLength) - chunksX / 2; x <= (center.x / chunkSideLength) + chunksX / 2; ++x) {
            WorldChunk *chunk = get_chunk(world, x, y);
            v2 screenPos = chunk_coords_to_screen_coords(center, screenSize, x, y);
            // Render all tiles
            for (int32 tileY = screenPos.y; tileY < screenPos.y + chunkSideLength; tileY += TILE_SIZE) {
                for (int tileX = screenPos.x; tileX < screenPos.x + chunkSideLength; tileX +=  TILE_SIZE) {
                    texture = texture == ATLAS_STONE ? ATLAS_DIRT : ATLAS_STONE;
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

    MousePosition.x = screenSize.x * MousePosition.x / windowSize.x;
    MousePosition.y = screenSize.y * MousePosition.y / windowSize.y;
    render_rect(ctx, MousePosition.x - 2, MousePosition.y - 2, 5, 5, white);

    render_rect(ctx, 100, 100, 100, 100, white);
}
