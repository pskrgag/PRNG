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
	FILE *results, *results_raw = NULL, *raw;
	size_t num_samples, i = 0;

	assert(state);

	if (argc < 3)
		die("Usage: %s <output file> <num samples> (<output for raw data>)", argv[0]);

	results = fopen(argv[1], "w");
	assert(results);

	raw = fopen("/tmp/raw", "w");
	assert(raw);

	if (argc > 3) {
		results_raw = fopen(argv[3], "w");
		assert(results_raw);
	}

	num_samples = atol(argv[2]);
	uint8_t *res = calloc(num_samples, 1);

	assert(res);

	printf("Sampling %zu raw bytes from memory noise source...\n", num_samples);

	if (results_raw) {
		memory_noise_extracted(state, res, num_samples);
		assert(write(fileno(results_raw), res, num_samples) == num_samples);
	}

	get_memory_noise(state, res, num_samples);
	assert(write(fileno(raw), res, num_samples) == num_samples);

	/* if (results_raw) */
	/* 	assert(write(fileno(results_raw), res, num_samples) == num_samples); */

	uint8_t *coder = calloc(num_samples * 2, 1);
	size_t bytes = encode(res, num_samples, coder, num_samples * 2);
	
	assert(write(fileno(results), coder, bytes) == bytes);

}
