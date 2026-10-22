#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

#define FMT_MAGIC_LEN 2
static const char FMT_MAGIC[] = "C*";

enum {
    FMT_TAG_RESOLUTION = 21,
    FMT_TAG_PIXEL = 49
};

#endif
