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

static int random_state_init(struct random_state *state);

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
		} else {
			res = random_state_init(state);
			if (res) {
				random_state_free(state);
				state = NULL;
			}
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

static int Hash(struct random_state *rng, void *in, size_t size_in, void *out, size_t out_size)
{
	int res;

	res = ak_hash_finalize(&rng->streebog, in, size_in, out, out_size);
	if (res) 
		return res;
	else
		ak_hash_clean(&rng->streebog);

	return res;
}

static int HashDf(struct random_state *rng, const void *in, size_t size_in, void *out, size_t size_out)
{
	struct {
		uint8_t counter;
		uint32_t len;
		uint8_t string[];
	} *string = malloc(sizeof(*string) + size_in);
	const size_t string_size = sizeof(*string) + size_in;
	int res;

	if (!string)
		return -1;

	string->counter = 0;
	string->len = size_out * CHAR_BIT / STREEBOG_OUTLEN_BITS + !!((size_out * CHAR_BIT) % STREEBOG_OUTLEN_BITS);
	memcpy(string->string, in, size_in);

	while (size_out) {
		size_t todo = size_out > STREEBOG_OUTLEN_BYTES ? STREEBOG_OUTLEN_BYTES : size_out;

		res = Hash(rng, string, string_size, out, todo);
		if (res)
			goto out;

		string->counter++;
		size_out -= todo;
		out += todo;
	}

	res = 0;

out:
	free(string);
	return res;
}

static int HashGen(struct random_state *rng, void *out, size_t size)
{
	int res;

	while (size) {
		size_t todo = size > STREEBOG_OUTLEN_BYTES ? STREEBOG_OUTLEN_BYTES : size;

		res = Hash(rng, &rng->state.V, sizeof(rng->state.V), out, todo);
		if (res)
			return res;

		bm_inc(rng->state.V, sizeof(rng->state.V));
		size -= todo;
		out += todo;
	}

	return 0;
}

/* NIST800-90A instantiate process
 *
 *	seed = entropy || nonsense
 *	V = HashDf(seed)
 *	C = HashDf(0x00 || seed)
 *	reseed_counter = 1
 */
static int random_state_init(struct random_state *state)
{
	int res;
	struct {
		uint64_t nonsense;
		uint8_t entropy[];
	} *stringV;

	const size_t stringV_size = sizeof(*stringV) + STREEBOG_OUTLEN_BYTES * 4;

	/* Initialize V. */

	/* Memory source gives about 3 bits/byte entropy. Let input entropy be at 
	 * least STREEBOG_OUTLEN_BITS bits. */
	stringV = malloc(stringV_size);
	if (!stringV)
		return -1;

	/* Get entropy */
	memory_noise_extracted(state, stringV->entropy, STREEBOG_OUTLEN_BYTES * 4);

	/* Get nonsense */
	stringV->nonsense = get_nsec();

	/* Hash string into V. */
	res = HashDf(state, stringV, stringV_size, &state->state.V, sizeof(state->state.V));
	if (res)
		goto out;

	/* Initialize C. */

	/* Append zero byte */
	state->state.pad = 0;

	/* Hash string into C */
	res = HashDf(state, &state->state.V, sizeof(state->state.V) + 1, &state->state.C, sizeof(state->state.C));
	state->state.counter = 1;

out:
	free(stringV);
	return res;
}

static int random_state_update(struct random_state *rng)
{
	uint8_t H[SEED_LENGHT];
	int res;

	res = Hash(rng, &rng->state.V, sizeof(rng->state.V) + 1, H, STREEBOG_OUTLEN_BYTES);
	if (res)
		return res;

	bm_add(rng->state.V, H, sizeof(rng->state.V));
	bm_add(rng->state.V, rng->state.C, sizeof(rng->state.V));
	bm_ui(rng->state.V, sizeof(rng->state.V), rng->state.counter);

	return res;
}

/* NIST800-90A reseeding process:
 *
 *	seed = 0x1 || V || entropy || additional_data
 *	V = HashDf(seed)
 *	C = HashDf(0x00 || V)
 *	reseed_counter = 0
 */
int random_reseed(struct random_state *state, void *data, size_t size)
{
	/* TODO: check overflow */
	const size_t entropy_size = STREEBOG_OUTLEN_BYTES * 4;
	uint8_t *reseed_string = malloc(size + entropy_size + sizeof(state->state.V) + 1);
	size_t offset = 0;
	int res;

	if (!reseed_string)
		return -1;

	memcpy(reseed_string + offset, data, size);
	offset += size;

	memory_noise_extracted(state, reseed_string + offset, entropy_size);
	offset += entropy_size;

	memcpy(reseed_string + offset, &state->state.V, sizeof(state->state.V));
	offset += sizeof(state->state.V);

	*(uint8_t *)(reseed_string + offset) = 0x1;

	res = HashDf(state, reseed_string, size + entropy_size + sizeof(state->state.V) + 1,
			&state->state.V, sizeof(state->state.V));
	if (res)
		goto out;

	state->state.pad = 0x00;

	res = HashDf(state, &state->state.V, sizeof(state->state.V) + 1, 
			state->state.C, sizeof(state->state.C));
	if (res)
		goto out;

	state->state.counter = 1;

out:
	free(reseed_string);
	return res;
}

int random_get_bytes(struct random_state *state, void *buffer, size_t size)
{
	int res;

	if (state->state.counter > RESEED_COUNTER_LIMIT) {
		/* Reseed with zero additional information. */
		random_reseed(state, NULL, 0);
	}

	res = HashGen(state, buffer, size);
	if (res)
		return res;

	state->state.counter += 1;

	return random_state_update(state);
}
