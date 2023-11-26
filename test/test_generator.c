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

	assert(state);

	if (argc < 3)
		die("Usage: %s <output file> <num samples>", argv[0]);

	results = fopen(argv[1], "w");
	assert(results);

	num_samples = atol(argv[2]);
	uint8_t *res = malloc(num_samples);

	assert(res);

	printf("Sampling %zu raw bytes from memory noise source...\n", num_samples);

	assert(random_get_bytes(state, res, num_samples) == 0);
	assert(write(fileno(results), res, num_samples) == (ssize_t) num_samples);

	random_state_free(state);
	free(res);
}
