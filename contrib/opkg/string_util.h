/* vi: set expandtab sw=4 sts=4: */
/* string_util.h - convenience routines for common file operations

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

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <stddef.h>

char *bin_to_hex(const void *bin_data, size_t len);

#endif /* STRING_UTIL_H */
