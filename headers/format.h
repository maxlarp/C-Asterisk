#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

#define FMT_MAGIC_LEN 2
static const char FMT_MAGIC[] = "C*";

enum {
    FMT_TAG_UNKNOWN    = 0,
    FMT_TAG_RESOLUTION = 1,
    FMT_TAG_PIXEL      = 2,
    FMT_TAG_RLE        = 3,
    FMT_TAG_ADVRLE     = 4,
    FMT_TAG_ADVANCE    = 5
};

#endif
