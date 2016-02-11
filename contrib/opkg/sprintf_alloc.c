/* vi: set expandtab sw=4 sts=4: */
/* sprintf_alloc.c -- like sprintf with memory allocation

   Copyright (C) 2010 Ubiq Technologies <graham.gower@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "sprintf_alloc.h"
#include "xfuncs.h"

void sprintf_alloc(char **str, const char *fmt, ...)
{
    va_list ap;
    int n;
    unsigned int size = 40;

    *str = xmalloc(size);

    for (;;) {
        va_start(ap, fmt);
        n = vsnprintf(*str, size, fmt, ap);
        va_end(ap);

        if (n < 0) {
            const char message[] = "sprintf_alloc: fatal error.\n";
            write(2, message, sizeof(message)-1);
            exit(EXIT_FAILURE);
        }

        if ((unsigned int)n < size)
            break;

        /* Truncated, try again with more space. */
        size = n + 1;
        *str = xrealloc(*str, size);
    }
}
