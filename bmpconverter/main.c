#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "../headers/tcolors.h"
#include "../headers/format.h"
#include "readbmp.h"

typedef struct {
    uint32_t color_key;
    int occurences;
    int *indices;
} ColorBucket;

typedef struct {
    uint32_t color_key;
    uint32_t count;
} HeapNode;

void swap_nodes(HeapNode *a, HeapNode *b) {
    HeapNode temp = *a;
    *a = *b;
    *b = temp;
}

void min_heapify(HeapNode heap[], int heap_size, int i) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 1;

    if (left < heap_size && heap[left].count < heap[smallest].count) smallest = left;
    if (right < heap_size && heap[right].count < heap[smallest].count) smallest = right;

    if (smallest != i) {
        swap_nodes(&heap[i], &heap[smallest]);
        min_heapify(heap, heap_size, i);
    }
}

// k is the amount of elements in the out_frequency color_array
void grab_frequent_colors(uint8_t* rgb_array, int k, uint32_t total_pixels, ColorBucket *out_frequency_color_array) {
    uint32_t *color_frequency = calloc(16777216, sizeof(ColorBucket)); // 16777216 is allocated since it is the amount of every possible RGB pixel

    for (uint32_t i = 0; i < total_pixels; i++) {
        int index = i * 3;
        uint32_t packed_rgb = ((uint32_t)rgb_array[index] << 16) |
                                      ((uint32_t)rgb_array[index + 1] << 8) |
                                      rgb_array[index + 2];
        color_frequency[packed_rgb]++;
    }

    HeapNode *min_heap = malloc(k * sizeof(HeapNode));
    int heap_size = 0;

    for (int color = 0; color < 16777216; color++) {
        uint32_t use_count = color_frequency[color];
        if (use_count < 5) continue;

        if (heap_size < k) {
            min_heap[heap_size].color_key = color;
            min_heap[heap_size].count = use_count;
            heap_size++;
            if (heap_size == k) {
                for (int i = (k / 2) - 1; i >= 0; i--) {
                    min_heapify(min_heap, heap_size, i);
                }
            }
        } else if (use_count > min_heap[0].count) {
            min_heap[0].color_key = color;
            min_heap[0].count = use_count;
            min_heapify(min_heap, k, 0);
        }
    }

    for (int i = 0; i < k; i++) {
        out_frequency_color_array[i].color_key = min_heap[i].color_key;
        out_frequency_color_array[i].occurences = 0;
        out_frequency_color_array[i].indices = malloc(min_heap[i].count * sizeof(int));
    }
    free(min_heap);

    uint32_t *color_to_bucket_map = color_frequency;
    for (int i = 0; i < 16777216; i++) {
        color_to_bucket_map[i] = -1;
    }
    for (int b_index = 0; b_index < k; b_index++) {
        color_to_bucket_map[out_frequency_color_array[b_index].color_key] = b_index;
    }

    int *write_offsets = calloc(k, sizeof(int));

    for (uint32_t i = 0; i < total_pixels; i++) {
        int index = i * 3;
        uint32_t packed_rgb = ((uint32_t)rgb_array[index] << 16) |
                                      ((uint32_t)rgb_array[index + 1] << 8) |
                                      rgb_array[index + 2];

        int b_index = color_to_bucket_map[packed_rgb];
        if (b_index != -1) {
            int pos = write_offsets[b_index];
            out_frequency_color_array[b_index].indices[pos] = i;
            write_offsets[b_index]++;
        }
    }

    free(color_frequency);
    free(write_offsets);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        puts(RED "Expected two or more arguments!" RESET);
        return 1;
    }

    int32_t width = 0;
    int32_t height = 0;

    uint8_t* rgb_array = bmp_to_array(argv[1], &width, &height);

    printf(RED "Detected resolution~!\n%dx%d\n" RESET, width, height);
    FILE* file = fopen(argv[2], "wb");

    if (fwrite(FMT_MAGIC, sizeof(char), FMT_MAGIC_LEN, file) < FMT_MAGIC_LEN) goto error_cleanup;
    if (fwrite((char[]){ FMT_TAG_RESOLUTION }, sizeof(char), 1, file) < 1) goto error_cleanup;
    if (fwrite(&width, sizeof(int32_t), 1, file) < 1) goto error_cleanup;
    if (fwrite(&height, sizeof(int32_t), 1, file) < 1) goto error_cleanup;

    uint32_t total_pixels = width * height;

    int k = 128;
    ColorBucket *top_buckets = malloc(k * sizeof(ColorBucket));
    grab_frequent_colors(rgb_array, k, total_pixels, top_buckets);

    uint32_t i = 0;
    while (i < total_pixels) {
        int j = i * 3; // j will be the index of the pixel
        uint8_t r = rgb_array[j];
        uint8_t g = rgb_array[j + 1];
        uint8_t b = rgb_array[j + 2];

        for (int i = 0; i < k; i++) {
            if (top_buckets[i].color_key == (uint32_t)(((r << 16) | (g << 8) | b))) {
                printf("%d ", top_buckets[i].indices[0]);
            }
        }

        char chunk[4] = { FMT_TAG_PIXEL, r, g, b};
        if (fwrite(chunk, sizeof(char), 4, file) < 4) goto error_cleanup;
    }

    fclose(file);
    return 0;

error_cleanup:
    perror(RED "Failed writing" RESET);
    fclose(file);
    return 1;
}
