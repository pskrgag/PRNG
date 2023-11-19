#include <rrand/random.h>
#include <rrand/noise.h>

#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

int main(int argc, char **argv)
{
	struct random_state *state = random_state_allocate(30);
	FILE *results;
	size_t num_samples;
	size_t retries;

	assert(state);

	if (argc < 4)
		die("Usage: %s <num samples> <num retries>", argv[0]);

	results = fopen(argv[1], "w");
	assert(results);

	num_samples = atol(argv[2]);
	retries = atol(argv[3]);

	uint8_t *res = malloc(num_samples);

	assert(res);

	for (size_t i = 0; i < retries; ++i) {
		printf("Sampling %zu raw bytes from memory noise source...\n", num_samples);

		get_memory_noise(state, res, num_samples);
		assert(write(fileno(results), res, num_samples) == num_samples);
	}
}
