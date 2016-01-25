/* vi: set expandtab sw=4 sts=4: */
/* opkg_verify.c - the opkg package management system

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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "file_util.h"
#include "opkg_conf.h"
#include "opkg_message.h"
#include "opkg_verify.h"

#ifdef HAVE_GPGME
#include "opkg_gpg.h"
#else
/* Dummy gpg signature verification. */
int opkg_verify_gpg_signature(const char *file, const char *sigfile)
{
    (void)file;
    (void)sigfile;

    opkg_msg(ERROR, "GPG signature checking not supported\n");
    return -1;
}
#endif

#ifdef HAVE_OPENSSL
#include "opkg_openssl.h"
#else
/* Dummy openssl signature verification. */
int opkg_verify_openssl_signature(const char *file, const char *sigfile)
{
    (void)file;
    (void)sigfile;

    opkg_msg(ERROR, "OpenSSL signature checking not supported\n");
    return -1;
}
#endif

int opkg_verify_md5sum(const char *file, const char *md5sum)
{
    int r;
    char *file_md5sum;

    if (!file_exists(file))
        return -1;

    file_md5sum = file_md5sum_alloc(file);
    if (!file_md5sum)
        return -1;

    r = strcmp(file_md5sum, md5sum);
    free(file_md5sum);

    return r;
}

int opkg_verify_sha256sum(const char *file, const char *sha256sum)
{
#ifdef HAVE_SHA256
    int r;
    char *file_sha256sum;

    if (!file_exists(file))
        return -1;

    file_sha256sum = file_sha256sum_alloc(file);
    if (!file_sha256sum)
        return -1;

    r = strcmp(file_sha256sum, sha256sum);
    free(file_sha256sum);

    return r;
#else
    (void)sha256sum;

    opkg_msg(INFO, "Ignoring sha256sum for file '%s'\n", file);
    return 0;
#endif
}

int opkg_verify_signature(const char *file, const char *sigfile)
{
    int use_gpg = (strcmp(opkg_config->signature_type, "gpg") == 0)
            || (strcmp(opkg_config->signature_type, "gpg-asc") == 0);
    if (use_gpg)
        return opkg_verify_gpg_signature(file, sigfile);
    else if (strcmp(opkg_config->signature_type, "openssl") == 0)
        return opkg_verify_openssl_signature(file, sigfile);

    opkg_msg(ERROR, "signature_type option '%s' not understood.\n",
             opkg_config->signature_type);
    return -1;
}
