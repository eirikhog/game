#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Assets.h"
#include "AssetGenerator.h"

#include <fstream>
#include <vector>

static char *asset_read_entire_file(char *filename) {
    char *buffer = NULL;

    FILE *fp = fopen(filename, "rb");
    if (fp) {

        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        buffer = (char*)malloc(size+1);
        fread(buffer, size, 1, fp);
        buffer[size] = 0;
        fclose(fp);
    }

    return buffer;
}

struct asset_generator {
    uint32 generator_id;
};

struct asset_generator_image {
    char *filepath;
    asset_tag tag;
    asset_generator_image *next;
};

enum asset_section_type {
    ASSET_SECTION_TEXTURE,
    ASSET_SECTION_SHADER
};

struct asset_generator_binary {
    char *filepath;
};

struct asset_generator_entry {
    uint32 id;
    asset_generator_entry *next;
    union {
        asset_generator_binary binary;
        asset_generator_image image;
    };
};

struct asset_generator_section {
    asset_section_type type;
    asset_generator_entry *first;
};

static asset_generator_section
create_section(asset_section_type type) {
    asset_generator_section section = {};
    section.type = type;

    return section;
}

static void
add_entry(asset_generator *generator, asset_generator_section *section, asset_generator_entry *entry) {
    if (section->first == NULL) {
        section->first = entry;
        return;
    }

    asset_generator_entry *current = section->first;
    while (current->next != NULL) {
        current = current->next;
    }
    
    current->next = entry;
}

static void
add_shader(asset_generator *generator, asset_generator_section *section, char *filename, asset_id type) {
    asset_generator_entry *entry = (asset_generator_entry *)malloc(sizeof(asset_generator_entry));
    entry->next = NULL;
    entry->binary.filepath = filename;
    add_entry(generator, section, entry);
}

static void
cleanup_section(asset_generator_section *section) {
    if (section->first == NULL) {
        return;
    }

    asset_generator_entry *current = section->first;
    while (current->next != NULL) {
        asset_generator_entry *prev = current;
        current = current->next;
        free(prev);
    }
}

static void
add_image(asset_generator *generator, asset_generator_section *section, char *file, asset_tag tag) {
    asset_generator_entry *entry = (asset_generator_entry *)malloc(sizeof(asset_generator_entry));
    entry->next = NULL;

    entry->image.filepath = file; // Don't copy, will be static defines.
    entry->image.tag = tag;

    add_entry(generator, section, entry);
}

static asset_spritemap
generate_spritemap(asset_generator *generator, asset_generator_section *section) {
    Assert(section->type == ASSET_SECTION_TEXTURE);

    asset_spritemap spritemap = {};
    spritemap.id = generator->generator_id++;

    return spritemap;
}

int main (int argc, char *argv[]) {

    asset_generator generator = {};

    // Textures
    asset_generator_section spritemap1 = create_section(ASSET_SECTION_TEXTURE);
    add_image(&generator, &spritemap1, "dirt1.bmp", ASSET_TAG_TEXTURE_DIRT);
    add_image(&generator, &spritemap1, "dirt2.bmp", ASSET_TAG_TEXTURE_DIRT);
    asset_spritemap spritemap = generate_spritemap(&generator, &spritemap1);
    // TODO: Add spritemap to asset file.

    // Shaders
    asset_generator_section shaders = create_section(ASSET_SECTION_SHADER);
    add_shader(&generator, &shaders, "minimal.vert", ASSET_SHADER_VERTEX);
    // TODO: Add shader section to asset file

    // Free allocated resources.
    cleanup_section(&spritemap1);
    cleanup_section(&shaders);
   
    return 0;
}

