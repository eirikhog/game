#ifndef _CHUNK_H
#define _CHUNK_H

#define CHUNK_DIM 16
#define TILE_SIZE 32
#define CHUNK_COUNT 64 
#define ENTITIES_MAX 128

struct ChunkPosition {
    i32 x;
    i32 y;
};

struct WorldChunk {
    ChunkPosition pos;
    u32 tiles[CHUNK_DIM*CHUNK_DIM];
    Entity entities[ENTITIES_MAX];
};

#endif

