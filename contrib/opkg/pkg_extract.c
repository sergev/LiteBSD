/* vi: set expandtab sw=4 sts=4: */
/* pkg_extract.c - the opkg package management system

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
#include <stdio.h>
#include <stdlib.h>

#include "opkg_message.h"
#include "opkg_archive.h"
#include "pkg_extract.h"
#include "sprintf_alloc.h"

int pkg_extract_control_file_to_stream(pkg_t * pkg, FILE * stream)
{
    int r;
    struct opkg_ar *ar;

    ar = ar_open_pkg_control_archive(pkg->local_filename);
    if (!ar) {
        opkg_msg(ERROR, "Failed to extract control.tar.gz from package '%s'.\n",
                 pkg->local_filename);
        return -1;
    }

    r = ar_extract_file_to_stream(ar, "control", stream);
    if (r < 0)
        opkg_msg(ERROR, "Failed to extract control file from package '%s'.\n",
                 pkg->local_filename);

    ar_close(ar);
    return r;
}

int pkg_extract_control_files_to_dir_with_prefix(pkg_t * pkg, const char *dir,
                                                 const char *prefix)
{
    int r = -1;
    char *dir_with_prefix;
    struct opkg_ar *ar;

    sprintf_alloc(&dir_with_prefix, "%s/%s", dir, prefix);

    ar = ar_open_pkg_control_archive(pkg->local_filename);
    if (!ar) {
        opkg_msg(ERROR, "Failed to extract control.tar.gz from package '%s'.\n",
                 pkg->local_filename);
        goto cleanup;
    }

    r = ar_extract_all(ar, dir_with_prefix);
    if (r < 0)
        opkg_msg(ERROR,
                 "Failed to extract all control files from package '%s'.\n",
                 pkg->local_filename);

 cleanup:
    free(dir_with_prefix);
    if (ar)
        ar_close(ar);
    return r;
}

int pkg_extract_control_files_to_dir(pkg_t * pkg, const char *dir)
{
    return pkg_extract_control_files_to_dir_with_prefix(pkg, dir, "");
}

int pkg_extract_data_files_to_dir(pkg_t * pkg, const char *dir)
{
    int r;
    struct opkg_ar *ar;

    ar = ar_open_pkg_data_archive(pkg->local_filename);
    if (!ar) {
        opkg_msg(ERROR, "Failed to extract data.tar.gz from package '%s'.\n",
                 pkg->local_filename);
        return -1;
    }

    r = ar_extract_all(ar, dir);
    if (r < 0)
        opkg_msg(ERROR, "Failed to extract data files from package '%s'.\n",
                 pkg->local_filename);

    ar_close(ar);
    return r;
}

int pkg_extract_data_file_names_to_stream(pkg_t * pkg, FILE * stream)
{
    int r;
    struct opkg_ar *ar;

    ar = ar_open_pkg_data_archive(pkg->local_filename);
    if (!ar) {
        opkg_msg(ERROR, "Failed to extract data.tar.gz from package '%s'.\n",
                 pkg->local_filename);
        return -1;
    }

    r = ar_extract_paths_to_stream(ar, stream);
    if (r < 0)
        opkg_msg(ERROR,
                 "Failed to extract data file names from package '%s'.\n",
                 pkg->local_filename);

    ar_close(ar);
    return r;
}
