#include "Assets.h"
#include "Memory.h"

GameAssets AssetsInit(PlatformAPI *api, MemorySegment memory) {
    GameAssets assets = {};
    assets.api = api;
    assets.memory = memory;

    // Load the entire asset file to memory for now.
    // TODO: Free this sometime.
    u32 size;
    u8 *data = (u8*)api->ReadEntireFile("assets.gap", &size);
    Assert(size > 0);

    AssetFileHeader *header = (AssetFileHeader*)data;
    Assert(header->magic == ASSETS_MAGIC);
    assets.assetFile = header;

    AssetFileEntry *entries = (AssetFileEntry*)(data + sizeof(AssetFileHeader));
    assets.entries = entries;

    return assets;
}

ShaderAsset *AssetGetShader(GameAssets *assets, u32 id) {
    u8 *data = (u8*)assets->assetFile;
    for (u32 i = 0; i < assets->assetFile->assetCount; ++i) {
        if (assets->entries[i].type == ASSET_SHADER) {
            Assert(assets->entries[i].type == ASSET_SHADER);
            ShaderAsset *shader = (ShaderAsset*)(data + assets->entries[i].offset);

            shader->content = (char *)((u8*)shader + sizeof(ShaderAsset));

            if (shader->id == id) {
                return shader;
            }
        }
    }

    InvalidCodePath();
    return 0;
}

AtlasAsset *AssetGetAtlas(GameAssets *assets, u32 id) {
    for (u32 i = 0; i < assets->assetFile->assetCount; ++i) {
        if (assets->entries[i].id == id) {
            AtlasAsset *atlas = (AtlasAsset*)((u8*)assets->assetFile + assets->entries[i].offset);
            atlas->entries = (AtlasAssetEntry *)((u8*)assets->assetFile + assets->entries[i].offset + sizeof(AtlasAsset));
            atlas->data = (u8*)((u8*)assets->assetFile + assets->entries[i].offset + sizeof(AtlasAsset) + sizeof(AtlasAssetEntry) * atlas->count);
            return atlas;
        }
    }

    InvalidCodePath();
    return 0;
}
