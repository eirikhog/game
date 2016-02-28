#include "Assets.h"
#include "Memory.h"

GameAssets AssetsInit(platform_api *api, MemorySegment memory) {
    GameAssets assets = {};
    assets.api = api;
    assets.memory = memory;

    // Load the entire asset file to memory for now.
    // TODO: Free this sometime.
    uint32 size;
    uint8 *data = (uint8*)api->ReadEntireFile("assets.gap", &size);
    Assert(size > 0);

    AssetFileHeader *header = (AssetFileHeader*)data;
    Assert(header->magic == ASSETS_MAGIC);
    assets.assetFile = header;

    AssetFileEntry *entries = (AssetFileEntry*)(data + sizeof(AssetFileHeader));
    assets.entries = entries;

    return assets;
}

ShaderAsset *AssetGetShader(GameAssets *assets, uint32 id) {
    uint8 *data = (uint8*)assets->assetFile;
    for (uint32 i = 0; i < assets->assetFile->assetCount; ++i) {
        if (assets->entries[i].type == ASSET_SHADER) {
            Assert(assets->entries[i].type == ASSET_SHADER);
            ShaderAsset *shader = (ShaderAsset*)(data + assets->entries[i].offset);

            shader->content = (char *)((uint8*)shader + sizeof(ShaderAsset));

            if (shader->id == id) {
                return shader;
            }
        }
    }

    InvalidCodePath();
    return 0;
}

AtlasAsset *AssetGetAtlas(GameAssets *assets, uint32 id) {
    for (uint32 i = 0; i < assets->assetFile->assetCount; ++i) {
        if (assets->entries[i].id == id) {
            AtlasAsset *atlas = (AtlasAsset*)((uint8*)assets->assetFile + assets->entries[i].offset);
            atlas->entries = (AtlasAssetEntry *)((uint8*)assets->assetFile + assets->entries[i].offset + sizeof(AtlasAsset));
            atlas->data = (uint8*)((uint8*)assets->assetFile + assets->entries[i].offset + sizeof(AtlasAsset) + sizeof(AtlasAssetEntry) * atlas->count);
            return atlas;
        }
    }

    InvalidCodePath();
    return 0;
}
