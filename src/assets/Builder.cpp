#define _CRT_SECURE_NO_WARNINGS

#include "Builder.h"
#include "FileBMP.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <chrono>

#include <Windows.h>

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
    u32 idCounter;
    std::vector<AssetGeneratorFileEntry> entries;
    u32 dataOffset;
} AssetFileGenerator;

typedef struct {
    u16 elementSize;
    u32 count;
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
    u32 position = ftell(fp);
    std::cout << "Writing AtlasAsset at offset " << position << std::endl;
    fwrite(&genEntry->entry, sizeof(AssetFileEntry), 1, fp);
}

void WriteAssetFile(AssetFileGenerator *generator) {
    TIMED_FUNCTION();
    generator->header = {};
    generator->header.magic = ASSETS_MAGIC;
    generator->header.assetCount = generator->entries.size();
    
    generator->dataOffset = sizeof(AssetFileHeader) + sizeof(AssetFileEntry) * generator->header.assetCount;

    FILE *fp = fopen(generator->filename, "wb");
    if (fp) {
        fwrite(&generator->header, sizeof(AssetFileHeader), 1, fp);
        for (auto it : generator->entries) {
            //it.data = (void*)generator->dataOffset;
            it.entry.offset = generator->dataOffset;
            generator->dataOffset += it.size;
            fwrite(&it.entry, sizeof(AssetFileEntry), 1, fp);
        }

        // Now write the data.
        for (auto it : generator->entries) {
            std::cout << "Writing data for asset (ID: " << it.entry.id << ", Size: " << it.size << ", Offset : " << ftell(fp) << ")" << std::endl;
            fwrite(it.data, it.size, 1, fp);
        }
        fclose(fp);
    }
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

#define TEXTURE_SIZE 32

AtlasAsset *CreateAtlas(AtlasGenerator *atlasGen) {
    TIMED_FUNCTION();
    u32 minimumDim = (u32)square_root((r32)atlasGen->count * TEXTURE_SIZE * TEXTURE_SIZE);
    
    u32 i = 0;
    u32 dim = 0;
    do {
        dim = (u32)pow(2, i++);
    } while (dim < minimumDim);

    u32 atlasSize = sizeof(AtlasAsset) + sizeof(AtlasAssetEntry) * atlasGen->count + dim*dim * 32;

    AtlasAsset *atlas = (AtlasAsset *)malloc(atlasSize);
    
    atlas->size = atlasSize;
    atlas->width = dim;
    atlas->height = dim;
    atlas->count = atlasGen->count;
    // Put the bitmap data at the end.
    atlas->data = (u8*)atlas + sizeof(AtlasAsset) + sizeof(AtlasAssetEntry) * atlasGen->count;
    atlas->entries = (AtlasAssetEntry*)((u8*)atlas + sizeof(AtlasAsset));

    for (i32 i = 0; i < (i32)atlasGen->count; ++i) {
        i32 offsetX = (i * TEXTURE_SIZE) % dim;
        i32 offsetY = TEXTURE_SIZE * ((i * TEXTURE_SIZE) / dim);

        // Add metadata to atlas
        atlas->entries[i].id = atlasGen->ids[i];
        atlas->entries[i].uvOrigin = { (r32)offsetX / (r32)dim, 1.0f - (r32)(offsetY + TEXTURE_SIZE) / (r32)dim };
        atlas->entries[i].uvEnd = { (r32)(offsetX + TEXTURE_SIZE) / (r32)dim, 1.0f - ((r32)offsetY / (r32)dim) };

        // Copy image to atlas
        u32 *dest = (u32*)atlas->data;
        u32 *src = (u32*)atlasGen->images[i].data;
        for (i32 y = 0; y < TEXTURE_SIZE; ++y) {
            for (i32 x = 0; x < TEXTURE_SIZE; ++x) {
                dest[(offsetX + x) + (offsetY + y) * dim] = src[x + y * TEXTURE_SIZE];
            }
        }
    }

    DumpBMP(atlas->data, atlas->width, atlas->height, "atlas.bmp");

    return atlas;
}

void AddAtlasToAssetFile(AssetFileGenerator *gen, AtlasAsset *atlas) {

    AssetFileEntry entry = {};
    entry.type = ASSET_ATLAS;
    entry.size = atlas->size;
    entry.id = gen->idCounter++;

    AssetGeneratorFileEntry fileEntry = {};
    fileEntry.entry = entry;
    fileEntry.data = atlas;
    fileEntry.size = atlas->size;

    gen->entries.push_back(fileEntry);
}

AtlasAsset *BuildFontSpritemap(AssetFileGenerator *gen) {

    const int BitmapSize = 256;


    HDC dc = CreateCompatibleDC(GetDC(0));
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = BitmapSize;
    bmi.bmiHeader.biHeight = BitmapSize;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *bitmapData = 0;
    HBITMAP bitmap = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bitmapData, 0, 0);

    SelectObject(dc, bitmap);
    SetBkColor(dc, RGB(0, 0, 0));

    char *fontName = "Lucida Console";
    char *fontFile = "c:/windows/fonts/lucon.ttf";
    AddFontResourceExA(fontFile, FR_PRIVATE, 0);
    HFONT fontHandle = CreateFontA(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName);

    SelectObject(dc, fontHandle);

    //// TODO: Render ALL glyphs in the font?
    //DWORD fontSetSize = GetFontUnicodeRanges(dc, NULL);
    //GLYPHSET *fontSet = (GLYPHSET*)malloc(fontSetSize);
    //GetFontUnicodeRanges(dc, fontSet);

    TEXTMETRIC metric;
    GetTextMetrics(dc, &metric);

    i32 bounds = 1;
    i32 offsetX = bounds;
    i32 offsetY = bounds;
    i32 highest = 0;

    u32 codepointCount = 0;
    wchar_t codepoints[512];

    // Add all english characters to spritemap
    for (wchar_t c = 'A'; c <= 'z'; ++c) {
        if (c == 'Z' + 1) {
            c = 'a';
        }
        codepoints[codepointCount++] = c;
    }

    for (i32 i = 0; i < 10; ++i) {
        codepoints[codepointCount++] = (wchar_t)('0' + i);
    }

    const char extra[] = { ' ', '_', '-', '+', '=', '!', '?', ';', ':', '\'', '\\', '/', '(', ')', '.', ',', '`', 
                           '<', '>', '"', '$', '#', '@', '^', '&', '*', '%' };
    for (i32 i = 0; i < sizeof(extra); ++i) {
        codepoints[codepointCount++] = extra[i];
    }

    u32 atlasSize = sizeof(AtlasAsset) + sizeof(AtlasAssetEntry) * codepointCount + BitmapSize * BitmapSize * 32;
    AtlasAsset *atlas = (AtlasAsset *)malloc(atlasSize);
    AtlasAssetEntry *entries = (AtlasAssetEntry *)((u8*)atlas + sizeof(AtlasAsset));

    atlas->size = atlasSize;
    atlas->width = BitmapSize;
    atlas->height = BitmapSize;
    atlas->count = codepointCount;
    atlas->entries = entries;
    atlas->data = (u8*)atlas + sizeof(AtlasAsset) + sizeof(AtlasAssetEntry) * atlas->count;

    // Create font spritemap
    for (u32 i = 0; i < codepointCount; ++i) {
        wchar_t c = codepoints[i];

        SIZE size;
        GetTextExtentPoint32W(dc, &c, 1, &size);
        if (offsetX + size.cx + bounds > BitmapSize) {
            offsetX = bounds;
            offsetY += highest + bounds;
            highest = 0;
        }

        SetTextColor(dc, RGB(255, 255, 255));
        TextOutW(dc, offsetX, offsetY, &c, 1);
        
        entries[i].id = (u32)c;
        entries[i].uvOrigin = { (r32)offsetX / (r32)BitmapSize, 1.0f - (r32)offsetY / (r32)BitmapSize };
        entries[i].uvEnd = { (r32)(offsetX + size.cx) / (r32)BitmapSize, 1.0f - (r32)(offsetY + size.cy) / (r32)BitmapSize };

        offsetX += size.cx + bounds;
        if (size.cy > highest) {
            highest = size.cy;
        }

    }

    DumpBMP((u8*)bitmapData, BitmapSize, BitmapSize, "fontmap.bmp");

    // Copy image data to asset
    u32 *dest = (u32*)atlas->data;
    for (i32 y = 0; y < BitmapSize; ++y) {
        for (i32 x = 0; x < BitmapSize; ++x) {
            // 0x00bbggrr
            u32 color = GetPixel(dc, x, y);
            dest[x + y * BitmapSize] = ((color & 0xFF) << 24) | color;
        }
    }

    DeleteObject(fontHandle);
    ReleaseDC(0, dc);

    return atlas;
}

void AddShader(AssetFileGenerator *gen, char *file, u32 id) {

    FILE *fp = fopen(file, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        u32 size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        u32 assetSize = sizeof(ShaderAsset) + size + 1; // +1 for zero termination.
        u8 *data = (u8*)malloc(assetSize);
        ZeroMemory(data, assetSize);

        // Data follows the asset struct.
        fread(data + sizeof(ShaderAsset), size, 1, fp);

        ShaderAsset *asset = (ShaderAsset*)data;
        asset->id = id;
        asset->size = size;
        asset->content = (char*)(data + sizeof(ShaderAsset));

        AssetFileEntry entry = {};
        entry.type = ASSET_SHADER;
        entry.size = assetSize;
        entry.id = gen->idCounter++;

        AssetGeneratorFileEntry fileEntry = {};
        fileEntry.entry = entry;
        fileEntry.data = data;
        fileEntry.size = assetSize;

        gen->entries.push_back(fileEntry);
    }
}

int main(int argc, char* argvp[]) {
    TIMED_FUNCTION();

    AssetFileGenerator gen = CreateGenerator("assets.gap");

    // Textures:
    AtlasGenerator atlasGen = CreateAtlasGenerator(&gen);
    AddImageToAtlas(&atlasGen, "../data/images/dirt1.bmp", ASSET_TEXTURE_DIRT);
    AddImageToAtlas(&atlasGen, "../data/images/stone1.bmp", ASSET_TEXTURE_STONE);
    AddImageToAtlas(&atlasGen, "../data/images/white.bmp", ASSET_TEXTURE_WHITE);
    AddImageToAtlas(&atlasGen, "../data/images/marker.bmp", ASSET_TEXTURE_MARKER);
    AddImageToAtlas(&atlasGen, "../data/images/shroud1.bmp", ASSET_TEXTURE_SHROUD);
    AddImageToAtlas(&atlasGen, "../data/images/q.bmp", ASSET_TEXTURE_Q);
    AddImageToAtlas(&atlasGen, "../data/images/grass1.bmp", ASSET_TEXTURE_GRASS);
    AddImageToAtlas(&atlasGen, "../data/images/colortest.bmp", ASSET_TEXTURE_COLORS);
    AddImageToAtlas(&atlasGen, "../data/images/water.bmp", ASSET_TEXTURE_WATER);
    AddImageToAtlas(&atlasGen, "../data/images/empty.bmp", ASSET_TEXTURE_EMPTY);
    AddImageToAtlas(&atlasGen, "../data/images/entity.bmp", ASSET_TEXTURE_ENTITY);
    AtlasAsset *atlas = CreateAtlas(&atlasGen);
    AddAtlasToAssetFile(&gen, atlas);

    // Fonts:
    AtlasAsset *fontAtlas = BuildFontSpritemap(&gen);
    AddAtlasToAssetFile(&gen, fontAtlas);

    // Shaders:
    AddShader(&gen, "../data/shaders/minimal.frag", ASSET_SHADER_FRAGMENT);
    AddShader(&gen, "../data/shaders/minimal.vert", ASSET_SHADER_VERTEX);

    WriteAssetFile(&gen);

    // TODO: Probably want to free resources in a better way.
    free(atlas);
    free(fontAtlas);

    return 0;
}
