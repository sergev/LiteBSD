/* vi: set expandtab sw=4 sts=4: */
/* pkg_src.c - the opkg package management system

   Carl D. Worth

   Copyright (C) 2001 University of Southern California

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "file_util.h"
#include "opkg_conf.h"
#include "opkg_download.h"
#include "opkg_message.h"
#include "opkg_verify.h"
#include "pkg_src.h"
#include "sprintf_alloc.h"
#include "xfuncs.h"

int pkg_src_init(pkg_src_t * src, const char *name, const char *base_url,
                 const char *extra_data, int gzip)
{
    src->gzip = gzip;
    src->name = xstrdup(name);
    src->value = xstrdup(base_url);
    if (extra_data)
        src->extra_data = xstrdup(extra_data);
    else
        src->extra_data = NULL;
    return 0;
}

void pkg_src_deinit(pkg_src_t * src)
{
    free(src->name);
    free(src->value);
    if (src->extra_data)
        free(src->extra_data);
}

int pkg_src_download(pkg_src_t * src)
{
    int err = 0;
    char *url;
    char *feed;
    const char *url_filename;

    sprintf_alloc(&feed, "%s/%s", opkg_config->lists_dir, src->name);

    url_filename = src->gzip ? "Packages.gz" : "Packages";
    if (src->extra_data)        /* debian style? */
        sprintf_alloc(&url, "%s/%s/%s", src->value, src->extra_data,
                      url_filename);
    else
        sprintf_alloc(&url, "%s/%s", src->value, url_filename);

    if (src->gzip) {
        char *cache_location;

        cache_location = opkg_download_cache(url, NULL, NULL);
        if (!cache_location) {
            err = -1;
            goto cleanup;
        }

        err = file_decompress(cache_location, feed);
        free(cache_location);
        if (err) {
            opkg_msg(ERROR, "Couldn't decompress feed for source %s.",
                     src->name);
            goto cleanup;
        }
    } else {
        err = opkg_download(url, feed, NULL, NULL);
        if (err)
            goto cleanup;
    }

    opkg_msg(DEBUG, "Downloaded package list for %s.\n", src->name);

 cleanup:
    free(feed);
    free(url);
    return err;
}

int pkg_src_download_signature(pkg_src_t * src)
{
    int err = 0;
    char *url;
    char *sigfile;
    const char *sigext;

    if (strcmp(opkg_config->signature_type, "gpg-asc") == 0)
        sigext = "asc";
    else
        sigext = "sig";

    sprintf_alloc(&sigfile, "%s/%s.%s", opkg_config->lists_dir, src->name,
                  sigext);

    /* get the url for the sig file */
    if (src->extra_data)        /* debian style? */
        sprintf_alloc(&url, "%s/%s/Packages.%s", src->value, src->extra_data,
                      sigext);
    else
        sprintf_alloc(&url, "%s/Packages.%s", src->value, sigext);

    err = opkg_download(url, sigfile, NULL, NULL);
    if (err) {
        opkg_msg(ERROR, "Failed to download signature for %s.\n", src->name);
        goto cleanup;
    }

    opkg_msg(DEBUG, "Downloaded signature for %s.\n", src->name);

 cleanup:
    free(sigfile);
    free(url);
    return err;
}

int pkg_src_verify(pkg_src_t * src)
{
    int err = 0;
    char *feed;
    char *sigfile;
    const char *sigext;

    if (strcmp(opkg_config->signature_type, "gpg-asc") == 0)
        sigext = "asc";
    else
        sigext = "sig";

    sprintf_alloc(&feed, "%s/%s", opkg_config->lists_dir, src->name);
    sprintf_alloc(&sigfile, "%s.%s", feed, sigext);

    if (!file_exists(sigfile)) {
        opkg_msg(ERROR,
                 "Signature file is missing for %s. "
                 "Perhaps you need to run 'opkg update'?\n", src->name);
        err = -1;
        goto cleanup;
    }

    err = opkg_verify_signature(feed, sigfile);
    if (err) {
        opkg_msg(ERROR, "Signature verification failed for %s.\n", src->name);
        goto cleanup;
    }

    opkg_msg(DEBUG, "Signature verification passed for %s.\n", src->name);

 cleanup:
    if (err) {
        /* Remove incorrect files. */
        unlink(feed);
        unlink(sigfile);
    }
    free(sigfile);
    free(feed);
    return err;
}

int pkg_src_update(pkg_src_t * src)
{
    int err;

    err = pkg_src_download(src);
    if (err)
        return err;

    if (opkg_config->check_signature) {
        err = pkg_src_download_signature(src);
        if (err)
            return err;

        /* pkg_src_verify deletes the downloaded files if they were
         * incorrect, so we don't have to do that here. */
        err = pkg_src_verify(src);
        if (err)
            return err;
    }

    opkg_msg(NOTICE, "Updated source '%s'.\n", src->name);
    return 0;
}
