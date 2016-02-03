#ifndef _ASSETS_H
#define _ASSETS_H

#include "Common.h"
#include "Math.h"
#include "Platform.h"
#include "Memory.h"

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
    ASSET_TEXTURE_WHITE,
    ASSET_TEXTURE_DIRT,
    ASSET_TEXTURE_STONE,
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
    v2 uvOrigin;
    v2 uvEnd;
    AssetId id;
} AtlasAssetEntry;

typedef struct {
    uint32 id;
    uint32 width;
    uint32 height;
    void *data;
    AtlasAssetEntry entries[64]; // TODO: Make dynamic
} AtlasAsset;

GameAssets assets_init(platform_api *api, MemorySegment memory);
ImageAsset asset_get_image(GameAssets *assets, uint32 id);
ShaderAsset asset_get_shader(GameAssets *assets, uint32 id);

#endif

