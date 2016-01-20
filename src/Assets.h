#pragma once

#include "Platform.h"
#include "Memory.h"

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

    ASSET_IMAGE_SPRITEMAP,

    ASSET_COUNT
};

enum asset_tag {
    ASSET_TAG_TEXTURE_DIRT,
    ASSET_TAG_COUNT
};

struct asset_file_entry {
    asset_id id;
    asset_tag tag;
    uint32 size;
    uint32 offset;
};

typedef struct {
    uint32 magic;
    uint32 version;
    uint32 size;
    asset_type types[ASSET_TYPE_COUNT];
    uint32 assets_count;
    asset_file_entry* assets;
} asset_file_header;

typedef struct {
    asset_file_header header;
} asset_file;

typedef struct {
    uint32 size;
    char *content;
} asset_shader;

typedef struct {
    uint32 id;
    asset_type type;
    uint32 data_offset;
    union {
        asset_shader shader;
    };
} game_asset;

typedef struct {
    uint32 assets_count;
    game_asset *assets;
    platform_api *api;
    memory_segment memory;
} game_assets;

struct asset_image {
    uint32 width;
    uint32 height;
    void *data;
};

struct asset_spritemap {
    uint32 id;
};

game_assets assets_initialize(platform_api *api, memory_segment memory);
char *get_shader(game_assets *assets, asset_id id, uint32 *size);
asset_image get_image(game_assets *assets, asset_id id);
game_asset get_asset(game_assets *assets, asset_id *id);


