
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

}

int main (int argc, char *argv) {
    
    printf("Running tests...\n");

    printf("testMemoryAllocator...");
    testMemoryAllocator();
    printf("OK!\n");

    printf("Done!\n");

    return 0;
}
