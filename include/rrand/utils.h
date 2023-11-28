#ifndef RRANDOM_UTILS_H
#define RRANDOM_UTILS_H

#include <stdint.h>
#include <time.h>

#ifdef DEBUG
#define RAND_EXPORT 	__attribute__((visibility ("default")))
#define debug_log(fmt, ...)	printf(fmt __VA_OPT__(,) __VA_ARGS__)
#define debug_assert(e)		assert(e)
#else
#define RAND_EXPORT
#define debug_log(fmt, ...)	((void) fmt)
#define debug_assert(e)		((void) (e))
#endif

#define ARRAY_SIZE(x)		((sizeof(x) / sizeof(x[0])))

#if defined(__x86_64__)
static inline uint64_t get_nsec(void)
{
	unsigned hi, lo;
	asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}

static inline void mfence(void)
{
	// Add memory constraint to force compiler to not
	// move this intstruction
	asm volatile ("mfence":::"memory");
}
#else
# error "Your arch is not supported yet"
#endif

#endif
