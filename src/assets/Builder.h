#ifndef _BUILDER_H
#define _BUILDER_H

#include "../Common.h"
#include "../Assets.h"


typedef struct {
    AssetFileEntry entry;
    u32 size;
    u32 type;
    void *data;
} AssetGeneratorFileEntry;

#endif

