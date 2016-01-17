/* vi: set expandtab sw=4 sts=4: */
/* opkg_utils.c - the opkg package management system

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
#include <ctype.h>
#include <string.h>
#ifdef HAVE_STATVFS
#   include <sys/statvfs.h>
#else
#   include <sys/param.h>
#   include <sys/mount.h>
#endif
#include "opkg_message.h"
#include "xfuncs.h"

unsigned long get_available_kbytes(char *filesystem)
{
    int r;
#ifdef HAVE_STATVFS
    struct statvfs f;

    r = statvfs(filesystem, &f);
    if (r == -1) {
        opkg_perror(ERROR, "Failed to statvfs for %s", filesystem);
        return 0;
    }
    // Actually ((sfs.f_bavail * sfs.f_frsize) / 1024)
    // and here we try to avoid overflow.
    if (f.f_frsize >= 1024)
        return (f.f_bavail * (f.f_frsize / 1024));
    else if (f.f_frsize > 0)
        return f.f_bavail / (1024 / f.f_frsize);
#else
    struct statfs f;

    r = statfs(filesystem, &f);
    if (r < 0) {
        opkg_perror(ERROR, "Failed to statfs for %s", filesystem);
        return 0;
    }
    // Actually ((sfs.f_bavail * sfs.f_bsize) / 1024)
    // and here we try to avoid overflow.
    if (f.f_bsize >= 1024)
        return (f.f_bavail * (f.f_bsize / 1024));
    else if (f.f_bsize > 0)
        return f.f_bavail / (1024 / f.f_bsize);
#endif
    opkg_msg(ERROR, "Unknown block size for target filesystem.\n");

    return 0;
}

/* something to remove whitespace, a hash pooper */
char *trim_xstrdup(const char *src)
{
    const char *end;

    /* remove it from the front */
    while (src && isspace(*src) && *src)
        src++;

    end = src + (strlen(src) - 1);

    /* and now from the back */
    while ((end > src) && isspace(*end))
        end--;

    end++;

    /* xstrndup will NULL terminate for us */
    return xstrndup(src, end - src);
}

int line_is_blank(const char *line)
{
    const char *s;

    for (s = line; *s; s++) {
        if (!isspace(*s))
            return 0;
    }
    return 1;
}

int str_starts_with(const char *str, const char *prefix)
{
    return (strncmp(str, prefix, strlen(prefix)) == 0);
}
