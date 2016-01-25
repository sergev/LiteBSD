/* vi: set expandtab sw=4 sts=4: */
/* opkg_download.c - the opkg package management system

   Carl D. Worth

   Copyright (C) 2001 University of Southern California
   Copyright (C) 2008 OpenMoko Inc

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "opkg_download.h"
#include "opkg_message.h"
#include "opkg_verify.h"
#include "opkg_utils.h"

#include "md5.h"
#include "sprintf_alloc.h"
#include "file_util.h"
#include "xfuncs.h"

/* Limit the short file name used to generate cache file names to 90 characters
 * so that when added to the md5sum (32 characters) and an underscore, the
 * resulting length is below 128 characters. The maximum file name length
 * differs between plaforms but 128 characters should be reasonable.
 */
#define MAX_SHORT_FILE_NAME_LENGTH 90

static int opkg_download_set_env()
{
    int r;

    if (opkg_config->http_proxy) {
        opkg_msg(DEBUG, "Setting environment variable: http_proxy = %s.\n",
                 opkg_config->http_proxy);
        r = setenv("http_proxy", opkg_config->http_proxy, 1);
        if (r != 0) {
            opkg_msg(ERROR, "Failed to set environment variable http_proxy");
            return r;
        }
    }
    if (opkg_config->https_proxy) {
        opkg_msg(DEBUG, "Setting environment variable: https_proxy = %s.\n",
                 opkg_config->https_proxy);
        r = setenv("https_proxy", opkg_config->https_proxy, 1);
        if (r != 0) {
            opkg_msg(ERROR, "Failed to set environment variable https_proxy");
            return r;
        }
    }
    if (opkg_config->ftp_proxy) {
        opkg_msg(DEBUG, "Setting environment variable: ftp_proxy = %s.\n",
                 opkg_config->ftp_proxy);
        r = setenv("ftp_proxy", opkg_config->ftp_proxy, 1);
        if (r != 0) {
            opkg_msg(ERROR, "Failed to set environment variable ftp_proxy");
            return r;
        }
    }
    if (opkg_config->no_proxy) {
        opkg_msg(DEBUG, "Setting environment variable: no_proxy = %s.\n",
                 opkg_config->no_proxy);
        r = setenv("no_proxy", opkg_config->no_proxy, 1);
        if (r != 0) {
            opkg_msg(ERROR, "Failed to set environment variable no_proxy");
            return r;
        }
    }

    return 0;
}

static int opkg_download_file(const char *src, const char *dest)
{
    if (!file_exists(src)) {
        opkg_msg(ERROR, "%s: No such file.\n", src);
        return -1;
    }

    /* Currently there is no attempt to check whether the destination file
     * already matches the source file. If doing so is worthwhile, it can be
     * added.
     */

    if (!opkg_config->cache_local_files)
        return file_link(src, dest);

    return file_copy(src, dest);
}

/** \brief opkg_download_internal: downloads file with existence check
 *
 * \param src absolute URI of file to download
 * \param dest destination path for downloaded file
 * \param cb callback for curl download progress
 * \param data data to pass to progress callback
 * \param use_cache 1 if file is downloaded into cache or 0 otherwise
 * \return 0 if success, -1 if error occurs
 *
 */
int opkg_download_internal(const char *src, const char *dest,
                           curl_progress_func cb, void *data, int use_cache)
{
    int ret;

    opkg_msg(NOTICE, "Downloading %s.\n", src);

    if (str_starts_with(src, "file:")) {
        const char *file_src = src + 5;

        return opkg_download_file(file_src, dest);
    }

    ret = opkg_download_set_env();
    if (ret != 0) {
        /* Error message already printed. */
        return ret;
    }

    return opkg_download_backend(src, dest, cb, data, use_cache);
}

/** \brief get_cache_location: generate cached file path
 *
 * \param src absolute URI of remote file to generate path for
 * \return generated file path
 *
 */
char *get_cache_location(const char *src)
{
    unsigned char md5sum_bin[16];
    char *md5sum_hex;
    char *cache_location;
    char *short_file_name;
    char *tmp = xstrdup(src);

    md5_buffer(src, strlen(src), md5sum_bin);
    md5sum_hex = md5_to_string(md5sum_bin);

    /* Generate a short file name which will be used along with an md5sum of the
     * full src URI in the cache file name. This short file name is limited to
     * MAX_SHORT_FILE_NAME_LENGTH to ensure that the total cache file name
     * length is reasonable.
     */
    short_file_name = strrchr(tmp, '/');
    short_file_name = short_file_name ? short_file_name+1 : tmp;
    if (strlen(short_file_name) > MAX_SHORT_FILE_NAME_LENGTH)
        short_file_name[MAX_SHORT_FILE_NAME_LENGTH] = '\0';

    sprintf_alloc(&cache_location, "%s/%s_%s", opkg_config->cache_dir,
                  md5sum_hex, short_file_name);
    free(md5sum_hex);
    free(tmp);
    return cache_location;
}

/** \brief opkg_download_cache: downloads file into cache
 *
 * \param src absolute URI of file to download
 * \param cb callback for curl download progress
 * \param data data to pass to progress callback
 * \return path of downloaded file in cache or NULL if error occurs
 *
 */
char *opkg_download_cache(const char *src, curl_progress_func cb, void *data)
{
    char *cache_location;
    int err;

    cache_location = get_cache_location(src);
    err = opkg_download_internal(src, cache_location, cb, data, 1);
    if (err) {
        free(cache_location);
        cache_location = NULL;
    }
    return cache_location;
}

/** \brief opkg_download_direct: downloads file directly
 *
 * \param src absolute URI of file to download
 * \param dest destination path for downloaded file
 * \param cb callback for curl download progress
 * \param data data to pass to progress callback
 * \return 0 on success, <0 on failure
 *
 */
int opkg_download_direct(const char *src, const char *dest,
                         curl_progress_func cb, void *data)
{
    return opkg_download_internal(src, dest, cb, data, 0);
}

int opkg_download(const char *src, const char *dest_file_name,
                  curl_progress_func cb, void *data)
{
    int err = -1;

    if (!opkg_config->volatile_cache) {
        char *cache_location = opkg_download_cache(src, cb, data);
        if (cache_location) {
            err = file_copy(cache_location, dest_file_name);
            free(cache_location);
        }
    } else {
        err = opkg_download_direct(src, dest_file_name, NULL, NULL);
    }
    return err;
}

static char *get_pkg_url(pkg_t * pkg)
{
    char *url;

    if (pkg->src == NULL) {
        opkg_msg(ERROR,
                 "Package %s is not available from any configured src.\n",
                 pkg->name);
        return NULL;
    }
    if (pkg->filename == NULL) {
        opkg_msg(ERROR, "Package %s does not have a valid filename field.\n",
                 pkg->name);
        return NULL;
    }

    sprintf_alloc(&url, "%s/%s", pkg->src->value, pkg->filename);
    return url;
}

char *pkg_download_signature(pkg_t * pkg)
{
    char *pkg_url;
    char *sig_url;
    char *sig_ext;
    char *sig_file;

    pkg_url = get_pkg_url(pkg);
    if (!pkg_url)
        return NULL;

    if (strcmp(opkg_config->signature_type, "gpg-asc") == 0)
        sig_ext = "asc";
    else
        sig_ext = "sig";

    sprintf_alloc(&sig_url, "%s.%s", pkg_url, sig_ext);
    free(pkg_url);

    sig_file = opkg_download_cache(sig_url, NULL, NULL);
    free(sig_url);

    return sig_file;
}

/** \brief opkg_download_pkg: download and verify a package
 *
 * \param pkg the package to download
 * \return 0 if success, -1 if error occurs
 *
 */
int opkg_download_pkg(pkg_t * pkg)
{
    char *url;
    int err = 0;

    url = get_pkg_url(pkg);
    if (!url)
        return -1;

    pkg->local_filename = get_cache_location(url);

    /* Check if valid package exists in cache */
    err = pkg_verify(pkg);
    if (!err)
        goto cleanup;

    err = opkg_download_internal(url, pkg->local_filename, NULL, NULL, 1);
    if (err) {
	free(pkg->local_filename);
	pkg->local_filename = NULL;
        goto cleanup;
    }

    /* Ensure downloaded package is valid. */
    err = pkg_verify(pkg);

 cleanup:
    free(url);
    return err;
}

int opkg_download_pkg_to_dir(pkg_t * pkg, const char *dir)
{
    char *dest_file_name;
    char *url = NULL;
    int err = 0;

    sprintf_alloc(&dest_file_name, "%s/%s", dir, pkg->filename);

    if (opkg_config->volatile_cache) {
        url = get_pkg_url(pkg);
        if (!url)
            goto cleanup;

        err = opkg_download_direct(url, dest_file_name, NULL, NULL);
        if (err)
            goto cleanup;

        /* This is a bit hackish
         * TODO: Clean this up!
         */
        pkg->local_filename = dest_file_name;
        err = pkg_verify(pkg);
        pkg->local_filename = NULL;
    } else {
        err = opkg_download_pkg(pkg);
        if (err)
            goto cleanup;

        err = file_copy(pkg->local_filename, dest_file_name);
    }

 cleanup:
    free(url);
    free(dest_file_name);
    return err;
}

/*
 * Returns 1 if URL "url" is prefixed by a remote protocol, 0 otherwise.
 */
static int url_has_remote_protocol(const char *url)
{
    static const char *remote_protos[] = {
        "http://",
        "ftp://",
        "https://",
        "ftps://"
    };
    static const size_t nb_remote_protos =
        sizeof(remote_protos) / sizeof(char *);

    unsigned int i = 0;
    for (i = 0; i < nb_remote_protos; ++i) {
        if (str_starts_with(url, remote_protos[i]))
            return 1;
    }

    return 0;
}

static int opkg_prepare_file_for_install(const char *path, char **namep)
{
    int r;
    pkg_t *pkg = pkg_new();

    r = pkg_init_from_file(pkg, path);
    if (r)
        return r;

    opkg_msg(DEBUG2, "Package %s provided by file '%s'.\n", pkg->name,
             pkg->local_filename);
    pkg->provided_by_hand = 1;

    pkg->dest = opkg_config->default_dest;
    pkg->state_want = SW_INSTALL;
    pkg->state_flag |= SF_PREFER;

    if (opkg_config->force_reinstall)
        pkg->force_reinstall = 1;
    else {
        /* Disallow a reinstall of the same package version if force_reinstall
         * is not set. This is because in this case orphaned files may be left
         * behind.
         */
        pkg_t *old_pkg = pkg_hash_fetch_installed_by_name(pkg->name);
        if (old_pkg && (pkg_compare_versions(pkg, old_pkg) == 0)) {
            char *version = pkg_version_str_alloc(old_pkg);
            opkg_msg(ERROR,
                     "Refusing to load file '%s' as it matches the installed version of %s (%s).\n",
                     path, old_pkg->name, version);
            free(version);
            pkg_deinit(pkg);
            free(pkg);
            return -1;
        }
    }

    hash_insert_pkg(pkg, 1);

    if (namep)
        *namep = pkg->name;
    return 0;
}

/* Prepare a given URL for installation. We use a few simple heuristics to
 * determine whether this is a remote URL, file name or abstract package name.
 */
int opkg_prepare_url_for_install(const char *url, char **namep)
{
    int r;

    /* First heuristic: Maybe it's a remote URL. */
    if (url_has_remote_protocol(url)) {
        char *cache_location;

        cache_location = opkg_download_cache(url, NULL, NULL);
        if (!cache_location)
            return -1;

        r = opkg_prepare_file_for_install(cache_location, namep);
        free(cache_location);
        return r;
    }

    /* Second heuristic: Maybe it's a package name.
     *
     * We check this before files incase an existing file incidentally shares a
     * name with an available package.
     */
    if (abstract_pkg_fetch_by_name(url)) {
        if (opkg_config->force_reinstall) {
            /* Reload the given package from its package file into a new package
             * object. This new object can then be marked as force_reinstall and
             * the reinstall should go ahead like an upgrade.
             */
            pkg_t *pkg;
            pkg = pkg_hash_fetch_best_installation_candidate_by_name(url);
            if (!pkg) {
                opkg_msg(ERROR, "Unknown package %s, cannot force reinstall.\n",
                         url);
                return -1;
            }
            r = opkg_download_pkg(pkg);
            if (r)
                return r;

            return opkg_prepare_file_for_install(pkg->local_filename, namep);
        }

        /* Nothing special to do. */
        return 0;
    }

    /* Third heuristic: Maybe it's a file. */
    if (file_exists(url))
        return opkg_prepare_file_for_install(url, namep);

    /* Can't find anything matching the requested URL. */
    opkg_msg(ERROR, "Couldn't find anything to satisfy '%s'.\n", url);
    return -1;
}
