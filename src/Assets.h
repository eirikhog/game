#ifndef _ASSETS_H
#define _ASSETS_H

#include "Common.h"
#include "Math.h"
#include "Platform.h"
#include "Memory.h"

#define ASSETS_MAGIC 0x2C4D

// TODO: Probably should remove these...
#define ASSET_SHADER_VERTEX 1
#define ASSET_SHADER_FRAGMENT 2
#define ASSET_IMAGE_SPRITEMAP 0

typedef enum {
    ASSET_SHADER,
    ASSET_IMAGE,
    ASSET_SOUND,
    ASSET_ATLAS
} AssetType;

typedef enum {
    ASSET_ATLAS1,
    ASSET_TEXTURE_WHITE,
    ASSET_TEXTURE_DIRT,
    ASSET_TEXTURE_STONE,
    ASSET_TEXTURE_MARKER,
} AssetId;

typedef struct {
    platform_api *api;
    MemorySegment memory;
} GameAssets;

typedef struct {
    uint32 id;
} AssetEntry;

typedef struct {
    uint32 width;
    uint32 height;
    void *data;
} ImageAsset;

typedef struct {
    uint32 id;
    uint32 size;
    char *content;
} ShaderAsset;

typedef struct {
    v2f uvOrigin;
    v2f uvEnd;
    AssetId id;
} AtlasAssetEntry;

typedef struct {
    uint32 id;
    uint32 width;
    uint32 height;
    uint32 count;
    uint8 *data;
    AtlasAssetEntry entries[64]; // TODO: Make dynamic
} AtlasAsset;

typedef struct {
    uint32 id;
    AssetType type;
    uint32 offset;
    union {
        ImageAsset image;
        ShaderAsset shader;
        AtlasAsset atlas;
    };
    uint32_t size;
} AssetFileEntry;

typedef struct {
    uint16 magic;
    uint32 assetCount;
    uint32 assetOffset;
} AssetFileHeader;

GameAssets assets_init(platform_api *api, MemorySegment memory);
ImageAsset asset_get_image(GameAssets *assets, uint32 id);
ShaderAsset asset_get_shader(GameAssets *assets, uint32 id);
AtlasAsset asset_get_atlas(GameAssets *assets, AssetId id);

#endif

