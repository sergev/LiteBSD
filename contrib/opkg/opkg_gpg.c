/* vi: set expandtab sw=4 sts=4: */
/* opkg_gpg.c - the opkg package management system

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
#include <gpgme.h>

#include "opkg_conf.h"
#include "opkg_message.h"
#include "opkg_gpg.h"

int opkg_verify_gpg_signature(const char *file, const char *sigfile)
{
    int status = -1;
    int ret = -1;
    gpgme_ctx_t ctx;
    int have_ctx = 0;
    gpgme_data_t sig, text, key;
    int have_sig = 0, have_text = 0, have_key = 0;
    gpgme_error_t err;
    gpgme_verify_result_t result;
    gpgme_signature_t s;
    gpgme_protocol_t protocol = GPGME_PROTOCOL_OpenPGP;
    char *trusted_path = NULL;

    if (opkg_config->check_signature == 0)
        return 0;

    gpgme_check_version(NULL);

    err = gpgme_new(&ctx);
    if (err) {
        opkg_msg(ERROR, "Unable to create gpgme context: %s\n",
                 gpg_strerror(err));
        goto out_err;
    }
    have_ctx = 1;

    err = gpgme_set_protocol(ctx, protocol);
    if (err) {
        opkg_msg(ERROR, "Unable to set gpgme protocol to OpenPGP: %s\n",
                 gpg_strerror(err));
        goto out_err;
    }

    trusted_path = root_filename_alloc("/etc/opkg/trusted.gpg");
    if (!trusted_path) {
        opkg_msg(ERROR, "Out of memory!\n");
        goto out_err;
    }

    err = gpgme_data_new_from_file(&key, trusted_path, 1);
    if (err) {
        opkg_msg(ERROR, "Unable to get data from file %s: %s\n", trusted_path,
                 gpg_strerror(err));
        goto out_err;
    }
    have_key = 1;

    err = gpgme_op_import(ctx, key);
    if (err) {
        opkg_msg(ERROR, "Unable to import key from file %s: %s\n", trusted_path,
                 gpg_strerror(err));
        goto out_err;
    }

    err = gpgme_data_new_from_file(&sig, sigfile, 1);
    if (err) {
        opkg_msg(ERROR, "Unable to get data from file %s: %s\n", sigfile,
                 gpg_strerror(err));
        goto out_err;
    }
    have_sig = 1;

    err = gpgme_data_new_from_file(&text, file, 1);
    if (err) {
        opkg_msg(ERROR, "Unable to get data from file %s: %s\n", file,
                 gpg_strerror(err));
        goto out_err;
    }
    have_text = 1;

    err = gpgme_op_verify(ctx, sig, text, NULL);
    if (err) {
        opkg_msg(ERROR, "Unable to verify signature: %s\n", gpg_strerror(err));
        goto out_err;
    }

    result = gpgme_op_verify_result(ctx);
    if (!result) {
        opkg_msg(ERROR, "Unable to get verification data: %s\n",
                 gpg_strerror(err));
        goto out_err;
    }

    /* see if any of the signitures matched */
    s = result->signatures;
    while (s) {
        status = gpg_err_code(s->status);
        if (status == GPG_ERR_NO_ERROR) {
            ret = 0;
            break;
        }
        s = s->next;
    }

 out_err:
    if (have_sig)
        gpgme_data_release(sig);
    if (have_text)
        gpgme_data_release(text);
    if (have_key)
        gpgme_data_release(key);
    if (trusted_path)
        free(trusted_path);
    if (have_ctx)
        gpgme_release(ctx);

    return ret;
}
