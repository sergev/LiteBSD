/*
 * Copyright (c) 2004 Ted Unangst and Todd Miller
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

long
strtonum(const char *numstr, long minval, long maxval, const char **errstrp)
{
	long ll = 0;
	int saved_errno = errno;
	char *ep;

	errno = 0;
	if (minval > maxval)
		goto invalid;

        ll = strtol(numstr, &ep, 10);
        if (numstr == ep || *ep != '\0')
		goto invalid;

        if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
                goto too_small;

        else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
                goto too_large;

	*errstrp = 0;
	errno = saved_errno;
	return ll;

invalid:
	*errstrp = "invalid";
	errno = EINVAL;
	return 0;

too_small:
	*errstrp = "too small";
	errno = ERANGE;
	return 0;

too_large:
	*errstrp = "too large";
	errno = ERANGE;
	return 0;
}
