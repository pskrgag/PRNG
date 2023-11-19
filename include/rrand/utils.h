#ifndef RRANDOM_UTILS_H
#define RRANDOM_UTILS_H

#include <stdint.h>
#include <time.h>

#ifdef DEBUG
#define RAND_EXPORT 	__attribute__((visibility ("default")))
#define debug_log(fmt, ...)	printf(fmt __VA_OPT(,) __VA_ARGS__)
#define debug_assert(e)		assert(e)
#else
#define RAND_EXPORT
#define debug_log(fmt, ...)	((void) fmt)
#define debug_assert(e)		((void) (e))
#endif

static inline uint64_t get_nsec(void)
{
	unsigned hi, lo;
	asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}

#endif
