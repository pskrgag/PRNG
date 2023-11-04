#ifndef RRANDOM_UTILS_H
#define RRANDOM_UTILS_H

#include <stdint.h>
#include <time.h>

#ifdef DEBUG
#define RAND_EXPORT 	__attribute__((visibility ("default")))
#else
#define RAND_EXPORT
#endif

static inline uint64_t get_nsec(void)
{
	uint64_t ns = 0;
	struct timespec time;

	if (clock_gettime(CLOCK_REALTIME, &time) == 0)
	{
		ns = time.tv_sec;
		ns <<= 32;
		ns |= time.tv_nsec;
	}
	
	return ns;
}

#endif
