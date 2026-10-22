#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dependencies/minifb/include/MiniFB.h"
#include "dependencies/minifb/include/MiniFB_enums.h"
#include "dependencies/minifb/include/MiniFB_types.h"
#include "../headers/format.h"

// Read 4 next bytes and turn them into an int, Little Endian
int32_t read_next_int_little_endian(FILE *fp) {
    uint8_t bytes[4];

    if (fread(bytes, sizeof(uint8_t), 4, fp) != 4) {
        return -1;
    }

    return (int32_t)((bytes[3] << 24) |
        (bytes[2] << 16) |
        (bytes[1] << 8) |
        (bytes[0]));
}

bool metadata_from_file(FILE *file, uint32_t *out_height, uint32_t *out_width) {
    char buf[FMT_MAGIC_LEN];
    if (fread(buf, sizeof(char), FMT_MAGIC_LEN, file) != 2 || memcmp(buf, FMT_MAGIC, FMT_MAGIC_LEN) != 0) {
        puts("Magic header not found!");
        return false;
    }
    if (fgetc(file) == FMT_TAG_RESOLUTION) {
        *out_width = read_next_int_little_endian(file);
        *out_height = read_next_int_little_endian(file);
    } else {
        puts("No resolution found!");
        return false;
    }
    printf("Detected resolution!\nWidth: %d, Height: %d\n", *out_width, *out_height);
    return true;
}

void populate_buffer(FILE *file, uint32_t *buffer) {
    char c;
    int32_t pixel = 0;
    while ((c = fgetc(file)) != EOF) {
        if (c == FMT_TAG_PIXEL) {
            int r = fgetc(file);
            int g = fgetc(file);
            int b = fgetc(file);

            buffer[pixel++] = (r << 16) | (g << 8) | b;
        }
    };
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        puts("One or more arguments is required");
        return 1;
    }
    FILE* file = fopen(argv[1], "r");

    uint32_t height, width;
    if (metadata_from_file(file, &height, &width) == false) {
        return 1;
    }

    uint32_t *buffer = (uint32_t *)calloc(width * height, sizeof(uint32_t));
    populate_buffer(file, buffer);

    struct mfb_window *window = mfb_open_ex("wow i am image", width, height, MFB_WF_RESIZABLE);
    do {
        mfb_update_state state = mfb_update_ex(window, buffer, width, height);
        if (state != MFB_STATE_OK) break;
    } while (mfb_wait_sync(window));

    free(buffer);
    return 0;
}
