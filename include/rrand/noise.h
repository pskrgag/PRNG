#ifndef RRANDOM_NOISE_H
#define RRANDOM_NOISE_H

#include <stdint.h>
#include <rrand/random.h>

void noise(struct random_state *state, void *buf, size_t size);

#endif
