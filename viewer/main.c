#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../headers/tcolors.h"
#include "dependencies/minifb/include/MiniFB.h"
#include "dependencies/minifb/include/MiniFB_enums.h"
#include "dependencies/minifb/include/MiniFB_types.h"
#include "../headers/format.h"

// Read 4 next bytes and turn them into an int, Little Endian
int32_t read_next_int_little_endian(FILE *file) {
    uint8_t bytes[4];

    if (fread(bytes, sizeof(uint8_t), 4, file) != 4) {
        return -1;
    }

    return (int32_t)((bytes[0] << 24) |
        (bytes[1] << 16) |
        (bytes[2] << 8) |
        (bytes[3]));
}

static inline void read_rgb(const uint8_t **p, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = *(*p)++;
    *g = *(*p)++;
    *b = *(*p)++;
}

static inline void read_variable_length_quantity_int(const uint8_t **p, uint32_t *out_number) {
    uint8_t shift = 0;
    uint8_t byte;

    do {
        // TODO: remove this debug
        byte = *(*p)++;

        *out_number |= ((uint32_t)(byte & 0x7F) << shift);

        shift += 7;
    } while (byte & 0x80);
}

bool metadata_from_file(FILE *file, uint32_t *out_height, uint32_t *out_width) {
    char buf[FMT_MAGIC_LEN];
    if (fread(buf, sizeof(char), FMT_MAGIC_LEN, file) != 2 || memcmp(buf, FMT_MAGIC, FMT_MAGIC_LEN) != 0) {
        puts(RED "Magic header not found!" RESET);
        return false;
    }
    if (fgetc(file) == FMT_TAG_RESOLUTION) {
        *out_width = read_next_int_little_endian(file);
        *out_height = read_next_int_little_endian(file);
    } else {
        puts(RED "No resolution found!" RESET);
        return false;
    }
    return true;
}

void populate_buffer(FILE *file, uint32_t *buffer) {
    /* I didn't use the SEEK_SET (to put the pointer at the start of the file)
     because it would report the size including the headers, which i want to avoid. */
    long current_pos = ftell(file);
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    fseek(file, current_pos, SEEK_SET);

    if (file_size <= 0) {
        perror(RED "Error reading file" RESET);
    }

    uint8_t *raw_buffer = malloc(file_size);
    if (raw_buffer == NULL) {
        puts(RED "Error allocating memory" RESET);
    }

    size_t bytes_read = fread(raw_buffer, 1, file_size, file);

    const uint8_t *p = raw_buffer;
    uint8_t *end = raw_buffer + bytes_read;
    int32_t pixel = 0;

    while (p < end) {
        uint8_t tag = *p++;

        if (tag == FMT_TAG_PIXEL) {
            if (p + 3 <= end) {
                uint8_t r, g, b;
                read_rgb(&p, &r,&g, &b);

                buffer[pixel++] = (r << 16) | (g << 8) | b;
            }
        }
        else if (tag == FMT_TAG_RLE) {
            uint8_t r, g, b;
            read_rgb(&p, &r,&g, &b);

            uint32_t length = 0;
            read_variable_length_quantity_int(&p, &length);

            uint32_t counter = 0;
            while (counter != length) {
                buffer[pixel+counter] = (r << 16) | (g << 8) | b;
                counter++;
            }
        }
        else if (tag == FMT_TAG_ADVRLE) {
            uint8_t r, g, b;
            read_rgb(&p, &r, &g, &b);

            uint32_t length = 0;
            read_variable_length_quantity_int(&p, &length);

            uint32_t counter = 0;
            while (counter != length) {
                buffer[pixel++] = (r << 16) | (g << 8) | b;
                counter++;
            }
        }
        else if (tag == FMT_TAG_ADVANCE) {
            uint32_t amount = 0;
            read_variable_length_quantity_int(&p, &amount);
            for (; amount > 0; amount--) {
                pixel++;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        puts(RED "Expected one or more arguments!" RESET);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");

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
