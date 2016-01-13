/* vi: set expandtab sw=4 sts=4: */
/*
 * Utility routines.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef	XFUNCS_H
#define	XFUNCS_H

#include <stddef.h>

extern void *xmalloc(size_t size);
extern void *xrealloc(void *old, size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern char *xstrdup(const char *s);
extern char *xstrndup(const char *s, int n);
extern char *xdirname(const char *path);

#endif                          /* XFUNCS_H */
