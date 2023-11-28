#include <rrand/random.h>
#include <rrand/noise.h>

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"

typedef void (*test_func)(struct random_state *, void *, size_t);
void get_pipeline_noise(struct random_state *state, void *buf, size_t size);
void get_memory_noise(struct random_state *state, void *buf, size_t size);

int main(int argc, char **argv)
{
	struct random_state *state = random_state_allocate(30);
	FILE *results;
	size_t num_samples;
	size_t retries;
	char file_name[PATH_MAX];
	test_func func;

	assert(state);

	if (argc < 5)
		die("Usage: %s <base file name> <num samples> <num retries> <noise-source>", argv[0]);

	num_samples = atol(argv[2]);
	retries = atol(argv[3]);

	uint8_t *res = malloc(num_samples);
	assert(res);

	if (strcmp(argv[4], "memory") == 0)
		func = get_memory_noise;
	else if (strcmp(argv[4], "pipeline") == 0)
		func = get_pipeline_noise;
	else
		die("Unsupported noise source");

	for (size_t i = 0; i < retries; ++i) {
		char tmp[100];
		printf("Sampling %zu raw bytes from memory noise source...\n", num_samples);

		snprintf(tmp, sizeof(tmp), "%zu", i);

		strcpy(file_name, argv[1]);
		strcat(file_name, tmp);

		printf("Saving to %s...\n", file_name);
		results = fopen(file_name, "w");
		assert(results);

		func(state, res, num_samples);
		assert(write(fileno(results), res, num_samples) == (ssize_t) num_samples);

		fclose(results);
	}
}
