#include <rrand/random.h>
#include <rrand/noise.h>
#include <rrand/coder.h>

#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

void get_memory_noise(struct random_state *state, void *buf, size_t size);

int main(int argc, char **argv)
{
	struct random_state *state = random_state_allocate(30);
	FILE *results, *f;
	size_t num_samples;

	assert(state);

	if (argc < 3)
		die("Usage: %s <output file> <num samples>", argv[0]);

	results = fopen(argv[1], "w");
	assert(results);

	f = fopen("/tmp/raw", "w");
	assert(f);

	num_samples = atol(argv[2]);
	if (num_samples == 0)
		die("Number of samples should be sane number %zu\n", num_samples);

	uint8_t *res = calloc(num_samples * ENCODER_SCALE_COUNT, 1);
	assert(res);

	printf("Sampling %zu bytes of extracted noise...\n", num_samples);

	get_memory_noise(state, res, num_samples * ENCODER_SCALE_COUNT);
	assert(write(fileno(f), res, num_samples * ENCODER_SCALE_COUNT) == num_samples * ENCODER_SCALE_COUNT);

	uint8_t *coder = calloc(num_samples, 1);
	assert(coder);

	size_t bytes = encode(res, num_samples, coder, num_samples);
	
	assert(write(fileno(results), coder, bytes) == (ssize_t) bytes);
}
