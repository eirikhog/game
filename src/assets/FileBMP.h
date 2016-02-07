#ifndef _FILEBMP_H
#define _FILEBMP_H

#define BMP_FILE_TYPE 0x4D42

#include <stdlib.h>
#include <stdio.h>

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
    void *data;
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
   
    // TODO: Support 4-bit, and 24-bit bitmaps as well. 
    Assert(dbi->bits_per_pixel == 8);

    void *pixel_data = ((uint8_t*)data + header->data_offset);
    palette_element *colors = (palette_element*)((uint8_t*)dbi + dbi->dbi_size);

    Image img;
    img.width = dbi->width;
    img.height = dbi->height;

    img.data = malloc(sizeof(uint8_t)*4*img.height*img.width);

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

    return img;
}

#endif
