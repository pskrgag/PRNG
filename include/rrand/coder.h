#ifndef RRANDOM_CODER_H
#define RRANDOM_CODER_H

#include <stddef.h>

#define ENCODER_SCALE_COUNT	1.2f

size_t encode(const void *in, size_t size, void *out, size_t max_size);

#endif
