#ifndef _FILEBMP_H
#define _FILEBMP_H

#define BMP_FILE_TYPE 0x4D42

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../Common.h"

#pragma pack(push, 1)
typedef struct {
    uint16_t file_type;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
} bmp_header;

typedef struct {
    uint32_t dbi_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t size_of_bitmap;
    int32_t horizontal_resolution;
    int32_t vertical_resolution;
    uint32_t colors_used;
    uint32_t colors_important;

} dbi_header;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
} palette_element;

#pragma pack(pop)

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} Image;

Image LoadBMP(char *filename) {
    void *data;
    FILE *fp = fopen(filename, "rb");
    Assert(fp);
    if (fp) {
        uint32_t size;
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = malloc(size);
        fread(data, size, 1, fp);
        fclose(fp);
    }

    bmp_header *header = (bmp_header*)data;
    Assert(header->file_type == BMP_FILE_TYPE);
    dbi_header *dbi = (dbi_header*)((uint8_t*)data + sizeof(bmp_header));

    // Note: Some versions of the header might be different.
    Assert(sizeof(dbi_header) == dbi->dbi_size);
   
    Image img;
    img.width = dbi->width;
    img.height = dbi->height;

    img.data = (uint8_t*)malloc(sizeof(uint8_t) * 4 * img.height*img.width);

    if (dbi->bits_per_pixel == 8) {
        void *pixel_data = ((uint8_t*)data + header->data_offset);
        palette_element *colors = (palette_element*)((uint8_t*)dbi + dbi->dbi_size);
        // TODO: Determine if the image axis.
        // Some images can have a negative height, in which case they should not be flipped.
        // Implement if needed...
        Assert(dbi->height > 0);

        uint32_t *dest = (uint32_t*)img.data;
        uint32_t padding = img.width % 4;
        uint8_t* bitmap_data = (uint8_t*)data + header->data_offset;
        for (uint32_t i = 0; i < dbi->width * dbi->height; ++i) {
            uint32_t ci = bitmap_data[i + padding*(i / dbi->width)];
            uint8_t alpha = 0xFF;
            dest[i] = alpha << 24 | colors[ci].blue << 16 | colors[ci].green << 8 | colors[ci].red;
        }
    } else {
        // We don't care about 4-bits or other formats
        Assert(dbi->bits_per_pixel == 24);

        uint32 pitch = (uint32)floor(dbi->bits_per_pixel * dbi->width / 32) * 4;
        uint32 pixelDataSize = pitch * dbi->height;
        uint8 *source = ((uint8*)data + header->data_offset);
        uint32 *dest = (uint32*)img.data;
        for (uint32 i = 0; i < dbi->width * dbi->height; ++i) {
            if (i % pitch <= 3 * dbi->width) {
                uint8 r = *source++;
                uint8 g = *source++;
                uint8 b = *source++;
                dest[i] = (0xFF << 24) | (b << 16) | (g << 8) | (r << 0);
            } else {
                // TODO: For images which are not a power of 2, we probably
                // need to advande the source to next row (ie. finish the pitch).
                InvalidCodePath();
            }
        }
    }

    return img;
}

// Very rudamentary code for dumping a bmp (for debug purposes)
void DumpBMP(uint8* data, uint32 width, uint32 height, char *filename) {

    bmp_header bmp;
    bmp.file_type = BMP_FILE_TYPE;
    bmp.data_offset = sizeof(bmp_header) + sizeof(dbi_header);
    bmp.file_size = bmp.data_offset + width*height*3;

    dbi_header dbi;
    dbi.dbi_size = sizeof(dbi_header);
    dbi.bits_per_pixel = 24;
    dbi.width = width;
    dbi.height = height;
    dbi.compression = 0;
    dbi.colors_used = 0;
    dbi.colors_important = 0;
    dbi.horizontal_resolution = 3780;
    dbi.vertical_resolution = 3780;
    dbi.planes = 1;
    dbi.size_of_bitmap = width * height * 3;

    FILE *fp = fopen(filename, "wb");
    if (fp) {

        uint32 pitch = (uint32)floor(dbi.bits_per_pixel * width / 32) * 4;
        uint32 pixelDataSize = pitch * height;
        
        uint8 *bitmap = (uint8*)malloc(pixelDataSize);
        uint32 *source = (uint32*)data;
        memset(bitmap, 0, pixelDataSize);
        for (uint32 y = 0; y < height; ++y) {
            for (uint32 x = 0; x < width; ++x) {
                uint32 color = source[x + y * width];
                uint8 r = color >> 0;
                uint8 g = color >> 8;
                uint8 b = color >> 16;
                //uint8 a = color >> 24; /* Discard alpha */
                bitmap[(x * 3) + y*pitch] = b;
                bitmap[(x * 3) + y*pitch + 1] = g;
                bitmap[(x * 3) + y*pitch + 2] = r;
            }
        }


        fwrite(&bmp, sizeof(bmp_header), 1, fp);
        fwrite(&dbi, sizeof(dbi_header), 1, fp);

        // Write bitmap
        fwrite(bitmap, pixelDataSize, 1, fp);
        free(bitmap);

        fclose(fp);
    }
}

#endif
