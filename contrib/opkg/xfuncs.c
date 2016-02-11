/* vi: set expandtab sw=4 sts=4: */
/*
 * Utility routines.
 *
 * Copyright (C) 1999,2000,2001 by Erik Andersen <andersee@debian.org>
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "opkg_message.h"
#include "xfuncs.h"

extern void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL && size != 0) {
        write(2, "xmalloc: Out of memory.\n", 24);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

extern void *xrealloc(void *ptr, size_t size)
{
    ptr = realloc(ptr, size);
    if (ptr == NULL && size != 0) {
        write(2, "xrealloc: Out of memory.\n", 25);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

extern void *xcalloc(size_t nmemb, size_t size)
{
    void *ptr = calloc(nmemb, size);
    if (ptr == NULL && nmemb != 0 && size != 0) {
        write(2, "xcalloc: Out of memory.\n", 24);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

extern char *xstrdup(const char *s)
{
    char *t;

    if (s == NULL)
        return NULL;

    t = strdup(s);

    if (t == NULL) {
        write(2, "xstrdup: Out of memory.\n", 24);
        exit(EXIT_FAILURE);
    }

    return t;
}

extern char *xstrndup(const char *s, int n)
{
    char *t;
    size_t len;

    if (s == NULL)
        return NULL;

    len = strlen (s);
    if (n < len)
        len = n;

    t = (char*) malloc (len + 1);
    if (t == NULL) {
        write(2, "xstrndup: Out of memory.\n", 25);
        exit(EXIT_FAILURE);
    }
    memcpy (t, s, len);
    t[len] = '\0';

    return t;
}

/* Sane dirname. */
extern char *xdirname(const char *path)
{
    char *parent, *tmp;

    /* Find and strip the base name */
    parent = xstrdup(path);
    tmp = strrchr(parent, '/');
    if (tmp && tmp[1] == 0) {
        /* Strip trailing slashes */
        do {
            *tmp-- = 0;
            if (tmp < parent) {
                /* Root directory */
root:           parent[0] = '/';
                parent[1] = 0;
                return parent;
            }
        } while (*tmp == '/');
        tmp = strrchr(parent, '/');
    }
    if (! tmp) {
        /* No directory prefix */
        parent[0] = '.';
        parent[1] = 0;
        return parent;
    }

    /* Strip base name */
    do {
        *tmp-- = 0;
        if (tmp < parent)
            goto root;
    } while (*tmp == '/');
    return parent;
}
