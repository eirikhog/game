#pragma once

#include "Platform.h"

#define ASSET_FILE_MAGIC 0x4747
#define ASSET_FILE_VERSION 1

enum asset_type {
    ASSET_TYPE_NONE = 0,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_COUNT
};

enum asset_id {
    ASSET_SHADER_VERTEX,
    ASSET_SHADER_FRAGMENT,

    ASSET_COUNT
};

typedef struct {
    uint32 magic;
    uint32 version;
    uint32 size;
    asset_type types[ASSET_TYPE_COUNT];
    uint32 shaders_offset;
} asset_file_header;

typedef struct {
    asset_file_header header;
} asset_file;

typedef struct {
    uint32 id;
    asset_type type;
    uint32 data_offset;
} game_asset;

typedef struct {
    uint32 assets_count;
    game_asset *assets;
    platform_api *api;
} game_assets;

game_assets assets_initialize(platform_api *api);
char *get_shader(game_assets *assets, asset_id id, uint32 *fileSize);


