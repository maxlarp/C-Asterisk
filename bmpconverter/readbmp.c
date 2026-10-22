#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFileHeader;

typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

uint8_t* bmp_to_array(const char *filename, int32_t *out_width, int32_t *out_height) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fread(&fileHeader, sizeof(BMPFileHeader), 1, file);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, file);

    if (fileHeader.bfType != 0x4D42 || infoHeader.biBitCount != 24 || infoHeader.biCompression != 0) {
        puts("Unsupported file format. Must be uncompressed 24-bit BMP.");
        fclose(file);
        return NULL;
    }

    int32_t width = infoHeader.biWidth;
    int32_t height = abs(infoHeader.biHeight);

    *out_width = width;
    *out_height = height;

    // Calculate row padding (BMP rows must align to 4-byte boundaries)
    int rowSize = ((width * 3 + 3) / 4) * 4;
    int padding = rowSize - (width * 3);

    // Allocate memory for the flat array: 3 bytes (R, G, B) per pixel
    uint8_t *rgb_array = (uint8_t *)malloc(width * height * 3);
    if (!rgb_array) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    fseek(file, fileHeader.bfOffBits, SEEK_SET);

    // Read row by row
    for (int i = 0; i < height; i++) {
        // Destination offset inside our flat RGB array for the current row
        uint8_t *row_dest = &rgb_array[i * width * 3];

        for (int j = 0; j < width; j++) {
            uint8_t bgr[3];
            fread(bgr, 1, 3, file); // BMP stores data as B, G, R

            // Swap to R, G, B layout
            row_dest[j * 3 + 0] = bgr[2]; // Red
            row_dest[j * 3 + 1] = bgr[1]; // Green
            row_dest[j * 3 + 2] = bgr[0]; // Blue
        }

        // Skip the dummy padding bytes at the end of the file row
        if (padding > 0) {
            fseek(file, padding, SEEK_CUR);
        }
    }

    fclose(file);
    return rgb_array;
}
