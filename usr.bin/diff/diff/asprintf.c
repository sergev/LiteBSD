/* From musl-libc, MIT licensed */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>

extern int vasprintf(char **, const char *, va_list);

int asprintf(char **s, const char *fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vasprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}
