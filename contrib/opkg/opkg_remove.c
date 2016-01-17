/* vi: set expandtab sw=4 sts=4: */
/* opkg_remove.c - the opkg package management system

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
#include <glob.h>
#include <unistd.h>
#include <stdlib.h>

#include "opkg_message.h"
#include "opkg_remove.h"
#include "opkg_cmd.h"
#include "file_util.h"
#include "sprintf_alloc.h"
#include "xfuncs.h"

void remove_data_files_and_list(pkg_t * pkg)
{
    str_list_t installed_dirs;
    str_list_t *installed_files;
    str_list_elt_t *iter;
    char *file_name;
    conffile_t *conffile;
    int removed_a_dir;
    pkg_t *owner;
    int rootdirlen = 0;
    int r;

    installed_files = pkg_get_installed_files(pkg);
    if (installed_files == NULL) {
        opkg_msg(ERROR,
                 "Failed to determine installed "
                 "files for %s. None removed.\n", pkg->name);
        return;
    }

    str_list_init(&installed_dirs);

    /* don't include trailing slash */
    if (opkg_config->offline_root)
        rootdirlen = strlen(opkg_config->offline_root);

    for (iter = str_list_first(installed_files); iter;
            iter = str_list_next(installed_files, iter)) {
        file_name = (char *)iter->data;

        owner = file_hash_get_file_owner(file_name);
        if (owner != pkg)
            /* File may have been claimed by another package. */
            continue;

        if (!file_is_symlink(file_name) && file_is_dir(file_name)) {
            str_list_append(&installed_dirs, file_name);
            continue;
        }

        conffile = pkg_get_conffile(pkg, file_name + rootdirlen);
        if (conffile) {
            if (conffile_has_been_modified(conffile)) {
                opkg_msg(NOTICE, "Not deleting modified conffile %s.\n",
                         file_name);
                continue;
            }
        }

        if (!opkg_config->noaction) {
            opkg_msg(INFO, "Deleting %s.\n", file_name);
            unlink(file_name);
        } else
            opkg_msg(INFO, "Not deleting %s. (noaction)\n", file_name);

        file_hash_remove(file_name);
    }

    /* Remove empty directories */
    if (!opkg_config->noaction) {
        do {
            removed_a_dir = 0;
            for (iter = str_list_first(&installed_dirs); iter;
                    iter = str_list_next(&installed_dirs, iter)) {
                file_name = (char *)iter->data;

                r = rmdir(file_name);
                if (r == 0) {
                    opkg_msg(INFO, "Deleting %s.\n", file_name);
                    removed_a_dir = 1;
                    str_list_remove(&installed_dirs, &iter);
                }
            }
        } while (removed_a_dir);
    }

    pkg_free_installed_files(pkg);
    pkg_remove_installed_files_list(pkg);

    /* Don't print warning for dirs that are provided by other packages */
    for (iter = str_list_first(&installed_dirs); iter;
            iter = str_list_next(&installed_dirs, iter)) {
        file_name = (char *)iter->data;

        owner = file_hash_get_file_owner(file_name);
        if (owner) {
            free(iter->data);
            iter->data = NULL;
            str_list_remove(&installed_dirs, &iter);
        }
    }

    /* cleanup */
    while (!void_list_empty(&installed_dirs)) {
        iter = str_list_pop(&installed_dirs);
        free(iter->data);
        free(iter);
    }
    str_list_deinit(&installed_dirs);
}

void remove_maintainer_scripts(pkg_t * pkg)
{
    unsigned int i;
    int err;
    char *globpattern;
    glob_t globbuf;

    if (opkg_config->noaction)
        return;

    sprintf_alloc(&globpattern, "%s/%s.*", pkg->dest->info_dir, pkg->name);

    err = glob(globpattern, 0, NULL, &globbuf);
    free(globpattern);
    if (err)
        return;

    for (i = 0; i < globbuf.gl_pathc; i++) {
        opkg_msg(INFO, "Deleting %s.\n", globbuf.gl_pathv[i]);
        unlink(globbuf.gl_pathv[i]);
    }
    globfree(&globbuf);
}

int opkg_remove_pkg(pkg_t * pkg)
{
    int err;
    int r;

    /*
     * If called from an upgrade and not from a normal remove,
     * ignore the essential flag.
     */
    if (pkg->essential) {
        if (opkg_config->force_removal_of_essential_packages) {
            opkg_msg(NOTICE,
                     "Removing essential package %s under your coercion.\n"
                     "\tIf your system breaks, you get to keep both pieces\n",
                     pkg->name);
        } else {
            opkg_msg(NOTICE,
                     "Refusing to remove essential package %s.\n"
                     "\tRemoving an essential package may lead to an unusable system, but if\n"
                     "\tyou enjoy that kind of pain, you can force opkg to proceed against\n"
                     "\tits will with the option: --force-removal-of-essential-packages\n",
                     pkg->name);
            return -1;
        }
    }

    if (pkg->parent == NULL)
        return 0;

    opkg_msg(NOTICE, "Removing package %s from %s...\n", pkg->name,
             pkg->dest->name);
    pkg->state_flag |= SF_FILELIST_CHANGED;

    pkg->state_want = SW_DEINSTALL;
    opkg_state_changed++;

    r = pkg_run_script(pkg, "prerm", "remove");
    if (r != 0) {
        if (!opkg_config->force_remove) {
            opkg_msg(ERROR,
                     "not removing package \"%s\", " "prerm script failed\n",
                     pkg->name);
            opkg_msg(NOTICE,
                     "You can force removal of packages with failed "
                     "prerm scripts with the option: \n" "\t--force-remove\n");
            return -1;
        }
    }

    /* DPKG_INCOMPATIBILITY: dpkg is slightly different here. It
     * maintains an empty filelist rather than deleting it. That seems
     * like a big pain, and I don't see that that should make a big
     * difference, but for anyone who wants tighter compatibility,
     * feel free to fix this. */
    remove_data_files_and_list(pkg);

    err = pkg_run_script(pkg, "postrm", "remove");

    remove_maintainer_scripts(pkg);
    pkg->state_status = SS_NOT_INSTALLED;

    pkg->parent->state_status = SS_NOT_INSTALLED;

    return err;
}
