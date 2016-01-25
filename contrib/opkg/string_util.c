/* vi: set expandtab sw=4 sts=4: */
/* string_util.c - convenience routines for common string operations

   Copyright (C) 2015 Paul Barker

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/
#include "string_util.h"
#include "xfuncs.h"

char *bin_to_hex(const void *bin_data, size_t len)
{
    const unsigned char *src = (const unsigned char *)bin_data;
    char *buf = xmalloc(2 * len + 1);
    int i;

    static const unsigned char bin2hex[16] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'a', 'b',
        'c', 'd', 'e', 'f'
    };

    for (i = 0; i < len; i++) {
        buf[i * 2] = bin2hex[src[i] >> 4];
        buf[i * 2 + 1] = bin2hex[src[i] & 0xf];
    }

    buf[len * 2] = '\0';
    return buf;
}
