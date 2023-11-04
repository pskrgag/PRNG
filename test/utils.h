#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static __attribute__((noreturn, format(printf, 1, 2))) void die(const char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);
	vfprintf(stderr, fmt, list);
	fprintf(stderr, "\n");

	exit(-1);
}

#endif
