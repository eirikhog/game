#define _CRT_SECURE_NO_WARNINGS

#include "Builder.h"
#include "FileBMP.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

typedef struct {
    AssetFileHeader header;
    char *filename;
    uint32 idCounter;
    std::vector<AssetGeneratorFileEntry> entries;
    uint32 dataOffset;
} AssetFileGenerator;

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

void WriteAssetFile(AssetFileGenerator *generator) {
    generator->header = {};
    generator->header.magic = ASSETS_MAGIC;
    generator->header.assetCount = generator->entries.size();
    
    generator->dataOffset = sizeof(AssetFileGenerator) + sizeof(AssetFileEntry) * generator->header.assetCount;

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
            }
        }
        // Now write the data.
        for (auto it : generator->entries) {
            std::cout << "Writing data for asset (ID: " << it.entry.id << ", Size: " << it.size << ")" << std::endl;
            fwrite(it.data, it.size, 1, fp);
        }
        fclose(fp);
    }
}

void AddImage(AssetFileGenerator *gen, char *filename) {
    // Load image from file
    // Create Imageasset struct with width, height, etc.
    // Keep data in memory until we read.

    LoadedImage img = LoadBMP(filename);

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

int main(int argc, char* argvp[]) {

    AssetFileGenerator gen = CreateGenerator("assets.gap");
    AddImage(&gen, "../data/images/spritemap.bmp");
    WriteAssetFile(&gen);

    return 0;
}
