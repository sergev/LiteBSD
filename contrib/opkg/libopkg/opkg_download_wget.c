/* vi: set expandtab sw=4 sts=4: */
/* opkg_download_wget.c - the opkg package management system

   Copyright (C) 2001 University of Southern California
   Copyright (C) 2008 OpenMoko Inc
   Copyright (C) 2014 Paul Barker

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#include "config.h"

#include <unistd.h>

#include "opkg_download.h"
#include "opkg_message.h"
#include "xsystem.h"

/* Download using wget backend.
 *
 * This backend should be as simple and minimalistic as possible. If users want
 * things like efficient caching and resuming of downloads they should use the
 * curl backend instead. By keeping the wget backend small, we can satisfy users
 * who want opkg to use very little memory.
 *
 * Both the gnu and busybox versions of wget must be supported by this backend,
 * so we are precluded from using most features anyway.
 */
int opkg_download_backend(const char *src, const char *dest,
                          curl_progress_func cb, void *data, int use_cache)
{
    int res;
    const char *argv[8];
    int i = 0;

    /* Unused arguments. */
    (void)cb;
    (void)data;
    (void)use_cache;

    unlink(dest);

    argv[i++] = "wget";
    argv[i++] = "-q";
    if (opkg_config->http_proxy || opkg_config->ftp_proxy) {
        argv[i++] = "-Y";
        argv[i++] = "on";
    }
    argv[i++] = "-O";
    argv[i++] = dest;
    argv[i++] = src;
    argv[i++] = NULL;
    res = xsystem(argv);

    if (res) {
        opkg_msg(ERROR, "Failed to download %s, wget returned %d.\n", src, res);
        return -1;
    }

    return 0;
}

void opkg_download_cleanup(void)
{
    /* Nothing to do. */
}
