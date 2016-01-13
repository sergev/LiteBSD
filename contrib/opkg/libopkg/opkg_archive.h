/* vi: set expandtab sw=4 sts=4: */
/* opkg_archive.h - the opkg package management system

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

#ifndef OPKG_ARCHIVE_H
#define OPKG_ARCHIVE_H

#ifdef __cplusplus
extern "C" {
#endif

struct opkg_ar {
    struct archive *ar;
    int extract_flags;
};

struct opkg_ar *ar_open_pkg_control_archive(const char *filename);
struct opkg_ar *ar_open_pkg_data_archive(const char *filename);
struct opkg_ar *ar_open_compressed_file(const char *filename);
int ar_copy_to_stream(struct opkg_ar *ar, FILE * stream);
int ar_extract_file_to_stream(struct opkg_ar *ar, const char *filename,
                              FILE * stream);
int ar_extract_paths_to_stream(struct opkg_ar *ar, FILE * stream);
int ar_extract_all(struct opkg_ar *ar, const char *prefix);
void ar_close(struct opkg_ar *ar);

#ifdef __cplusplus
}
#endif
#endif                          /* OPKG_ARCHIVE_H */
