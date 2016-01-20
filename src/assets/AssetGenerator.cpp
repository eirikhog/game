#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Assets.h"
#include "AssetGenerator.h"

#include <fstream>
#include <vector>

static char *asset_read_entire_file(char *filename, uint32 *file_size) {
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

        *file_size = size + 1;
    }

    return buffer;
}

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
    uint32 tag;
    uint32 size;
    void *data;
    asset_generator_entry *next;
    union {
        asset_generator_binary binary;
        asset_generator_image image;
    };
};

struct asset_generator_section {
    asset_section_type type;
    asset_generator_entry *entries;
    asset_generator_section *next;
};

struct asset_generator {
    uint32 generator_id;
    asset_generator_section *sections;
};

static asset_generator_section *
create_section(asset_generator *generator, asset_section_type type) {
    asset_generator_section *section = (asset_generator_section *)malloc(sizeof(asset_generator_section));
    section->type = type;
    section->next = NULL;
    section->entries = NULL;

    if (generator->sections == NULL) {
        generator->sections = section;
    }
    else {
        asset_generator_section *current = generator->sections;
        while (current->next) {
            current = current->next;
        }
        current->next = section;
    }

    return section;
}

static void
add_entry(asset_generator *generator, asset_generator_section *section, asset_generator_entry *entry) {
    if (section->entries == NULL) {
        section->entries = entry;
        return;
    }

    asset_generator_entry *current = section->entries;
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

    uint32 size;
    char filepath_buffer[256];
    sprintf(filepath_buffer, "../data/shaders/%s", filename);
    entry->data = asset_read_entire_file(filepath_buffer, &size);
    entry->size = size;
}

static void
cleanup_section(asset_generator_section *section) {
    if (section->entries == NULL) {
        return;
    }

    asset_generator_entry *current = section->entries;
    while (current->next != NULL) {
        asset_generator_entry *prev = current;
        current = current->next;
        free(prev);
    }
}

static void
add_image(asset_generator *generator, asset_generator_section *section, char *filename, asset_tag tag) {
    asset_generator_entry *entry = (asset_generator_entry *)malloc(sizeof(asset_generator_entry));
    entry->next = NULL;

    entry->image.filepath = filename; // Don't copy, will be static defines.
    entry->image.tag = tag;

    add_entry(generator, section, entry);

    uint32 size;
    char filepath_buffer[256];
    sprintf(filepath_buffer, "../data/images/%s", filename);
    entry->data = asset_read_entire_file(filepath_buffer, &size);
    entry->size = size;
}

static asset_spritemap
generate_spritemap(asset_generator *generator, asset_generator_section *section) {
    Assert(section->type == ASSET_SECTION_TEXTURE);

    asset_spritemap spritemap = {};
    spritemap.id = generator->generator_id++;

    return spritemap;
}

void write_texture_section(FILE *fp, asset_generator_section *section) {
    fprintf(stderr, "Not implemented! " __FUNCTION__ "\n");
}

void write_shader_section(FILE *fp, asset_generator_section *section) {
    fprintf(stderr, "Not implemented! " __FUNCTION__ "\n");
}

void write_asset_file(asset_generator *generator, char *output) {

    FILE *fp = fopen(output, "w");
    if (!fp) {
        fprintf(stderr, "Could not open asset file for writing.\n");
        return;
    }

    // Write the header
    asset_file_header header = {};
    header.magic = ASSET_FILE_MAGIC;
    header.version = ASSET_FILE_VERSION;

    // Count the number of assets
    uint32 assets_count = 0;

    asset_generator_section *section = generator->sections;
    while (section != NULL) {

        asset_generator_entry *entry = section->entries;
        while (entry != NULL) {
            assets_count++;
            entry = entry->next;
        }

        section = section->next;
    }

    printf("Number of assets: %d\n", assets_count);
    header.assets_count = assets_count;

    fwrite(&header, sizeof(asset_file_header), 1, fp);

    uint32 data_offset = sizeof(asset_file_header) * sizeof(asset_file_entry) * assets_count;

    section = generator->sections;
    while (section != NULL) {

        asset_generator_entry *entry = section->entries;
        while (entry != NULL) {

            asset_file_entry generated_entry = {};
            generated_entry.id = (asset_id)entry->id;
            generated_entry.tag = (asset_tag)entry->tag;
            generated_entry.size = entry->size;
            generated_entry.offset = data_offset;
            data_offset += entry->size;

            fwrite(&generated_entry, sizeof(asset_file_entry), 1, fp);

            entry = entry->next;
        }

        section = section->next;
    }

    // Write asset data
    section = generator->sections;
    while (section != NULL) {

        asset_generator_entry *entry = section->entries;
        while (entry != NULL) {

            // TODO: Write meta data

            fwrite(entry->data, entry->size, 1, fp);

            entry = entry->next;
        }

        section = section->next;
    }
    

    fclose(fp);
}

int main (int argc, char *argv[]) {

    asset_generator generator = {};

    // Textures
    printf("Processing textures...\n");
    asset_generator_section *spritemap1 = create_section(&generator, ASSET_SECTION_TEXTURE);
    add_image(&generator, spritemap1, "dirt1.bmp", ASSET_TAG_TEXTURE_DIRT);
    add_image(&generator, spritemap1, "dirt2.bmp", ASSET_TAG_TEXTURE_DIRT);
    asset_spritemap spritemap = generate_spritemap(&generator, spritemap1);
    // TODO: Convert image files to bmp RBGA format.
    // TODO: Assemble spritemap.
    // TODO: Add spritemap to asset file.

    // Shaders
    printf("Processing shaders...\n");
    asset_generator_section *shaders = create_section(&generator, ASSET_SECTION_SHADER);
    add_shader(&generator, shaders, "minimal.vert", ASSET_SHADER_VERTEX);
    add_shader(&generator, shaders, "minimal.frag", ASSET_SHADER_FRAGMENT);

    printf("Writing asset file...\n");
    write_asset_file(&generator, "test_asset_file.pga");

    printf("Cleanup...\n");
    // Free allocated resources.
    cleanup_section(spritemap1);
    cleanup_section(shaders);
    // TODO: Cleanup generator.

    printf("Done!\n");
   
    return 0;
}

