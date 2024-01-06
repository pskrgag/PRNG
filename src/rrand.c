#include <rrand/random.h>
#include <rrand/private.h>
#include <rrand/noise.h>
#include <rrand/coder.h>
#include <rrand/utils.h>
#include <rrand/bm.h>

#include <libakrypt.h>

#include <stdlib.h>
#include <assert.h>
#include <limits.h>

/* TODO: better to retreive cache sizes from the OS to stress memory bus more. */
#define MEMORY_LOG_DEFAULT	20

static struct random_state *__random_state_allocate(size_t memlog)
{
	struct random_state *state = malloc(sizeof(*state));
	
	if (state) {
		state->mem = malloc(1UL << memlog);
		state->mem_size = memlog;
		if (!state->mem) {
			free(state);
			state = NULL;
		}
	}

	return state;
}

struct random_state *random_state_allocate(state_flags_t flags)
{
	size_t mem_size = flags & MEMORY_SIZE_LOG_MASK;
	struct random_state *state;
	int res;

	if (mem_size == 0)
		mem_size = MEMORY_LOG_DEFAULT;
	
	state = __random_state_allocate(mem_size);
	if (state) {
		res = ak_hash_create_streebog512(&state->streebog);
		if (res) {
			random_state_free(state);
			state = NULL;
		}
	}

	return state;
}

void random_state_free(struct random_state *state)
{
	debug_assert(state);

	ak_hash_destroy(&state->streebog);
	free(state->mem);
	free(state);
}

static void gen_more_than_hash_code(struct random_state *state, void *buffer, size_t size, size_t *generated)
{
	const size_t q = size / STREEBOG_OUTLEN_BYTES;
	size_t i;

	for (i = 1; i <= q; ++i, *generated += STREEBOG_OUTLEN_BYTES) {
		uint8_t tmp[STREEBOG_OUTLEN_BYTES];

		bm_inc(state->internal_state, sizeof(state->internal_state));
		ak_hash_finalize(&state->streebog, state->internal_state, sizeof(state->internal_state),
				 tmp, sizeof(tmp));

		memcpy(buffer + *generated, tmp, sizeof(tmp));
	}
}

static void gen_less_than_hash_code(struct random_state *state, void *buffer, size_t size, size_t *generated)
{
	uint8_t tmp[STREEBOG_OUTLEN_BYTES];

	(void)size;

	bm_inc(state->internal_state, sizeof(state->internal_state));
	ak_hash_finalize(&state->streebog, state->internal_state, sizeof(state->internal_state),
			 tmp, sizeof(tmp));

	memcpy(buffer + *generated, tmp, size - *generated);

	// To check invariant later
	*generated += (size - *generated);
}

int random_get_bytes(struct random_state *state, void *buffer, size_t size)
{
	size_t generated = 0;
	// Let R -- random vector
	// Let m -- block size of the unerlying hash function in bits

	// 1. Zero out buffer
	memset(buffer, 0x0, size);

	// size = q * h + r
	const size_t q = size / STREEBOG_OUTLEN_BYTES;
	const size_t r = size % STREEBOG_OUTLEN_BYTES;

	// Generate m - 128 bits of random data and P0 = K || 0^l, where
	// l = m - s - 8
	memset(state->internal_state, 0x0, sizeof(state->internal_state));
	noise(state, buffer + 128 / CHAR_BIT, (STREEBOG_OUTLEN_BITS - 128) / CHAR_BIT);

	// If q != 0 then generate q * h bytes of random data
	if (q) {
		gen_more_than_hash_code(state, buffer, size, &generated);
	}

	// If r != 0 then generate r bytes of random data
	if (r) {
		gen_less_than_hash_code(state, buffer, size, &generated);
	}

	// Check invariant
	assert(generated == size);

	return 0;
}
