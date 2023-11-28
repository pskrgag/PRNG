#include <rrand/random.h>
#include <rrand/utils.h>
#include <rrand/private.h>
#include <rrand/coder.h>
#include <rrand/noise.h>
#include <rrand/lcg.h>

#include <limits.h>
#include <math.h>
#include <assert.h>
#include <sys/mman.h>

#define MEM_LOOP_COUNT		256

#define READ_ONCE(x)		*((volatile typeof(x)) x)
#define WRITE_ONCE(x, val)	*((volatile typeof(x)) x) = (val)

typedef void (*noise_source)(struct random_state *);
typedef void (*noise_source_cb)(struct random_state *, void *, size_t);

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

static void stall_pipeline(struct random_state *state)
{
	(void) state;

	mfence();
}

static uint64_t one_test(struct random_state *state, noise_source source)
{
	uint64_t nsec_before;
	uint64_t nsec_after;

	nsec_before = get_nsec();
	source(state);
	nsec_after = get_nsec();

	return nsec_after - nsec_before;
}

static void get_noise(struct random_state *state, void *buf, size_t size, noise_source source)
{
	uint8_t *res = buf;
	size_t i = 0;

	while (i != size) {
		uint64_t seed = one_test(state, source);

		while (seed) {
			res[i++] = seed & 0xff;
			seed >>= CHAR_BIT;

			if (i == size)
				break;
		}
	}
}

void get_memory_noise(struct random_state *state, void *buf, size_t size)
{
	get_noise(state, buf, size, memory_accesss);
}

void get_pipeline_noise(struct random_state *state, void *buf, size_t size)
{
	get_noise(state, buf, size, stall_pipeline);
}

static void noise_extracted(struct random_state *state, void *buf, size_t size, noise_source_cb cb)
{
	const size_t need_size = ceil(size * ENCODER_SCALE_COUNT);
	void *out = malloc(need_size);
	size_t res;

	debug_assert(out);

	cb(state, out, need_size);
	res = encode(out, need_size, buf, size);

	debug_log("res %u size %u\n", res, size);
	debug_assert(res == size);
	
	free(out);
}

void noise(struct random_state *state, void *buf, size_t size)
{
	static const noise_source_cb cbs[] = {
		get_memory_noise,
		get_pipeline_noise,
	};
	size_t i, j;
	unsigned char *res = malloc(size);
	unsigned char *buf_u8 = buf;

	assert(res);

	noise_extracted(state, buf_u8, size, cbs[0]);

	for (i = 1; i < ARRAY_SIZE(cbs); ++i) {
		noise_extracted(state, res, size, cbs[i]);
	
		for (j = 0; j < size; ++j) {
			buf_u8[j] ^= res[j];
		}
	}

	free(res);
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
