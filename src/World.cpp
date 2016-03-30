#include "World.h"
#include "Math.h"
#include "Platform.h"
#include "Chunk.h"

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

    for (i32 i = 0; i < 20; ++i) {
        SetTile(world, i / 10, i % 10, ASSET_TEXTURE_STONE);
    }

    v2f origins[] = { v2f(-100.0f, -100.0f), v2f(500.0f, 100.0f), v2f(534, 1234), v2f(-4562, -1455) };
    v2f targets[] = { v2f(224.0f, 224.0f), v2f(288.0f, 288.0f), v2f(224, 288), v2f(288, 224) };

    for (i32 i = 0; i < 4; ++i) {
        world->entities[i].type = EntityType_Unit;
        world->entities[i].position = origins[i];
        world->entities[i].moveTarget = targets[i];
        world->entityCount++;
    }
}

void WorldUpdate(World *world, GameInput *input, r32 dt) {

    world->mousePos = input->mouse_position;

    bool32 dragSelect = 0;
    Rect2Di dragTarget;
    bool32 setMovePos = 0;
    v2i movePos;

    if (input->mouse_buttons & MOUSE_RIGHT) {
        world->mouseRightHoldTime += dt;
        if (magnitude(input->mouse_delta) > 5.0f || world->mouseRightHoldTime > 0.5f) {
            world->camera -= input->mouse_delta;
            world->cameraMoving = 1;
        }
    } else {
        if (!world->cameraMoving && world->mouseRightHoldTime > 0.0f && world->mouseRightHoldTime < 0.2f) {
            setMovePos = 1;
            movePos = ScreenCoordsToWorldCoords(world, world->mousePos);
        }
        world->cameraMoving = 0;
        world->mouseRightHoldTime = 0.0f;
    }

    if (input->mouse_buttons & MOUSE_LEFT) {
        world->mouseLeftHoldTime += dt;
        if (!world->mouseDrag) {
            world->mouseDragOrigin = world->mousePos;
            world->mouseDrag = 1;
        }

        //v2i tilePos = GetTileFromScreenPosition(world, input->mouse_position);
        //SetTile(world, tilePos.x, tilePos.y, ASSET_TEXTURE_SHROUD);
    } else {
        world->mouseLeftHoldTime = 0.0f;
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
        v2f prevPos = e->position;

        if (dragSelect) {
            Rect2Di eRect((i32)e->position.x, (i32)e->position.y, TILE_SIZE, TILE_SIZE);
            if (Intersects(dragTarget, eRect)) {
                e->selected = 1;
            } else {
                e->selected = 0;
            }
        }

        if (setMovePos && e->selected) {
            e->moveTarget = { (r32)movePos.x - TILE_SIZE/2, (r32)movePos.y - TILE_SIZE/2 };
        }

        // TODO: This is probably too slow...
        if (magnitude(e->position - e->moveTarget) > 1.0f) {
            v2f direction = unit(e->moveTarget - e->position);
            r32 speed = 2.0f;
            e->position += direction * speed;
        } else {
            e->position = e->moveTarget;
        }

        // TODO: Collision detection, proper
        // We want to compare only entities in this and adjacent chunks.
        Rect2Di a((i32)e->position.x, (i32)e->position.y, TILE_SIZE, TILE_SIZE);
        for (u32 j = 0; j < world->entityCount; ++j) {
            if (j == i) {
                continue;
            }
            Rect2Di b((i32)world->entities[j].position.x, (i32)world->entities[j].position.y, TILE_SIZE, TILE_SIZE);
            if (Intersects(a, b)) {
                e->position = prevPos;
                //e->moveTarget = prevPos;
            }

        }
    }

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
