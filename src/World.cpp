
#define WORLD_FILE "world.dat"

bool32 LoadWorld(PlatformAPI *platform, World **world) {
    // TODO: Load the world...
    //
    return 0;
}

bool32 SaveWorld(PlatformAPI *platform) {

    // Save world file.

    return 0;
}

static inline v2i
ScreenCoordsToWorldCoords(World *world, v2i screenCoords) {
    v2i result(-world->screenSize.x / 2 - world->camera.x + screenCoords.x,
               -world->screenSize.y / 2 - world->camera.y + screenCoords.y);

    return result;
}

void AddEntity(World *world, Entity e) {
    world->loadedEntities[world->loadedEntitiesCount++] = e;
}

// Check if a tile is passable
// TODO: Improve!
inline bool32 IsPassable(u32 tile) {
    return tile != ASSET_TEXTURE_STONE;
}

bool32 ContainsTile(PathfinderTile *tiles, u32 tileCount, v2i coords) {
    for (u32 i = 0; i < tileCount; ++i) {
        if (tiles[i].position == coords) {
            return 1;
        }
    }

    return 0;
}

u32 NodesInPath(PathfinderTile *end) {
    
    v2i prevMoveDelta;
    v2i prevPosition;

    u32 nodes = 0;
    PathfinderTile *current = end;
    while (current) {

        v2i moveDelta = current->position - prevPosition;
        // TODO: Count only direction chanes.
        nodes++;
        prevMoveDelta = moveDelta;
        prevPosition = current->position;

        current = current->prevTile;
    }

    return nodes;
}

MoveWaypoint *ReconstructPath(World *world, PathfinderTile *end) {

    u32 nodes = NodesInPath(end);
    MoveWaypoint *result = (MoveWaypoint*)Allocate(&world->transientMemory, sizeof(MoveWaypoint)*nodes);
    Assert(result != NULL);

    // Create the nodes
    v2i prevMoveDelta;
    v2i prevPosition;

    PathfinderTile *current = end;
    MoveWaypoint *next = 0;
    PathfinderTile *prev = end;
    u32 i = nodes;
    while (current) {

        // TODO: We only need to keep waypoints when direction changes.
        v2i moveDelta = current->position - prevPosition;
        MoveWaypoint *wp = result + (i - 1);
        --i;
        wp->position = current->position * TILE_SIZE + TILE_SIZE / 2;
        wp->next = next;
        next = wp;
        //char *wpCreateLog = mprintf("Creating waypoint at (%d, %d).", wp->position.x, wp->position.y);
        //SCOPE_FREE(wpCreateLog);
        //WriteConsole(world->console, wpCreateLog);

        prevMoveDelta = moveDelta;
        prevPosition = current->position;

        current = current->prevTile;
    }

    char *reconstructLog = mprintf("Reconstructing path with %d nodes.", nodes);
    SCOPE_FREE(reconstructLog);
    WriteConsole(world->console, reconstructLog);

    return result;
}

MoveWaypoint* FindPath(World *world, v2i start, v2i end) {
    // TODO: Implement
    // Find a valid path between two points, and create waypoints
    // between them.
    //
    // Idea for implementation:
    // (0. Check if points are in two connected regions)
    // 1. Do a A* search for shortest path
    // 2. Convert the path to waypoints
    // 3. Return a structure containing the path.
    //
    // Limit resources: If no path is found within X memory/time, return preliminary path.
    //

    // TODO: Cannot alloc this way, we need to free the memory (cannot use a stack)
    u32 memorySize = Kilobytes(128);
    void *memory = Allocate(&world->transientMemory, memorySize);
    SCOPE_ALLOC_FREE(&world->transientMemory, memory);

    v2i startTile = v2i((i32)floor((r32)start.x / TILE_SIZE), (i32)floor((r32)start.y / TILE_SIZE));
    v2i endTile = v2i((i32)floor((r32)end.x / TILE_SIZE), (i32)floor((r32)end.y / TILE_SIZE));
    
    char *findLog = mprintf("Finding path from (%d, %d) to (%d, %d)", startTile.x, startTile.y, endTile.x, endTile.y);
    SCOPE_FREE(findLog);
    WriteConsole(world->console, findLog);
    
    // 1. Add all adjacent, passable tiles to the set.
    const v2i validDirections[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1} };
    constexpr u32 validDirCount = sizeof(validDirections) / sizeof(validDirections[0]);

    PathfinderTile *calculatedTiles = (PathfinderTile*)memory;
    u32 maxTiles = memorySize / sizeof(PathfinderTile);
    u32 tileCount = 0;

    calculatedTiles[0].position = startTile;
    calculatedTiles[0].weight = 0;
    calculatedTiles[0].prevTile = 0;
    tileCount = 1;

    bool32 pathFound = startTile == endTile;
    PathfinderTile *originTile = calculatedTiles;

    while (!pathFound) {

        // Error condition.. if we used all memory, stop looking
        if (tileCount + 4 > maxTiles) {
            char *failLog = mprintf("Could not find optimal path from (%d, %d) to (%d, %d)... giving up :(", startTile.x, startTile.y, endTile.x, endTile.y);
            SCOPE_FREE(failLog);
            WriteConsole(world->console, failLog);
            return 0;
        }

        // Add valid neighbours.

        for (u32 dirIndex = 0; dirIndex < validDirCount; ++dirIndex) {
            if (tileCount == maxTiles) {
                // Abort, all memory used
                // TODO: Use best path so far...
                InvalidCodePath();
            } 

            v2i currentPos = originTile->position + validDirections[dirIndex];

            // TODO: Is this tile even passable?
            u32 tile = GetTile(world, currentPos.x * TILE_SIZE, currentPos.y * TILE_SIZE);
            if (!IsPassable(tile)) {
                continue;
            }

            if (ContainsTile(calculatedTiles, tileCount, currentPos)) {
                // Skip this tile...
                // TODO: New weight?
                continue;
            }

            PathfinderTile *current = calculatedTiles + tileCount;
            current->position = currentPos;
            current->weight = originTile->weight + 1; 
            current->lineDist = current->weight + 0.5 * magnitude(endTile - current->position);
            current->prevTile = originTile;
            current->visited = 0;
            tileCount++;
        }

        originTile->visited = 1;

        if (!pathFound) {
            // Select most promising tile as next origin
            // TODO: Probably select first unvisited as next.
            PathfinderTile *best = originTile;
            for (u32 i = 0; i < tileCount; ++i) {
                if (best->visited && !calculatedTiles[i].visited) {
                    best = calculatedTiles + i;
                } else if (!calculatedTiles[i].visited && calculatedTiles[i].lineDist < best->lineDist) {
                    best = calculatedTiles + i;
                }
            }

#if DEBUG_PATHFINDING
            char *nextLog = mprintf("Next tile will be (%d, %d) with weight %d, dist %.02f.", best->position.x, best->position.y, best->weight, best->lineDist);
            SCOPE_FREE(nextLog);
            WriteConsole(world->console, nextLog);
#endif
            Assert(originTile != best);
            originTile = best;
        }

        // Are we there yet?
        if (originTile->position == endTile) {
            WriteConsole(world->console, "Found best path!");
            pathFound = 1;
            // SUCCESS!
        }

    }

    char *resultLog = mprintf("Found path to (%d, %d), with %d tiles examined!", endTile.x, endTile.y, tileCount);
    SCOPE_FREE(resultLog);
    WriteConsole(world->console, resultLog);

    MoveWaypoint *waypoints = ReconstructPath(world, originTile);

    return waypoints;
}

// Create world from scratch
void WorldCreate(PlatformAPI *api, World *world, MemoryPool memory, ConsoleState *console) {

    // See if there is an existing world...
    u32 gameStartCount = 0;

    if (api->FileExists("world.dat")) {
        FileHandle fh = api->FileOpen("world.dat");
        u32 readSize = api->FileRead(fh, 4, &gameStartCount);
        api->FileClose(fh);
    }

    gameStartCount++;

    api->FileMove("world.dat", "world.dat.old");
    FileHandle fh = api->FileOpen("world.dat");    
    api->FileWrite(fh, 4, &gameStartCount);
    api->FileClose(fh);

    char *openstatus = mprintf("Game starts: %d\n", gameStartCount);
    SCOPE_FREE(openstatus);
    WriteConsole(console, openstatus); 

    *world = {};

    world->transientMemory = memory;
    world->console = console;

    // Start at center
    CenterOnChunk(world, { 0, 0 });

    for (i32 i = 0; i < CHUNK_COUNT; ++i) {
        world->chunks[i].pos.x = (i % (i32)sqrt(CHUNK_COUNT))-3;
        world->chunks[i].pos.y = (i / (i32)sqrt(CHUNK_COUNT))-3;
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
        SetTile(world, 20 + i / 10, i % 10, ASSET_TEXTURE_STONE);
    }

#if 0
    v2f origins[] = { v2f(-100.0f, -100.0f), v2f(500.0f, 100.0f), v2f(534, 1234), v2f(-4562, -1455) };
    v2f targets[] = { v2f(224.0f, 224.0f), v2f(288.0f, 288.0f), v2f(224, 288), v2f(288, 224) };

    for (i32 i = 0; i < 4; ++i) {
        Entity e = {};
        e.type = EntityType_Unit;
        e.position = origins[i];
        e.moveTarget = targets[i];
        e.command = EntityCommand_Move;
        e.flags = EntityFlag_Collidable | EntityFlag_Selectable;
        AddEntity(world, e);
    }
#endif

    Entity unit1 = {};
    unit1.type = EntityType_Unit;
    unit1.flags = EntityFlag_Collidable | EntityFlag_Selectable;
    AddEntity(world, unit1);

}

void WorldUpdate(World *world, GameInput *input, r32 dt) {

    world->mousePos = input->mouse_position;

    world->dragSelect = 0;
    world->setMovePos = 0;

    if (input->mouse_buttons & MOUSE_RIGHT) {
        world->mouseRightHoldTime += dt;
        if (magnitude(input->mouse_delta) > 5.0f || world->mouseRightHoldTime > 0.5f) {
            world->camera -= input->mouse_delta;
            world->cameraMoving = 1;
        }
    } else {
        if (!world->cameraMoving && world->mouseRightHoldTime > 0.0f && world->mouseRightHoldTime < 0.2f) {
            world->setMovePos = 1;
            world->movePos = ScreenCoordsToWorldCoords(world, world->mousePos);
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
            world->dragTarget = Normalize(targetRect);
            world->dragSelect = 1;
        }
    }

    for (u32 i = 0; i < world->loadedEntitiesCount; ++i) {
        Entity *e = &world->loadedEntities[i];
        v2f prevPos = e->position;

        if (world->dragSelect) {
            Rect2Di eRect((i32)e->position.x - TILE_SIZE / 2, (i32)e->position.y - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE);
            if (IsSelectable(e) && Intersects(world->dragTarget, eRect)) {
                e->selected = 1;
            } else {
                e->selected = 0;
            }
        }

        if (world->setMovePos && e->selected) {
            e->command = EntityCommand_Move;
            e->speed = v2f();
            e->acceleration = v2f();
            if (e->firstWaypoint) {
                Free(&world->transientMemory, e->firstWaypoint);
                e->firstWaypoint = 0;
            }
            MoveWaypoint *newPath = FindPath(world, v2i((i32)floor(e->position.x), (i32)floor(e->position.y)), world->movePos);
            if (newPath) {
                e->firstWaypoint = newPath;
                e->moveWaypoints = newPath;
                e->moveTarget = v2f(newPath->position.x, newPath->position.y);
            } else {
                // TODO: Probably remove this when FindPath returns "best so far"
                e->moveTarget = v2f(world->movePos.x, world->movePos.y);
            }
        }

        // Movement
        r32 maxSpeed = 10000.0f * dt;
        v2f acceleration = e->acceleration;

        if (e->command == EntityCommand_Move) {
            v2f direction = unit(e->moveTarget - e->position);
            acceleration += direction * 10000.0f;
        }

        e->speed += acceleration * dt;
        if (magnitude(e->speed) > maxSpeed) {
            e->speed = unit(e->speed) * maxSpeed;
        }

        e->position += e->speed * dt;
        e->acceleration = e->speed * -5.0f;

        if (magnitude(e->position - e->moveTarget) < 3.0f) {
            if (e->moveWaypoints && e->moveWaypoints->next) {
                MoveWaypoint *next = e->moveWaypoints->next;
                e->moveTarget = v2f(next->position.x, next->position.y);
                e->moveWaypoints = next;
            } else {
                e->speed = v2f();
                e->command = EntityCommand_None;
                Free(&world->transientMemory, e->firstWaypoint);
                e->firstWaypoint = 0;
                e->moveWaypoints = 0;
            }
        }


        if (IsCollidable(&world->loadedEntities[i])) {

            // Check if there are other entities which are colliding
            Rect2Di a((i32)e->position.x, (i32)e->position.y, TILE_SIZE, TILE_SIZE);
            v2f a_c = e->position + (r32)TILE_SIZE / 2.0f;
            for (u32 j = 0; j < world->loadedEntitiesCount; ++j) {
                if (j == i || !IsCollidable(&world->loadedEntities[j])) {
                    continue;
                }
                Rect2Di b((i32)world->loadedEntities[j].position.x, (i32)world->loadedEntities[j].position.y, TILE_SIZE, TILE_SIZE);
                v2f b_c = world->loadedEntities[j].position + (r32)TILE_SIZE / 2.0f;
                if (Intersects(a, b)) {
                    v2f distance = a_c - b_c;
                    r32 dist = magnitude(distance);
                    if (dist < 0.0001f) {
                        // Avoid division by zero
                        // If the entities are at the same _exact_ location, randomize push force
                        distance = unit(v2f(rand(), rand()));
                        dist = 0.0001f;
                    }
                    v2f force = (unit(distance) / dist) * 2000.0f;
                    e->acceleration += force;
                }
            }

            // Check if new position is free space (ie. cannot move into a wall)
            v2i newTilePos = v2i(e->position.x, e->position.y);
            u32 tile = GetTile(world, newTilePos.x, newTilePos.y);
            if (!IsPassable(tile)) {
                e->position = prevPos;
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

    //char *entityText = mprintf("Entities: %i", world->entityCount);
    //SCOPE_FREE(entityText);
    //DrawText(ctx, entityText, { 0, 64 }, white);
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

    for (u32 i = 0; i < world->loadedEntitiesCount; ++i) {
        Entity *e = &world->loadedEntities[i];
        v2i screenPos = WorldToScreenPosition(world, { (i32)e->position.x, (i32)e->position.y });
        Rect2Di target(screenPos.x - TILE_SIZE / 2, screenPos.y - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE);
        DrawImage(ctx, target, ASSET_TEXTURE_ENTITY);

        if (e->selected) {
            DrawRect(ctx, target, { 0.0f, 1.0f, 0.0f });
        }

        // Draw the path
        if (e->moveWaypoints) {
            MoveWaypoint *current = e->moveWaypoints;
            while (current) {
                v2i screenPos = WorldToScreenPosition(world, current->position);
                Rect2Di wpRect(screenPos.x - 10, screenPos.y - 10, 20, 20);
                DrawSolidRect(ctx, wpRect, Color(0.2f, 0.43f, 1.0f));
                current = current->next;
            }
        }
    }

    if (world->mouseDrag) {
        Rect2Di dragArea = { world->mouseDragOrigin.x, world->mouseDragOrigin.y, 
            world->mousePos.x - world->mouseDragOrigin.x, world->mousePos.y - world->mouseDragOrigin.y };
        DrawRect(ctx, dragArea, white);
    }

    // Highlight tile under mouse
    v2i tileCoords = GetTileFromScreenPosition(world, world->mousePos);
    v2i currentTilePos = WorldToScreenPosition(world, tileCoords * TILE_SIZE);
    Rect2Di currentTileRect(currentTilePos.x, currentTilePos.y, TILE_SIZE, TILE_SIZE);
    DrawRect(ctx, currentTileRect, white);


    DrawSolidRect(ctx, Rect2Di((i32)screenSize.x / 2, (i32)screenSize.y / 2, 1, 16), { 1.0f, 0.0f, 0.0f });
    DrawSolidRect(ctx, Rect2Di((i32)screenSize.x / 2, (i32)screenSize.y / 2, 16, 1), { 0.0f, 0.0f, 1.0f });

    DrawDiagnostics(world, ctx);
}
