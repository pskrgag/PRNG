#ifndef RRANDOM_RANDOM_H
#define RRANDOM_RANDOM_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t state_flags_t;

#define MEMORY_SIZE_LOG_MASK	((1UL << 6) - 1)

#define MEMORY_SIZE_LOG(x)	(x & MEMORY_SIZE_LOG)

/* Forward declaration */
struct random_state;

struct random_state *random_state_allocate(state_flags_t flags);
void random_state_free(struct random_state *state);

int random_get_bytes(struct random_state *state, void *buffer, size_t size);
int random_reseed(struct random_state *state, void *data, size_t size);

#endif
