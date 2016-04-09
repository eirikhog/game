
//WorldChunk *LoadChunk(ChunkPosition pos) {
//}

//void SaveChunk(WorldChunk *chunk) {
//    // Save the state of the chunk to persistent storage (disk)
//}
//

static WorldChunk*
GetChunk(World *world, i32 x, i32 y) {
    for (i32 i = 0; i < CHUNK_COUNT; ++i) {
        if (world->chunks[i].pos.x == x && world->chunks[i].pos.y == y) {
            return &(world->chunks[i]);
        }
    }

    return 0;
}

static WorldChunk*
GetChunk(World *world, ChunkPosition pos) {
    return GetChunk(world, pos.x, pos.y);
}

ChunkPosition GetChunkFromWorldCoords(v2i worldPos) {
    ChunkPosition result;
    result.x = worldPos.x / (CHUNK_DIM * TILE_SIZE);
    result.y = worldPos.y / (CHUNK_DIM * TILE_SIZE);
    return result;
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

void UpdateChunk(World *world, WorldChunk *chunk, r32 dt) {
    
    // Compare entities with neighbor chunks too...
    // TODO: Move entities to new chunk if it falls outside this.
#if 0
    for (u32 i = 0; i < chunk->entityCount; ++i) {
        Entity *e = &chunk->entities[i];
        v2f prevPos = e->position;

        if (world->dragSelect) {
            Rect2Di eRect((i32)e->position.x, (i32)e->position.y, TILE_SIZE, TILE_SIZE);
            if (Intersects(world->dragTarget, eRect)) {
                e->selected = 1;
            } else {
                e->selected = 0;
            }
        }

        if (world->setMovePos && e->selected) {
            e->moveTarget = { (r32)world->movePos.x - TILE_SIZE/2, (r32)world->movePos.y - TILE_SIZE/2 };
        }
        
        // TODO: This is probably too slow...
        if (magnitude(e->position - e->moveTarget) > 1.0f) {
            v2f direction = unit(e->moveTarget - e->position);
            r32 speed = 2.0f;
            e->position += direction * speed;
        } else {
            e->position = e->moveTarget;
        }

        WorldChunk *neighbours[8];
        neighbours[0] = GetChunk(world, chunk->pos.x - 1, chunk->pos.y - 1);
        neighbours[1] = GetChunk(world, chunk->pos.x, chunk->pos.y - 1);
        neighbours[2] = GetChunk(world, chunk->pos.x + 1, chunk->pos.y - 1);
        neighbours[3] = GetChunk(world, chunk->pos.x - 1, chunk->pos.y);
        neighbours[4] = GetChunk(world, chunk->pos.x + 1, chunk->pos.y);
        neighbours[5] = GetChunk(world, chunk->pos.x - 1, chunk->pos.y + 1);
        neighbours[6] = GetChunk(world, chunk->pos.x, chunk->pos.y + 1);
        neighbours[7] = GetChunk(world, chunk->pos.x + 1, chunk->pos.y + 1);

        // TODO: Collision detection, proper
        // We want to compare only entities in this and adjacent chunks.
        Rect2Di a((i32)e->position.x, (i32)e->position.y, TILE_SIZE, TILE_SIZE);
        for (u32 j = 0; j < chunk->entityCount; ++j) {
            if (j == i) {
                continue;
            }
            Rect2Di b((i32)chunk->entities[j].position.x, (i32)chunk->entities[j].position.y, TILE_SIZE, TILE_SIZE);
            if (Intersects(a, b)) {
                e->position = prevPos;
                //e->moveTarget = prevPos;
            }
        }

        // Check neightbour cunks
        for (u32 cid = 0; cid < 8; ++cid) {
            WorldChunk *nb = neighbours[cid];
            if (nb == 0) {
                continue;
            }

            // TODO: Unify collision detection
            for (u32 j = 0; j < nb->entityCount; ++j) {
                Rect2Di b((i32)chunk->entities[j].position.x, (i32)chunk->entities[j].position.y, TILE_SIZE, TILE_SIZE);
                if (Intersects(a, b)) {
                    e->position = prevPos;
                    //e->moveTarget = prevPos;
                }
            }
        }

        ChunkPosition newChunkPos = GetChunkFromWorldCoords({ (i32)e->position.x, (i32)e->position.y });
        if (newChunkPos.x != chunk->pos.x && newChunkPos.y != chunk->pos.y) {
            WorldChunk *newChunk = GetChunk(world, newChunkPos);
            newChunk->entities[newChunk->entityCount++] = *e;

            // TODO: Improve removal
            e->deleted = true;

        }
    }

    // Remove deleted items
    u32 ni = 0;
    Entity newEntityArray[256];
    for (u32 i = 0; i < chunk->entityCount; ++i) {
        if (!chunk->entities[i].deleted) {
            newEntityArray[ni++] = chunk->entities[i];
        }
        Assert(i < 256);
    }

    chunk->entityCount = ni;
    memcpy(newEntityArray, chunk->entities, sizeof(Entity)*ni);
#endif
}


