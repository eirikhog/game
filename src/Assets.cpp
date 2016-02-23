#include "Assets.h"
#include "Memory.h"

// TODO: Remove
#include <stdlib.h>
#include <string.h>

struct loaded_file {
    uint32 size;
    void *data;
};

GameAssets assets_init(platform_api *api, MemorySegment memory) {
    GameAssets assets = {};
    assets.api = api;
    assets.memory = memory;

    return assets;
}

ShaderAsset asset_get_shader(GameAssets *assets, uint32 id) {
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

AtlasAsset asset_get_atlas(GameAssets *assets, AssetId id) {
    uint32 size;
    void *data = assets->api->ReadEntireFile("assets.gap", &size);
    Assert(size > sizeof(AssetFileHeader));

    AssetFileHeader *header = (AssetFileHeader*)data;
    Assert(header->magic == ASSETS_MAGIC);

    for (uint32 i = 0; i < header->assetCount; ++i) {
        AssetFileEntry *asset = (AssetFileEntry*)((uint8*)data + sizeof(AssetFileHeader) + sizeof(AssetFileEntry) * i);
        if (asset->id == id) {
            AtlasAsset result = asset->atlas;
            result.data = (uint8*)data + asset->offset;
            return result;
        }
    }

    // TODO: Invalid code path
    Assert(0);
    return {};
}
