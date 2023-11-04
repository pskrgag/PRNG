#ifndef RRANDOM_BN_H
#define RRANDOM_BN_H

#include <stdint.h>
#include <stddef.h>

static void bm_add(uint8_t *a, uint8_t *b, size_t size)
{
	uint8_t carry = 0;

	for (size_t i = 0; i < size; ++i) {
		uint8_t tmp = a[i];

		a[i] += b[i] + carry;
		carry = !!(tmp > a[i]);
	}
}

static void bm_ui(uint8_t *a, size_t size, uint64_t add)
{
	uint8_t carry = 0;
	uint8_t tmp = a[0];

	a[0] += add;

	carry = !!(tmp > a[0]);

	for (size_t i = 1; i < size && carry; ++i) {
		tmp = a[i];
		a[i] += carry;
		carry = !!(tmp > a[i]);
	}
}

static void bm_inc(uint8_t *a, size_t size)
{
	return bm_ui(a, size, 1);
}

#endif
