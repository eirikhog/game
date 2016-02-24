#include "Assets.h"
#include "Memory.h"

// TODO: Remove
#include <stdlib.h>
#include <string.h>

struct loaded_file {
    uint32 size;
    void *data;
};

GameAssets AssetsInit(platform_api *api, MemorySegment memory) {
    GameAssets assets = {};
    assets.api = api;
    assets.memory = memory;

    return assets;
}

ShaderAsset AssetGetShader(GameAssets *assets, uint32 id) {
    char *shader;
    uint32 size = 0;
    switch (id) {
        case ASSET_SHADER_VERTEX:{
            shader = assets->api->ReadEntireFile("../data/shaders/minimal.vert", &size);
        } break;
        case ASSET_SHADER_FRAGMENT:{
            shader = assets->api->ReadEntireFile("../data/shaders/minimal.frag", &size);
        } break;
        default:
            InvalidCodePath();
            break;
    }

    ShaderAsset asset;
    asset.id = id;
    asset.content = shader;
    asset.size = size;
    return asset;
}

AtlasAsset *AssetGetAtlas(GameAssets *assets, uint32 id) {
    uint32 size;
    uint8 *data = (uint8*)assets->api->ReadEntireFile("assets.gap", &size);
    Assert(size > sizeof(AssetFileHeader));

    AssetFileHeader *header = (AssetFileHeader*)data;
    Assert(header->magic == ASSETS_MAGIC);

    AssetFileEntry *entries = (AssetFileEntry*)(data + sizeof(AssetFileHeader));
    for (uint32 i = 0; i < header->assetCount; ++i) {
        if (entries[i].id == id) {
            AtlasAsset *atlas = (AtlasAsset*)(data + entries[i].offset);
            atlas->entries = (AtlasAssetEntry *)(data + entries[i].offset + sizeof(AtlasAsset));
            atlas->data = (uint8*)(data + entries[i].offset + sizeof(AtlasAsset) + sizeof(AtlasAssetEntry) * atlas->count);
            return atlas;
        }
    }

    InvalidCodePath();
    return 0;
}
