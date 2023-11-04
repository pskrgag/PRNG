#ifndef RRANDOM_PRIVATE_H
#define RRANDOM_PRIVATE_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#include <libakrypt.h>

#define STREEBOG_OUTLEN_BITS		512
#define STREEBOG_OUTLEN_BYTES		(512 / CHAR_BIT)

#define SEED_LENGHT			(888 / CHAR_BIT)

#define RESEED_COUNTER_LIMIT		(1UL << 48)

_Static_assert(SEED_LENGHT >= STREEBOG_OUTLEN_BYTES, "");

struct random_state {
	struct hash streebog;

	struct {
		uint8_t  C[SEED_LENGHT];
		struct {
			uint8_t  V[SEED_LENGHT];
			uint8_t  pad;
		};
		uint64_t counter;
	} state;

	/* Noise sources. */

	/* Memory access */
	uint8_t *mem;
	size_t mem_size;
};

#endif
