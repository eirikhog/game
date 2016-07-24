
#include <stdio.h>

#include "Common.h"
#include "Memory.h"

void testMemoryAllocator() {
    
    // Test memory
    u8 *memory = (u8*)malloc(64 * 1024);

    MemorySegment segment;
    segment.base = memory;
    segment.size = 64 * 1024;

    MemoryPool pool = InitializeAllocator(&segment);
    
    // Check that the memory is reused
    void *test = Allocate(&pool, 1024);
    Free(&pool, test);

    void *test2 = Allocate(&pool, 1024);
    Free(&pool, test);

    Assert(test == test2);

    // Check that two allocations does not overlap
    void *overlap1 = Allocate(&pool, 512);
    void *overlap2 = Allocate(&pool, 512);

    Assert(overlap1 != overlap2);
    Assert((u8*)overlap2 > (u8*)overlap1 + 512);

    Free(&pool, overlap1);
    Free(&pool, overlap2);

    void *tooBig = Allocate(&pool, 128 * 1024);
    Assert(tooBig == NULL);

    // Scattered allocations
    void *resrv[3];
    resrv[0] = Allocate(&pool, 128);
    resrv[1] = Allocate(&pool, 64);
    resrv[2] = Allocate(&pool, 128);

    Free(&pool, resrv[1]);
    void *nxt = Allocate(&pool, 64);
    Assert(nxt == resrv[1]);
    Free(&pool, resrv[0]);
    Free(&pool, resrv[1]);
    Free(&pool, resrv[2]);

    free(memory);
}

void testMemoryAllocatorFuzzy() {
    const int memSize = 64 * 1024 * 1024;
    u8 *memory = (u8*)malloc(memSize);

    MemorySegment segment;
    segment.base = memory;
    segment.size = memSize;

    MemoryPool pool = InitializeAllocator(&segment);
    void *resrv[128];
    memset(resrv, 0, 128 * sizeof(void*));

    for (int i = 0; i < 128; ++i) {
        resrv[i] = Allocate(&pool, rand() % (memSize / 128));
    }

    for (int i = 0; i < 64; ++i) {
        Free(&pool, resrv[rand() % 128]);
    }

    for (int i = 0; i < 128; ++i) {
        Free(&pool, resrv[i]);
    }

    free(memory);

}

int main (int argc, char *argv) {
    
    printf("Running tests...\n");

    printf("testMemoryAllocator...");
    testMemoryAllocator();
    printf("OK!\n");

    printf("testMemoryAllocatorFuzzy...");
    testMemoryAllocatorFuzzy();
    printf("OK!\n");

    printf("Done!\n");

    return 0;
}
