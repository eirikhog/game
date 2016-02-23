#include "World.h"
#include "Math.h"
#include "Platform.h"

static WorldChunk*
GetChunk(World *world, int x, int y) {
    for (int i = 0; i < 16; ++i) {
        if (world->chunks[i].x == x && world->chunks[i].y == y) {
            return &(world->chunks[i]);
        }
    }

    return 0;
}

static uint32
GetTile(World *world, real32 x, real32 y) {
    int32 chunkX = (int32)floor(x / CHUNK_DIM);
    int32 chunkY = (int32)floor(y / CHUNK_DIM);
    WorldChunk *chunk = GetChunk(world, chunkX, chunkY);
    if (chunk) {
        int32 localX = (int32)x % CHUNK_DIM;
        int32 localY = (int32)y % CHUNK_DIM;
        return chunk->tiles[localX + localY * CHUNK_DIM];
    }

    InvalidCodePath();
    return 0;
}

static v2i
GetTileFromScreenPosition(World *world, v2i screenPos) {
    v2i camera = world->camera;
    v2i worldPosition = { screenPos.x - world->screenSize.x / 2 - world->camera.x,
                         screenPos.y - world->screenSize.y / 2 - world->camera.y };
    v2i result = { (int32)floor((real32)worldPosition.x / TILE_SIZE), (int32)floor((real32)worldPosition.y / TILE_SIZE) };
    return result;
}

void SetTile(World *world, int32 x, int32 y, uint32 tileValue) {
    int32 chunkX = (int32)floor((real32)x / CHUNK_DIM);
    int32 chunkY = (int32)floor((real32)y / CHUNK_DIM);
    WorldChunk *chunk = GetChunk(world, chunkX, chunkY);
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
void WorldCreate(World *world) {
    *world = {};

    // Start at center
    world->camera = { 0, 0 };

    for (int i = 0; i < 32; ++i) {
        world->chunks[i].x = (i % 4)-2;
        world->chunks[i].y = (i / 4)-2;
        for (int j = 0; j < CHUNK_DIM * CHUNK_DIM; ++j) {
            world->chunks[i].tiles[j] = ASSET_TEXTURE_STONE;
        }
    }

    SetTile(world, 0, -1, ASSET_TEXTURE_DIRT);
    SetTile(world, -1, 0, ASSET_TEXTURE_DIRT);
    SetTile(world, 0, 0, ASSET_TEXTURE_DIRT);
    SetTile(world, 0, 1, ASSET_TEXTURE_DIRT);
    SetTile(world, 1, 0, ASSET_TEXTURE_DIRT);
}

void WorldUpdate(World *world, game_input *input, real32 dt) {

    if (input->mouse_buttons & MOUSE_RIGHT) {
        world->camera -= input->mouse_delta;
    }

    if (input->mouse_buttons & MOUSE_LEFT) {
        v2i tilePos = GetTileFromScreenPosition(world, input->mouse_position);
        SetTile(world, tilePos.x, tilePos.y, ASSET_TEXTURE_SHROUD);
    }

}

static inline v2i
ChunkCoordsToScreenCorrds(v2i camera, v2i screenSize, int x, int y) {
    const int32 chunkSideLength = (TILE_SIZE * CHUNK_DIM);
    v2i result = { (int32)(camera.x + screenSize.x / 2.0f + x * chunkSideLength),
                   (int32)(camera.y + screenSize.y / 2.0f + y * chunkSideLength) };
    return result;
}

static inline v2i
GetTileCoordinate(v2i camera, v2i screenSize, v2i position) {
    v2i worldPosition = { position.x - screenSize.x / 2 - camera.x,
                          position.y - screenSize.y / 2 - camera.y };
    v2i tile = { (int32)floor(worldPosition.x / TILE_SIZE), (int32)floor(worldPosition.y / TILE_SIZE) };

    v2i result = { camera.x + screenSize.x / 2 + tile.x * TILE_SIZE, camera.y + screenSize.y / 2 + tile.y * TILE_SIZE };

    return result;
}

void WorldRender(World *world, RenderContext *ctx, v2i windowSize) {

    const v2i screenSize = windowSize;

    Rect2Di drawWindow((int32)(-world->camera.x - windowSize.x / 2), (int32)(-world->camera.y - windowSize.y / 2), (int32)windowSize.x, (int32)windowSize.y);

    const int32 chunkSideLength = (TILE_SIZE * CHUNK_DIM);
    int32 chunkStartX = (int32)floorf((real32)drawWindow.x / chunkSideLength);
    int32 chunkStartY = (int32)floorf((real32)drawWindow.y / chunkSideLength);
    int32 chunkEndX = (int32)ceil((real32)(drawWindow.x + drawWindow.width) / chunkSideLength);
    int32 chunkEndY = (int32)ceil((real32)(drawWindow.y + drawWindow.height) / chunkSideLength);

    Color white = { 1.0f, 1.0f, 1.0f };
    Color black = { 0.0f, 0.0f, 0.0f };

    DrawSolidRect(ctx, Rect2Di(0, 0, (int32)screenSize.x, (int32)screenSize.y), { 0.486f, 0.678f, 0.965f });

    AssetId texture = ASSET_TEXTURE_DIRT;

    v2i center = world->camera;
    for (int32 y = chunkStartY; y <= chunkEndY; ++y) {
        for (int32 x = chunkStartX; x <= chunkEndX; ++x) {
            WorldChunk *chunk = GetChunk(world, x, y);
            if (!chunk) {
                continue;
            }

            v2i screenPos = ChunkCoordsToScreenCorrds(center, screenSize, x, y);

            // Don't draw stuff outside the window.
            int32 startX = screenPos.x < 0 ? (int32)abs((int32)screenPos.x / TILE_SIZE) : 0;
            int32 startY = screenPos.y < 0 ? (int32)abs((int32)screenPos.y / TILE_SIZE) : 0;
            int32 endX = CHUNK_DIM;
            if ((int32)screenPos.x + CHUNK_DIM * TILE_SIZE > world->screenSize.x) {
                int32 overflow = (int32)screenPos.x + CHUNK_DIM * TILE_SIZE - (int32)world->screenSize.x;
                endX = CHUNK_DIM - overflow / TILE_SIZE;
            }
            int32 endY = CHUNK_DIM;
            if ((int32)screenPos.y + CHUNK_DIM * TILE_SIZE > world->screenSize.y) {
                int32 overflow = (int32)screenPos.y + CHUNK_DIM * TILE_SIZE - (int32)world->screenSize.y;
                endY = CHUNK_DIM - overflow / TILE_SIZE;
            }

            for (int32 tileY = startY; tileY < endY; ++tileY) {
                for (int32 tileX = startX; tileX < endX; ++tileX) {
                    v2i offset = { (int32)tileX * TILE_SIZE, (int32)tileY * TILE_SIZE };
                    v2i pos = screenPos + offset;
                    texture = (AssetId)chunk->tiles[tileX + tileY * CHUNK_DIM];
                    DrawImage(ctx, Rect2Di((int32)pos.x, (int32)pos.y, TILE_SIZE, TILE_SIZE), texture);
                }
            }

            DrawRect(ctx, { (int32)screenPos.x, (int32)screenPos.y, CHUNK_DIM * TILE_SIZE, CHUNK_DIM * TILE_SIZE }, white);
        }
    }

    DrawSolidRect(ctx, Rect2Di((int32)screenSize.x / 2, (int32)screenSize.y / 2, 1, 16), { 1.0f, 0.0f, 0.0f });
    DrawSolidRect(ctx, Rect2Di((int32)screenSize.x / 2, (int32)screenSize.y / 2, 16, 1), { 0.0f, 0.0f, 1.0f });
}
