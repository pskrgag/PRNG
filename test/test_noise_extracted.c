#include <rrand/random.h>
#include <rrand/noise.h>

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

void noise(struct random_state *state, void *buf, size_t size);

int main(int argc, char **argv)
{
	struct random_state *state = random_state_allocate(30);
	FILE *results;
	size_t num_samples;

	assert(state);

	if (argc < 3)
		die("Usage: %s <file name> <num samles>", argv[0]);

	num_samples = atol(argv[2]);

	uint8_t *res = malloc(num_samples);
	assert(res);

	printf("Sampling %zu of extracted noise sources...\n", num_samples);
	results = fopen(argv[1], "w");
	assert(results);

	noise(state, res, num_samples);
	assert(write(fileno(results), res, num_samples) == (ssize_t) num_samples);

	fclose(results);
	random_state_free(state);
	free(res);
}
