#ifndef _BUILDER_H
#define _BUILDER_H

#include "../Common.h"
#include "../Assets.h"


typedef struct {
    AssetFileEntry entry;
    uint32 size;
    uint32 type;
    void *data;
} AssetGeneratorFileEntry;

#endif

