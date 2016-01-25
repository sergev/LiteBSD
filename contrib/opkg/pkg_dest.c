/* vi: set expandtab sw=4 sts=4: */
/* pkg_dest.c - the opkg package management system

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
#include <string.h>
#include <stdlib.h>

#include "pkg_dest.h"
#include "file_util.h"
#include "sprintf_alloc.h"
#include "opkg_conf.h"
#include "opkg_cmd.h"
#include "xfuncs.h"

int pkg_dest_init(pkg_dest_t * dest, const char *name, const char *root_dir)
{
    char *status_file_dir;

    dest->name = xstrdup(name);

    /* Guarantee that dest->root_dir ends with a '/' */
    if (root_dir[strlen(root_dir) - 1] == '/') {
        dest->root_dir = xstrdup(root_dir);
    } else {
        sprintf_alloc(&dest->root_dir, "%s/", root_dir);
    }
    file_mkdir_hier(dest->root_dir, 0755);

    sprintf_alloc(&dest->info_dir, "%s/%s", dest->root_dir,
                  opkg_config->info_dir);
    file_mkdir_hier(dest->info_dir, 0755);

    sprintf_alloc(&dest->status_file_name, "%s/%s", dest->root_dir,
                  opkg_config->status_file);

    /* Ensure that the directory in which we will create the status file exists.
     */
    status_file_dir = xdirname(dest->status_file_name);
    file_mkdir_hier(status_file_dir, 0755);
    free(status_file_dir);

    return 0;
}

void pkg_dest_deinit(pkg_dest_t * dest)
{
    free(dest->name);
    dest->name = NULL;

    free(dest->root_dir);
    dest->root_dir = NULL;

    free(dest->info_dir);
    dest->info_dir = NULL;

    free(dest->status_file_name);
    dest->status_file_name = NULL;
}
