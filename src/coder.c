#include <rrand/coder.h>
#include <rrand/utils.h>

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

#define ALPHABETH_SIZE	256UL
#define WINDOW_SIZE	256UL

#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#define log2(n)		(63UL - __builtin_clzll(n))

#define T		log2(ALPHABETH_SIZE + WINDOW_SIZE)
#define REG_SIZE	(log2(ALPHABETH_SIZE + WINDOW_SIZE) + 2)
#define REG_MAX_BIT	(REG_SIZE - 1)
#define REG_MAX		((1U << REG_SIZE) - 1)

_Static_assert(ALPHABETH_SIZE == WINDOW_SIZE, "Sum should be power of 2");
_Static_assert(T == 9, "9");

static inline __attribute__((pure, unused)) unsigned long genmask(uint8_t h, uint8_t l) 
{
	unsigned long ones = ~(0UL);

	return (ones - (1UL << l) + 1) & (ones >> (64 - 1 - h));
}

static int msb(uint16_t val)
{
	return !!(val & (1 << REG_MAX_BIT));
}

static int pmsb(uint16_t val)
{
	return !!(val & (1 << (REG_MAX_BIT - 1)));
}

#define BIT_OUT(bit, out, count, max)					\
	do {								\
		out[count / CHAR_BIT] |= ((bit) << (count % CHAR_BIT));	\
		if (++count / CHAR_BIT == max)				\
			return max;					\
	} while (0)

size_t encode(const void *_in, size_t size, void *_out, size_t max_size)
{
	uint64_t probs[ALPHABETH_SIZE];		/* probabilities of previous window */
	uint64_t Q[ALPHABETH_SIZE + 1];		/* cumulative sums */
	uint8_t w[WINDOW_SIZE];			/* window */
	unsigned long i;
	size_t read = 0;
	uint64_t bits_cons = 0;

	const uint8_t *in = _in;
	uint8_t *out = _out;

	/* Otherwise there is not need. */
	debug_assert(size >= max_size);

	memset(out, 0x0, max_size);

	for (i = 0; i < WINDOW_SIZE; ++i)
		w[i] = i;

	uint16_t d, h = REG_MAX, l = 0, s = 0;

	while (size) {
		for (i = 0; i < ALPHABETH_SIZE; ++i) {
			probs[i] = 1;
		}
	
		for (i = 0; i < WINDOW_SIZE; ++i) {
			probs[w[i]] += 1;
		}

		Q[0] = 0;

		for (i = 1; i < ALPHABETH_SIZE + 1; ++i) {
			Q[i] = Q[i - 1] + probs[i - 1];
		}

		uint8_t c = in[read++];

		d = h - l + 1;
		h = l + ((d * Q[c + 1]) >> T) - 1;
		l = l + ((d * Q[c]) >> T);

		while (msb(l) == msb(h)) {
			uint8_t msb_l = msb(l);

			debug_assert(msb_l < 2);

			BIT_OUT(msb_l, out, bits_cons, max_size);

			for (i = 0; i < s; ++i) {
				BIT_OUT(1 - msb_l, out, bits_cons, max_size);
			}

			s = 0;

			l = (l << 1) & REG_MAX;
			h = ((h << 1) + 1) & REG_MAX;
		}

		while (pmsb(l) == 1 && pmsb(h) == 0) {
			s++;
			l = (l << 1) & REG_MAX;
			h = ((h << 1) + 1) & REG_MAX;
		}

		l &= ~(1 << REG_MAX_BIT);
		h |= (1 << REG_MAX_BIT);

		/* Move window. */
		for (i = 0; i < WINDOW_SIZE - 1; ++i) {
			w[i] = w[i + 1];
		}

		w[WINDOW_SIZE - 1] = c;
		size -= 1;
	}

	uint8_t msb_l = pmsb(l);

	debug_assert(msb_l < 2);
	s++;

	BIT_OUT(msb_l, out, bits_cons, max_size);

	for (i = 0; i < s; ++i) {
		BIT_OUT(1 - msb_l, out, bits_cons, max_size);
	}

	return bits_cons / CHAR_BIT;
}
