#ifndef _CHUNK_H
#define _CHUNK_H

#include "Common.h"

#define CHUNK_DIM 16
#define TILE_SIZE 32

typedef struct {
    i32 x;
    i32 y;
    u32 tiles[CHUNK_DIM*CHUNK_DIM];
} WorldChunk;

#endif

