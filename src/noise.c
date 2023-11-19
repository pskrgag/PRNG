#include <rrand/random.h>
#include <rrand/utils.h>
#include <rrand/private.h>
#include <rrand/coder.h>
#include <rrand/noise.h>

#include <limits.h>
#include <math.h>
#include <assert.h>
#include <sys/mman.h>

#define MEM_LOOP_COUNT		256

#define READ_ONCE(x)		*((volatile typeof(x)) x)
#define WRITE_ONCE(x, val)	*((volatile typeof(x)) x) = (val)

struct lcg {
	uint64_t seed[2];
};

static uint64_t rand_lcg(uint64_t s[static 2])
{
	uint64_t m  = 0x9b60933458e17d7d;
	uint64_t a0 = 0xd737232eeccdf7ed;
	uint64_t a1 = 0x8b260b70b8e98891;
	uint64_t p0 = s[0];
	uint64_t p1 = s[1];
	int r0 = 29 - (p0 >> 61);
	int r1 = 29 - (p1 >> 61);
	uint64_t high = p0 >> r0;
	uint32_t low  = p1 >> r1;
	
	s[0] = p0 * m + a0;
	s[1] = p1 * m + a1;

	return (high << 32) | low;
}

static void memory_accesss(struct random_state *state)
{
	uint64_t seed[2] = { get_nsec(), get_nsec() };
	const uint64_t mem_mask = (1ULL << state->mem_size) - 1;

	for (int i = 0; i < MEM_LOOP_COUNT; ++i) {
		uint8_t *mem = state->mem + (rand_lcg(seed) & mem_mask);

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
	const size_t need_size = ceil(size * ENCODER_SCALE_COUNT);
	void *out = malloc(need_size);
	size_t res;

	debug_assert(out);

	get_memory_noise(rng, out, need_size);
	res = encode(out, need_size, buf, size);

	debug_log("res %u size %u\n", res, size);
	debug_assert(res == size);
	
	free(out);
}
