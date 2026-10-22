#ifndef READBMP_H
#define READBMP_H

#include <stdint.h>

uint8_t* bmp_to_array(const char *filename, int32_t *outWidth, int32_t *outHeight);

#endif
