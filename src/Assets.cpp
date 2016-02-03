#include "Assets.h"
#include "Memory.h"

// TODO: Remove
#include <stdlib.h>
#include <string.h>

static ImageAsset load_bmp(void *data);

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

// TODO: Keep images within Asset's memory.
ImageAsset asset_get_image(GameAssets *assets, uint32 id) {
    uint32 file_size;
    void *file_content = assets->api->ReadEntireFile("../data/images/spritemap.bmp", &file_size);

    ImageAsset result = load_bmp(file_content);

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

static ImageAsset load_bmp(void *data) {
    bmp_header *header = (bmp_header*)data;
    Assert(header->file_type == BMP_FILE_TYPE);
    dbi_header *dbi = (dbi_header*)((uint8*)data + sizeof(bmp_header));

    // Note: Some versions of the header might be different.
    Assert(sizeof(dbi_header) == dbi->dbi_size);

    // TODO: Support 4-bit, and 24-bit bitmaps as well. 
    Assert(dbi->bits_per_pixel == 8);

    void *pixel_data = ((uint8*)data + header->data_offset);
    palette_element *colors = (palette_element*)((uint8*)dbi + dbi->dbi_size);

    ImageAsset img;
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

    // Flip the image
    for (uint32_t j = 0; j < img.height / 2; ++j) {
        for (uint32_t i = 0; i < img.width; ++i) {
            uint32 tmp = ((uint32*)img.data)[j*img.width + i];
            ((uint32*)img.data)[j*img.width + i] = ((uint32*)img.data)[(img.height - 1 - j)*img.width + i];
            ((uint32*)img.data)[(img.height - 1 - j)*img.width + i] = tmp;
        }
    }

    return img;
}
