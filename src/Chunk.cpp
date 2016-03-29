#include "Chunk.h"
#include "World.h"
#include "Math.h"

// Concept:
// A "chunk" is a region on the game world, which includes elements
// on the chunk. Chunks are loaded when they are required, ie. when
// the player is near it and it need to be rendered, or if pathing
// requires inspection of the chunk.

//WorldChunk *LoadChunk(v2i chunkCoordinate) {
//    return 0;
//}

//void SaveChunk(WorldChunk *chunk) {
//    // Save the state of the chunk to persistent storage (disk)
//}


static WorldChunk*
GetChunk(World *world, i32 x, i32 y) {
    for (i32 i = 0; i < CHUNK_COUNT; ++i) {
        if (world->chunks[i].x == x && world->chunks[i].y == y) {
            return &(world->chunks[i]);
        }
    }

    return 0;
}

static u32
GetTile(World *world, r32 x, r32 y) {
    i32 chunkX = (i32)floor(x / CHUNK_DIM);
    i32 chunkY = (i32)floor(y / CHUNK_DIM);
    WorldChunk *chunk = GetChunk(world, chunkX, chunkY);
    if (chunk) {
        i32 localX = (i32)x % CHUNK_DIM;
        i32 localY = (i32)y % CHUNK_DIM;
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
    v2i result = { (i32)floor((r32)worldPosition.x / TILE_SIZE), (i32)floor((r32)worldPosition.y / TILE_SIZE) };
    return result;
}

void SetTile(World *world, i32 x, i32 y, u32 tileValue) {
    i32 chunkX = (i32)floor((r32)x / CHUNK_DIM);
    i32 chunkY = (i32)floor((r32)y / CHUNK_DIM);
    WorldChunk *chunk = GetChunk(world, chunkX, chunkY);
    if (chunk) {
        i32 localX = (i32)x % CHUNK_DIM;
        if (localX < 0) {
            localX = localX + CHUNK_DIM;
        }
        i32 localY = (i32)y % CHUNK_DIM;
        if (localY < 0) {
            localY += CHUNK_DIM;
        }

        chunk->tiles[localX + localY * CHUNK_DIM] = tileValue;
    } else {
        InvalidCodePath();
    }
}

void CenterOnTile(World *world, v2i tile) {
    v2i newCamera = tile * -TILE_SIZE;
    newCamera += { -TILE_SIZE / 2, -TILE_SIZE / 2 };
    world->camera = newCamera;
}

void CenterOnChunk(World *world, v2i chunk) {
    i32 chunkWidth = (TILE_SIZE * CHUNK_DIM);
    v2i newCamera = chunk * chunkWidth;
    newCamera += { -chunkWidth / 2, -chunkWidth / 2 };
    newCamera += { -TILE_SIZE / 2, -TILE_SIZE / 2};
    world->camera = newCamera;
}

static inline v2i
ChunkCoordsToScreenCorrds(v2i camera, v2i screenSize, i32 x, i32 y) {
    const i32 chunkSideLength = (TILE_SIZE * CHUNK_DIM);
    v2i result = { (i32)(camera.x + screenSize.x / 2.0f + x * chunkSideLength),
                   (i32)(camera.y + screenSize.y / 2.0f + y * chunkSideLength) };
    return result;
}

static inline v2i
GetTileCoordinate(v2i camera, v2i screenSize, v2i position) {
    v2i worldPosition = { position.x - screenSize.x / 2 - camera.x,
                          position.y - screenSize.y / 2 - camera.y };
    v2i tile = { (i32)floor(worldPosition.x / TILE_SIZE), (i32)floor(worldPosition.y / TILE_SIZE) };

    v2i result = { camera.x + screenSize.x / 2 + tile.x * TILE_SIZE, camera.y + screenSize.y / 2 + tile.y * TILE_SIZE };

    return result;
}

