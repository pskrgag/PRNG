#ifndef RRANDOM_CODER_H
#define RRANDOM_CODER_H

#include <stddef.h>

size_t encoder_redundancy(void);
size_t encode(const void *in, size_t size, void *out, size_t max_size);

#endif
