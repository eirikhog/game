#include "Chunk.h"

#include "Math.h"

// Concept:
// A "chunk" is a region on the game world, which includes elements
// on the chunk. Chunks are loaded when they are required, ie. when
// the player is near it and it need to be rendered, or if pathing
// requires inspection of the chunk.

WorldChunk *LoadChunk(v2i chunkCoordinate) {
    return 0;
}

void SaveChunk(WorldChunk *chunk) {
    // Save the state of the chunk to persistent storage (disk)
}

