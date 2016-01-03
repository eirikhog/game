#include "Assets.h"
#include "Memory.h"

// TODO: Remove, using for malloc debug
#include <stdlib.h>
#include <string.h>

struct loaded_file {
    uint32 size;
    void *data;
};

static asset_image load_bmp(void *data);

game_assets assets_initialize(platform_api *api, memory_segment memory) {
    game_assets assets = {};
    assets.api = api;
    assets.memory = memory;

    return assets;
}

char *get_shader(game_assets *assets, asset_id id, uint32 *size) {
    char *shader;
    *size = 0;
    switch (id) {
        case ASSET_SHADER_VERTEX:{
            shader = assets->api->ReadEntireFile("../data/shaders/minimal.vert", size);
        } break;
        case ASSET_SHADER_FRAGMENT:{
            shader = assets->api->ReadEntireFile("../data/shaders/minimal.frag", size);
        } break;
        default:
            InvalidCodePath();
            break;
    }

    return shader;
}

asset_image get_image(game_assets *assets, asset_id id) {

    uint32 file_size;
    void *file_content = assets->api->ReadEntireFile("../data/images/dirt1.bmp", &file_size);

    asset_image result = load_bmp(file_content);

    Assert(file_size > 0);
    Assert(file_content);

    return result;
}


// BMP File specification: https://en.wikipedia.org/wiki/BMP_file_format
// TODO: Move BMP handling to a seperate file?

#define BMP_FILE_TYPE 0x4D42

#pragma pack(push, 1)
typedef struct {
    uint16 file_type;
    uint32 file_size;
    uint32 reserved;
    uint32 data_offset;
} bmp_header;

typedef struct {
    uint32 dbi_size;
    uint32 width;
    uint32 height;
    uint16 planes;
    uint16 bits_per_pixel;
    uint32 compression;
    uint32 size_of_bitmap;
    int32 horizontal_resolution;
    int32 vertical_resolution;
    uint32 colors_used;
    uint32 colors_important;
} dbi_header;

typedef struct {
    uint8 blue;
    uint8 green;
    uint8 red;
    uint8 reserved;
} palette_element;

#pragma pack(pop)

union bmp_pixel {
    struct {
        uint8 r;
        uint8 g;
        uint8 b;
        uint8 a;
    };
    uint32 value;
};

static asset_image load_bmp(void *data) {
    bmp_header *header = (bmp_header*)data;
    Assert(header->file_type == BMP_FILE_TYPE);
    dbi_header *dbi = (dbi_header*)((uint8*)data + sizeof(bmp_header));

    // Note: Some versions of the header might be different.
    Assert(sizeof(dbi_header) == dbi->dbi_size);

    // TODO: Support 4-bit, and 24-bit bitmaps as well. 
    Assert(dbi->bits_per_pixel == 8);

    void *pixel_data = ((uint8*)data + header->data_offset);
    palette_element *colors = (palette_element*)((uint8*)dbi + dbi->dbi_size);

    asset_image img;
    img.width = dbi->width;
    img.height = dbi->height;

    img.data = malloc(img.width * img.height * sizeof(bmp_pixel));

    // TODO: Determine if the image axis is correct.
    Assert(dbi->height > 0);

    uint32 padding = img.width % 4;
    uint8* bitmap_data = (uint8*)data + header->data_offset;
    for (uint32 i = 0; i < dbi->width * dbi->height; ++i) {
        uint32 ci = bitmap_data[i + padding*(i / dbi->width)];
        bmp_pixel p = { colors[ci].blue, colors[ci].green, colors[ci].red, 255 };
        ((uint32*)img.data)[i] = p.value;
    }

    return img;
}