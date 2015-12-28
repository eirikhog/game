#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../Assets.h"
#include "asset_generator.h"

#include <fstream>

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

int main (int argc, char *argv[]) {

    // TODO: Implementation!
   
    return 0;
}

