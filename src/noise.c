#include <rrand/random.h>
#include <rrand/utils.h>
#include <rrand/private.h>
#include <rrand/coder.h>

#include <limits.h>
#include <assert.h>

#define MEM_LOOP_COUNT		256

#define READ_ONCE(x)		*((volatile typeof(x)) x)
#define WRITE_ONCE(x, val)	*((volatile typeof(x)) x) = (val)

struct lcg {
	uint64_t seed;
};

static inline int rand_lcg(struct lcg *lcg)
{
	return lcg->seed = (lcg->seed * 1103515245 + 12345);
}

static void memory_accesss(struct random_state *state)
{
	struct lcg lcg = { .seed = get_nsec() };
	const uint64_t mem_mask = (1ULL << state->mem_size) - 1;

	for (int i = 0; i < MEM_LOOP_COUNT; ++i) {
		uint8_t *mem = state->mem + (rand_lcg(&lcg) & mem_mask);

		/* Prevent compiler from optimizing and flit the bit. */
		WRITE_ONCE(mem, READ_ONCE(mem) ^ 0x1);
	}
}

static uint64_t one_test(struct random_state *state)
{
	uint64_t nsec_before;
	uint64_t nsec_after;

	nsec_before = get_nsec();
	memory_accesss(state);
	nsec_after = get_nsec();

	return nsec_after - nsec_before;
}

void get_memory_noise(struct random_state *state, void *buf, size_t size)
{
	uint8_t *res = buf;
	size_t i = 0;

	while (i != size) {
		uint64_t seed = one_test(state);

		while (seed) {
			res[i++] = seed & 0xff;
			seed >>= CHAR_BIT;

			if (i == size)
				break;
		}
	}
}

void memory_noise_extracted(struct random_state *rng, void *buf, size_t size)
{
	void *out = malloc(size);

	assert(out);

	get_memory_noise(rng, out, size);
	encode(out, size, buf, size);
	
	free(out);
}
