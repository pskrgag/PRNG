#include <rrand/coder.h>
#include <stdint.h>
#include <limits.h>

#define ALPHABETH_SIZE	256UL
#define WINDOW_SIZE	256UL

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#define log2(n)		(63UL - __builtin_clzll(n))

_Static_assert((((ALPHABETH_SIZE + WINDOW_SIZE) - 1) & (ALPHABETH_SIZE + WINDOW_SIZE)) == 0, "Sum should be power of 2");

static inline unsigned long genmask(uint8_t h, uint8_t l)
{
	unsigned long ones = ~(0UL);

	return (ones - (1UL << l) + 1) & (ones >> (64 - 1 - h));
}


size_t encoder_redundancy(void)
{
#define	LOG2E		1.4426950408889634

	return LOG2E * (ALPHABETH_SIZE - 1) / (WINDOW_SIZE + 1) + 
		(ALPHABETH_SIZE + 1) / (WINDOW_SIZE + ALPHABETH_SIZE);
}

size_t encode(const void *_in, size_t size, void *_out, size_t max_size)
{
	uint64_t probs[ALPHABETH_SIZE];		/* probabilities of previous window */
	uint64_t Q[ALPHABETH_SIZE];		/* cumulative sums */
	uint8_t w[WINDOW_SIZE];			/* window */
	unsigned long i;
	size_t read = 0;
	uint64_t bits_cons = 0;

	const uint8_t *in = _in;
	uint8_t *out = _out;

	for (i = 0; i < ALPHABETH_SIZE; ++i)
		w[i] = i;

	while (size && bits_cons / CHAR_BIT < max_size) {
		for (i = 0; i < ALPHABETH_SIZE; ++i) {
			probs[i] = 1;
		}
	
		for (i = 0; i < ALPHABETH_SIZE; ++i) {
			probs[w[i]] += 1;
		}

		double pr = 0;

		for (i = 0; i < ALPHABETH_SIZE; ++i) {
			Q[i] = pr + (probs[i] >> 1);
			pr += probs[i];
		}

		uint8_t c = in[read++];
		uint64_t coder_res;
		unsigned long k = 1 + log2(ALPHABETH_SIZE + WINDOW_SIZE) - log2(probs[c]);

		unsigned clz = max(Q[c] ? log2((Q[c])) : 0, k);
		coder_res = (Q[c] & genmask(clz, clz - k)) >> (clz - k);

		for (i = 0; i < k && bits_cons / 8 < max_size; ++i, ++bits_cons) {
			out[bits_cons / 8] |= (!!(coder_res & (1 << i)) << (bits_cons % 8));
		}

		for (i = 0; i < WINDOW_SIZE - 1; ++i) {
			w[i] = w[i + 1];
		}
	
		w[ALPHABETH_SIZE - 1] = c;
		size -= 1;
	}

	return bits_cons / CHAR_BIT;
}
