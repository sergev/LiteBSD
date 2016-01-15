/* vi: set expandtab sw=4 sts=4: */
/* opkg_utils.h - the opkg package management system

   Steven M. Ayer

   Copyright (C) 2002 Compaq Computer Corporation

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#ifndef OPKG_UTILS_H
#define OPKG_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned long get_available_kbytes(char *filesystem);
char *trim_xstrdup(const char *line);
int line_is_blank(const char *line);
int str_starts_with(const char *str, const char *prefix);

#ifdef __cplusplus
}
#endif
#endif                          /* OPKG_UTILS_H */
