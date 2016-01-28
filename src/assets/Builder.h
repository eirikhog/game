#ifndef _BUILDER_H
#define _BUILDER_H

#define ASSETS_MAGIC 0x2C4D

#include "../Common.h"
#include "../Assets.h"

typedef struct {
    uint32 id;
    AssetType type;
    uint32 offset;
    union {
        ImageAsset image;
        ShaderAsset shader;
    };
    uint32_t size;
} AssetFileEntry;

typedef struct {
    AssetFileEntry entry;
    uint32_t size;
    void *data;
} AssetGeneratorFileEntry;

typedef struct {
    uint16 magic;
    uint32 assetCount;
    uint32 assetOffset;
} AssetFileHeader;

#endif

