#define _CRT_SECURE_NO_WARNINGS

#include "Builder.h"
#include "FileBMP.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <chrono>

// Rudamentary performance monitor
class Timer {
public:
    Timer(char *name) : m_name(name) { this->start = std::chrono::system_clock::now(); }
    ~Timer() {
        std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << m_name << " took " << elapsed.count() << " seconds." << std::endl;
    }
private:
    char *m_name;
    std::chrono::time_point<std::chrono::system_clock> start;
};

#define TIMED_FUNCTION_(name, line) Timer timer_##line(name)
#define TIMED_FUNCTION() TIMED_FUNCTION_(__FUNCTION__, __LINE__)

typedef struct {
    AssetFileHeader header;
    char *filename;
    uint32 idCounter;
    std::vector<AssetGeneratorFileEntry> entries;
    uint32 dataOffset;
} AssetFileGenerator;

typedef struct {
    uint16 elementSize;
    uint32 count;
    Image images[256];
    AssetId ids[256];
} AtlasGenerator;

AssetFileGenerator CreateGenerator(char *filename) {
    AssetFileGenerator generator = {};
    generator.filename = filename;
    generator.dataOffset = sizeof(AssetFileGenerator);
    return generator;
}

void WriteImageAsset(FILE *fp, AssetFileGenerator *gen, AssetGeneratorFileEntry *genEntry) {
    genEntry->entry.offset = gen->dataOffset;
    gen->dataOffset += genEntry->entry.size;

    fwrite(&genEntry->entry, sizeof(AssetFileEntry), 1, fp);

}

void WriteSoundAsset(FILE *fp, AssetFileGenerator *gen, AssetGeneratorFileEntry *genEntry) {
}

void WriteShaderAsset(FILE *fp, AssetFileGenerator *gen, AssetGeneratorFileEntry *genEntry) {
}

void WriteAtlasAsset(FILE *fp, AssetFileGenerator *gen, AssetGeneratorFileEntry *genEntry) {
    genEntry->entry.offset = gen->dataOffset;
    gen->dataOffset += genEntry->entry.size;
    uint32 position = ftell(fp);
    std::cout << "Writing AtlasAsset at offset " << position << std::endl;
    fwrite(&genEntry->entry, sizeof(AssetFileEntry), 1, fp);
}

void WriteAssetFile(AssetFileGenerator *generator) {
    TIMED_FUNCTION();
    generator->header = {};
    generator->header.magic = ASSETS_MAGIC;
    generator->header.assetCount = generator->entries.size();
    
    generator->dataOffset = sizeof(AssetFileHeader) + sizeof(AssetFileEntry) * generator->header.assetCount;

    FILE *fp = fopen(generator->filename, "w");
    if (fp) {
        fwrite(&generator->header, sizeof(AssetFileHeader), 1, fp);
        for (auto it : generator->entries) {
            AssetGeneratorFileEntry *genEntry = &it;
            switch (genEntry->entry.type) {
                case ASSET_IMAGE:
                    std::cout << "Writing image (ID: " << genEntry->entry.id << ")" << std::endl;
                    WriteImageAsset(fp, generator, genEntry);
                    break;
                case ASSET_SHADER:
                    std::cout << "Writing shader (ID: " << genEntry->entry.id << ")" << std::endl;
                    WriteShaderAsset(fp, generator, genEntry);
                    break;
                case ASSET_SOUND:
                    std::cout << "Writing sound (ID: " << genEntry->entry.id << ")" << std::endl;
                    WriteSoundAsset(fp, generator, genEntry);
                    break;
                case ASSET_ATLAS:
                    std::cout << "Writing atlas (ID: " << genEntry->entry.id << ")" << std::endl;
                    WriteAtlasAsset(fp, generator, genEntry);
                    break;
            }
        }
        // Now write the data.
        for (auto it : generator->entries) {
            std::cout << "Writing data for asset (ID: " << it.entry.id << ", Size: " << it.size << ", Offset : " << ftell(fp) << ")" << std::endl;
            fwrite(it.data, it.size, 1, fp);
        }
        fclose(fp);
    }
}

void AddImage(AssetFileGenerator *gen, char *filename) {
    // Load image from file
    // Create Imageasset struct with width, height, etc.
    // Keep data in memory until we read.

    Image img = LoadBMP(filename);

    AssetFileEntry entry = {};
    entry.id = gen->idCounter++;
    entry.type = ASSET_IMAGE;
    entry.offset = 0; // This is calculated on write.
    entry.image.width = img.width;
    entry.image.height = img.height;

    AssetGeneratorFileEntry genEntry = {};
    genEntry.entry = entry;
    genEntry.size = img.width * img.height * 4;
    genEntry.data = img.data;

    gen->entries.push_back(genEntry);
}

AtlasGenerator CreateAtlasGenerator(AssetFileGenerator *gen) {
    AtlasGenerator atlasGen = {};
    atlasGen.elementSize = 16;

    return atlasGen;
}

void AddImageToAtlas(AtlasGenerator *gen, char *filePath, AssetId id) {
    Image image = LoadBMP(filePath);

    gen->ids[gen->count] = id;
    gen->images[gen->count] = image;
    gen->count++;
}

#define TEXTURE_SIZE 16

AtlasAsset CreateAtlas(AtlasGenerator *atlasGen) {
    TIMED_FUNCTION();
    uint32 minimumDim = sqrt(atlasGen->count * TEXTURE_SIZE * TEXTURE_SIZE);
    
    uint32 i = 0;
    uint32 dim = 0;
    do {
        dim = pow(2, i++);
    } while (dim < minimumDim);

    AtlasAsset atlas = {};
    atlas.width = dim;
    atlas.height = dim;
    atlas.data = malloc(dim*dim * 32);

    for (int32 i = 0; i < atlasGen->count; ++i) {
        int32 offsetX = (i * TEXTURE_SIZE) % dim;
        int32 offsetY = TEXTURE_SIZE * ((i * TEXTURE_SIZE) / dim);

        // Add metadata to atlas
        atlas.entries[i].id = atlasGen->ids[i];
        atlas.entries[i].uvOrigin = { (real32)offsetX / (real32)dim, 1.0f - (real32)(offsetY + TEXTURE_SIZE) / (real32)dim };
        atlas.entries[i].uvEnd = { (real32)(offsetX + TEXTURE_SIZE) / (real32)dim, 1.0f - ((real32)offsetY / (real32)dim) };

        // Copy image to atlas
        uint32_t *dest = (uint32_t*)atlas.data;
        uint32_t *src = (uint32_t*)atlasGen->images[i].data;
        for (int32 y = 0; y < TEXTURE_SIZE; ++y) {
            for (int32 x = 0; x < TEXTURE_SIZE; ++x) {
                dest[(offsetX + x) + (offsetY + y) * dim] = src[x + y * TEXTURE_SIZE];
            }
        }
    }

    // Testing
    //uint32_t *dest = (uint32_t*)atlas.data;
    //for (int y = 0; y < dim; ++y) {
    //    for (int x = 0; x < dim; ++x) {
    //        if (x < 16 && y < 16) {
    //            dest[x + dim*y] = 0x11111111;
    //        } else if (x >= 16 && y < 16) {
    //            dest[x + dim*y] = 0x22222222;
    //        }
    //        else if (x < 16 && y >= 16) {
    //            dest[x + dim*y] = 0x33333333;
    //        }
    //        else {
    //            dest[x + dim*y] = 0x44444444;
    //        }
    //    }
    //}

    return atlas;
}

void AddAtlasToAssetFile(AssetFileGenerator *gen, AtlasAsset atlas) {
    AssetFileEntry entry = {};
    entry.type = ASSET_ATLAS;
    entry.atlas = atlas;
    entry.size = atlas.width * atlas.height * 32;
    entry.id = gen->idCounter++;

    AssetGeneratorFileEntry fileEntry = {};
    fileEntry.entry = entry;
    fileEntry.data = atlas.data;
    fileEntry.size = entry.size;

    gen->entries.push_back(fileEntry);
}

int main(int argc, char* argvp[]) {
    TIMED_FUNCTION();

    AssetFileGenerator gen = CreateGenerator("assets.gap");
    //AddImage(&gen, "../data/images/spritemap.bmp");

    AtlasGenerator atlasGen = CreateAtlasGenerator(&gen);
    AddImageToAtlas(&atlasGen, "../data/images/dirt1.bmp", ASSET_TEXTURE_DIRT);
    AddImageToAtlas(&atlasGen, "../data/images/stone1.bmp", ASSET_TEXTURE_STONE);
    AddImageToAtlas(&atlasGen, "../data/images/white.bmp", ASSET_TEXTURE_WHITE);
    AddImageToAtlas(&atlasGen, "../data/images/marker.bmp", ASSET_TEXTURE_MARKER);
    AtlasAsset atlas = CreateAtlas(&atlasGen);

    AddAtlasToAssetFile(&gen, atlas);

    WriteAssetFile(&gen);

    return 0;
}
