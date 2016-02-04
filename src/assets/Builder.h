#ifndef _BUILDER_H
#define _BUILDER_H

#include "../Common.h"
#include "../Assets.h"

typedef struct {
    AssetFileEntry entry;
    uint32_t size;
    void *data;
} AssetGeneratorFileEntry;

#endif

