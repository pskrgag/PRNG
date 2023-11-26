#include <rrand/random.h>
#include <rrand/noise.h>

#include <stdio.h>
#include <string.h>
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
	char file_name[PATH_MAX];

	assert(state);

	if (argc < 4)
		die("Usage: %s <base file name> <num samples> <num retries>", argv[0]);


	num_samples = atol(argv[2]);
	retries = atol(argv[3]);

	uint8_t *res = malloc(num_samples);

	assert(res);

	for (size_t i = 0; i < retries; ++i) {
		char tmp[100];
		printf("Sampling %zu raw bytes from memory noise source...\n", num_samples);

		snprintf(tmp, sizeof(tmp), "%zu", i);

		strcpy(file_name, argv[1]);
		strcat(file_name, tmp);

		printf("Saving to %s...\n", file_name);
		results = fopen(file_name, "w");
		assert(results);

		get_memory_noise(state, res, num_samples);
		assert(write(fileno(results), res, num_samples) == (ssize_t) num_samples);

		fclose(results);
	}
}
