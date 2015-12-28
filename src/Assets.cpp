#include "Assets.h"
#include "Memory.h"

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


