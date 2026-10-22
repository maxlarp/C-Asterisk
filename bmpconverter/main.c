#include <stdint.h>
#include <stdio.h>
#include "../headers/format.h"
#include "readbmp.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        puts("Expected two arguments");
        return 1;
    }

    int32_t width = 0;
    int32_t height = 0;

    uint8_t* rgb_array = bmp_to_array(argv[1], &width, &height);

    printf("Detected resolution~!\n%dx%d\n", width, height);
    FILE* file = fopen(argv[2], "wb");

    if (fwrite(FMT_MAGIC, sizeof(char), FMT_MAGIC_LEN, file) < FMT_MAGIC_LEN) goto error_cleanup;
    if (fwrite((char[]){ FMT_TAG_RESOLUTION }, sizeof(char), 1, file) < 1) goto error_cleanup;
    if (fwrite(&width, sizeof(int32_t), 1, file) < 1) goto error_cleanup;
    if (fwrite(&height, sizeof(int32_t), 1, file) < 1) goto error_cleanup;

    for (int i = 0; i < width * height * 3; i += 3) {
        char chunk[4] = { FMT_TAG_PIXEL, rgb_array[i], rgb_array[i + 1], rgb_array[i + 2]};
        if (fwrite(chunk, sizeof(char), 4, file) < 4) goto error_cleanup;
    }

    fclose(file);
    return 0;

error_cleanup:
    perror("Failed writing");
    fclose(file);
    return 1;
}
