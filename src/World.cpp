#include "World.h"
#include "Math.h"
#include "Platform.h"
#include "Chunk.h"

enum EntityType {
    EntityType_None,
    EntityType_Unit,
};

struct Entity {
    EntityType type;
    v2f position;
    bool32 selected;
};

#define CHUNK_DIM 16
#define TILE_SIZE 32
#define CHUNK_COUNT 64 
#define ENTITIES_MAX 128

typedef struct {
    i32 x;
    i32 y;
    u32 tiles[CHUNK_DIM*CHUNK_DIM];
} WorldChunk;

struct World {
    v2i camera;
    v2i screenSize;
    WorldChunk chunks[CHUNK_COUNT];
    v2i mousePos;
    bool32 mouseDrag;
    v2i mouseDragOrigin;

    u32 entityCount;
    Entity entities[ENTITIES_MAX];
};

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
ScreenCoordsToWorldCoords(World *world, v2i screenCoords) {
    v2i result(-world->screenSize.x / 2 - world->camera.x + screenCoords.x,
               -world->screenSize.y / 2 - world->camera.y + screenCoords.y);

    return result;
}

// Create world from scratch
void WorldCreate(World *world) {
    *world = {};

    // Start at center
    CenterOnChunk(world, { 0, 0 });

    for (i32 i = 0; i < CHUNK_COUNT; ++i) {
        world->chunks[i].x = (i % (i32)sqrt(CHUNK_COUNT))-3;
        world->chunks[i].y = (i / (i32)sqrt(CHUNK_COUNT))-3;
        for (i32 j = 0; j < CHUNK_DIM * CHUNK_DIM; ++j) {
            world->chunks[i].tiles[j] = ASSET_TEXTURE_GRASS;
        }
    }

    SetTile(world, 8, 7, ASSET_TEXTURE_DIRT);
    SetTile(world, 7, 8, ASSET_TEXTURE_DIRT);
    SetTile(world, 8, 8, ASSET_TEXTURE_DIRT);
    SetTile(world, 8, 9, ASSET_TEXTURE_DIRT);
    SetTile(world, 9, 8, ASSET_TEXTURE_DIRT);

    world->entities[0].type = EntityType_Unit;
    world->entities[0].position = { -100, -100 };
    world->entityCount++;
}

void WorldUpdate(World *world, GameInput *input, r32 dt) {

    world->mousePos = input->mouse_position;

    bool32 dragSelect = 0;
    Rect2Di dragTarget;

    if (input->mouse_buttons & MOUSE_RIGHT) {
        world->camera -= input->mouse_delta;
    }

    if (input->mouse_buttons & MOUSE_LEFT) {
        if (!world->mouseDrag) {
            world->mouseDragOrigin = world->mousePos;
            world->mouseDrag = 1;
        }

        //v2i tilePos = GetTileFromScreenPosition(world, input->mouse_position);
        //SetTile(world, tilePos.x, tilePos.y, ASSET_TEXTURE_SHROUD);
    } else {
        if (world->mouseDrag) {
            world->mouseDrag = 0;
            // Figure out what we selected...
            v2i worldCoords = ScreenCoordsToWorldCoords(world, world->mouseDragOrigin);
            Rect2Di targetRect(worldCoords.x, worldCoords.y,
                               world->mousePos.x - world->mouseDragOrigin.x, world->mousePos.y - world->mouseDragOrigin.y);
            dragTarget = targetRect;
            dragSelect = 1;
        }
    }

    for (u32 i = 0; i < world->entityCount; ++i) {
        Entity *e = &world->entities[i];

        if (dragSelect) {
            Rect2Di eRect((i32)e->position.x, (i32)e->position.y, TILE_SIZE, TILE_SIZE);
            if (Intersects(dragTarget, eRect)) {
                e->selected = 1;
            } else {
                e->selected = 0;
            }
        }

        e->position.x += 0.3f;
        e->position.y += 0.2f;
        if (e->position.x > 0.0f) {
            e->position = { -100, -100 };
        }
    }

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

void DrawDiagnostics(World *world, RenderContext *ctx) {
    const Color white = { 1.0f, 1.0f, 1.0f };
    char *text = mprintf("Screen size: %ix%i", world->screenSize.x, world->screenSize.y);
    SCOPE_FREE((void*)text);
    DrawText(ctx, text, { 0, 0 }, white);

    char *mouseText = mprintf("Mouse position: (%i, %i)", world->mousePos.x, world->mousePos.y);
    SCOPE_FREE(mouseText);
    DrawText(ctx, mouseText, { 0, 16 }, white);

    char *worldPosText = mprintf("Camera position: (%i, %i)", world->camera.x, world->camera.y);
    SCOPE_FREE(worldPosText);
    DrawText(ctx, worldPosText, { 0, 32 }, white);

    char *tilesText = mprintf("Drawn objects: %i", RenderedObjects(ctx));
    SCOPE_FREE(tilesText);
    DrawText(ctx, tilesText, { 0, 48 }, white);

    char *entityText = mprintf("Entities: %i", world->entityCount);
    SCOPE_FREE(entityText);
    DrawText(ctx, entityText, { 0, 64 }, white);
}

inline v2i WorldToScreenPosition(World *world, v2i worldPos) {
    v2i result = { world->camera.x + worldPos.x + world->screenSize.x / 2,
                   world->camera.y + worldPos.y + world->screenSize.y / 2 };
    return result;
}

void WorldRender(World *world, RenderContext *ctx, v2i windowSize) {

    const v2i screenSize = windowSize;

    Rect2Di drawWindow((i32)(-world->camera.x - windowSize.x / 2), (i32)(-world->camera.y - windowSize.y / 2), (i32)windowSize.x, (i32)windowSize.y);

    const i32 chunkSideLength = (TILE_SIZE * CHUNK_DIM);
    i32 chunkStartX = (i32)floorf((r32)drawWindow.x / chunkSideLength);
    i32 chunkStartY = (i32)floorf((r32)drawWindow.y / chunkSideLength);
    i32 chunkEndX = (i32)ceil((r32)(drawWindow.x + drawWindow.width) / chunkSideLength);
    i32 chunkEndY = (i32)ceil((r32)(drawWindow.y + drawWindow.height) / chunkSideLength);

    Color white = { 1.0f, 1.0f, 1.0f };
    Color black = { 0.0f, 0.0f, 0.0f };

    DrawSolidRect(ctx, Rect2Di(0, 0, (i32)screenSize.x, (i32)screenSize.y), { 0.486f, 0.678f, 0.965f });

    u32 texture = (u32)ASSET_TEXTURE_DIRT;
    i32 drawnTiles = 0;
    v2i center = world->camera;
    for (i32 y = chunkStartY; y <= chunkEndY; ++y) {
        for (i32 x = chunkStartX; x <= chunkEndX; ++x) {
            WorldChunk *chunk = GetChunk(world, x, y);
            if (!chunk) {
                continue;
            }

            v2i screenPos = ChunkCoordsToScreenCorrds(center, screenSize, x, y);

            // Don't draw stuff outside the window.
            i32 startX = screenPos.x < 0 ? (i32)abs((i32)screenPos.x / TILE_SIZE) : 0;
            i32 startY = screenPos.y < 0 ? (i32)abs((i32)screenPos.y / TILE_SIZE) : 0;
            i32 endX = CHUNK_DIM;
            if ((i32)screenPos.x + CHUNK_DIM * TILE_SIZE > world->screenSize.x) {
                i32 overflow = (i32)screenPos.x + CHUNK_DIM * TILE_SIZE - (i32)world->screenSize.x;
                endX = CHUNK_DIM - overflow / TILE_SIZE;
            }
            i32 endY = CHUNK_DIM;
            if ((i32)screenPos.y + CHUNK_DIM * TILE_SIZE > world->screenSize.y) {
                i32 overflow = (i32)screenPos.y + CHUNK_DIM * TILE_SIZE - (i32)world->screenSize.y;
                endY = CHUNK_DIM - overflow / TILE_SIZE;
            }

            for (i32 tileY = startY; tileY < endY; ++tileY) {
                for (i32 tileX = startX; tileX < endX; ++tileX) {
                    v2i offset = { (i32)tileX * TILE_SIZE, (i32)tileY * TILE_SIZE };
                    v2i pos = screenPos + offset;
                    texture = (AssetId)chunk->tiles[tileX + tileY * CHUNK_DIM];
                    DrawImage(ctx, Rect2Di((i32)pos.x, (i32)pos.y, TILE_SIZE, TILE_SIZE), texture);
                    drawnTiles++;
                }
            }

            //DrawRect(ctx, { (i32)screenPos.x, (i32)screenPos.y, CHUNK_DIM * TILE_SIZE, CHUNK_DIM * TILE_SIZE }, white);
        }
    }

    // TODO: Move entities to different chunks
    for (u32 i = 0; i < world->entityCount; ++i) {
        Entity *e = &world->entities[i];
        v2i screenPos = WorldToScreenPosition(world, { (i32)e->position.x, (i32)e->position.y });
        Rect2Di target(screenPos.x, screenPos.y, 32, 32);
        DrawImage(ctx, target, ASSET_TEXTURE_ENTITY);

        if (e->selected) {
            DrawRect(ctx, target, { 0.0f, 1.0f, 0.0f });
        }
    }

    if (world->mouseDrag) {
        Rect2Di dragArea = { world->mouseDragOrigin.x, world->mouseDragOrigin.y, 
            world->mousePos.x - world->mouseDragOrigin.x, world->mousePos.y - world->mouseDragOrigin.y };
        DrawRect(ctx, dragArea, white);
    }

    DrawSolidRect(ctx, Rect2Di((i32)screenSize.x / 2, (i32)screenSize.y / 2, 1, 16), { 1.0f, 0.0f, 0.0f });
    DrawSolidRect(ctx, Rect2Di((i32)screenSize.x / 2, (i32)screenSize.y / 2, 16, 1), { 0.0f, 0.0f, 1.0f });

    DrawDiagnostics(world, ctx);
}
