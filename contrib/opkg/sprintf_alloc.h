/* vi: set expandtab sw=4 sts=4: */
/* sprintf_alloca.c -- like sprintf with memory allocation

   Carl D. Worth

   Copyright (C) 2001 University of Southern California

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#ifndef SPRINTF_ALLOC_H
#define SPRINTF_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

void sprintf_alloc(char **str, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif                          /* SPRINTF_ALLOC_H */
