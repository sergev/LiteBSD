/* vi: set expandtab sw=4 sts=4: */
/* opkg_action.c - the opkg package management system

   Copyright (C) 2015 National Instruments Corp.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/
#include "opkg_solver_internal.h"

#include <fnmatch.h>
#include <signal.h>
#include <stdlib.h>

#include "opkg_message.h"
#include "opkg_install.h"
#include "opkg_upgrade.h"
#include "opkg_remove.h"
#include "pkg.h"

static void print_dependents_warning(pkg_t *pkg, abstract_pkg_t **dependents)
{
    abstract_pkg_t *dep_ab_pkg;

    opkg_msg(ERROR, "Package %s is depended upon by packages:\n", pkg->name);
    while ((dep_ab_pkg = *dependents++) != NULL) {
        if (dep_ab_pkg->state_status == SS_INSTALLED)
            opkg_msg(ERROR, "\t%s\n", dep_ab_pkg->name);
    }
    opkg_msg(ERROR, "These might cease to work if package %s is removed.\n\n",
             pkg->name);
    opkg_msg(ERROR, "Force removal of this package with --force-depends.\n");
    opkg_msg(ERROR, "Force removal of this package and its dependents\n");
    opkg_msg(ERROR, "with --force-removal-of-dependent-packages.\n");
}

int opkg_solver_remove(int num_pkgs, char **pkg_names)
{
    int i, err = 0;
    unsigned int a;
    pkg_t *pkg;
    pkg_t *pkg_to_remove;
    pkg_vec_t *available, *pkgs_to_remove;
    int done = 0;

    available = pkg_vec_alloc();
    pkgs_to_remove = pkg_vec_alloc();
    pkg_hash_fetch_all_installed(available);

    for (i = 0; i < num_pkgs; i++) {
        for (a = 0; a < available->len; a++) {
            pkg = available->pkgs[a];
            if (fnmatch(pkg_names[i], pkg->name, 0))
                 continue;
            if (opkg_config->restrict_to_default_dest) {
                pkg_to_remove = pkg_hash_fetch_installed_by_name_dest(pkg->name,
                opkg_config->default_dest);
            } else {
                pkg_to_remove = pkg_hash_fetch_installed_by_name(pkg->name);
            }

            if (pkg_to_remove == NULL) {
                opkg_msg(ERROR, "Package %s is not installed.\n", pkg->name);
                continue;
            }
            if (pkg->state_status == SS_NOT_INSTALLED) {
                opkg_msg(ERROR, "Package %s not installed.\n", pkg->name);
                continue;
            }
            /* While remove pkg with '--force-removal-of-dependent-packages',
             * pkg may be added to remove list multiple times, add status
             * check to make sure pkg only be removed once. */
            if (opkg_config->force_removal_of_dependent_packages
                && pkg->state_flag & SF_FILELIST_CHANGED
                && pkg->state_status == SS_NOT_INSTALLED)
                done = 1;

            pkg_vec_insert(pkgs_to_remove, pkg);

            /* only attempt to remove dependent installed packages if
             * force_depends is not specified or the package is being
             * replaced.
             */
            if (!opkg_config->force_depends && !(pkg->state_flag & SF_REPLACE)) {
                abstract_pkg_t **dependents;
                int has_installed_dependents = pkg_has_installed_dependents(pkg,
                        &dependents);

                if (has_installed_dependents) {
                    /*
                     * if this package is depended upon by others, then either we should
                     * not remove it or we should remove it and all of its dependents
                     */

                    if (!opkg_config->force_removal_of_dependent_packages) {
                        print_dependents_warning(pkg, dependents);
                        free(dependents);
                        err = -1;
                        continue;
                    }

                    /* get packages depending on this package - Karthik */
                    if (opkg_get_dependent_pkgs(pkg, dependents, pkgs_to_remove)) {
                        free(dependents);
                        err = -1;
                        continue;
                    }
                }
                free(dependents);
            }

            /* get autoinstalled packages that are orphaned by the removal of
             * this one */
            if (opkg_config->autoremove) {
                if (opkg_get_autoinstalled_pkgs(pkg, pkgs_to_remove))
                    err = -1;
            }
        }
    }

    for (i = 0; i < pkgs_to_remove->len; i++) {
        if (opkg_remove_pkg(pkgs_to_remove->pkgs[i])) {
            err = -1;
            break;
        } else {
            done = 1;
        }
    }

    if (done == 0)
        opkg_msg(NOTICE, "No packages removed.\n");

    pkg_vec_free(available);
    pkg_vec_free(pkgs_to_remove);
    return err;
}

int opkg_solver_install(int num_pkgs, char **pkg_names)
{
    int i;
    char *pkg_name;
    int err = 0;
    str_list_t *pkg_names_to_install = NULL;
    int r;

    if (opkg_config->combine)
        pkg_names_to_install = str_list_alloc();

    for (i = 0; i < num_pkgs; i++) {
        pkg_name = pkg_names[i];
        if (opkg_config->combine) {
            str_list_append(pkg_names_to_install, pkg_name);
        } else {
            r = opkg_install_by_name(pkg_name);
            if (r != 0) {
                opkg_msg(ERROR, "Cannot install package %s.\n", pkg_name);
                err = -1;
            }
        }
    }

    if (opkg_config->combine) {
        r = opkg_install_multiple_by_name(pkg_names_to_install);
        if (r != 0)
            err = -1;
        str_list_purge(pkg_names_to_install);
    }

    return err;
}

int opkg_solver_upgrade(int num_pkgs, char **pkg_names)
{
    int i;
    unsigned int j;
    pkg_t *pkg;
    int err = 0;
    pkg_vec_t *pkgs_to_upgrade = NULL;
    int r;

    if (num_pkgs) {
        if (opkg_config->combine)
            pkgs_to_upgrade = pkg_vec_alloc();

        for (i = 0; i < num_pkgs; i++) {
            if (opkg_config->restrict_to_default_dest) {
                pkg = pkg_hash_fetch_installed_by_name_dest(pkg_names[i],
                    opkg_config->default_dest);
                if (pkg == NULL) {
                    opkg_msg(ERROR, "Package %s not installed in %s.\n",
                        pkg_names[i], opkg_config->default_dest->name);
                    continue;
                }
            } else {
                pkg = pkg_hash_fetch_installed_by_name(pkg_names[i]);
                if (pkg == NULL) {
                    opkg_msg(ERROR, "Package %s not installed.\n", pkg_names[i]);
                    continue;
                }
            }
            if (opkg_config->combine) {
                pkg_vec_insert(pkgs_to_upgrade, pkg);
            } else {
                r = opkg_upgrade_pkg(pkg);
                if (r != 0)
                   err = -1;
            }
        }

        if (opkg_config->combine) {
            r = opkg_upgrade_multiple_pkgs(pkgs_to_upgrade);
            if (r != 0)
                err = -1;

            pkg_vec_free(pkgs_to_upgrade);
        }
    } else {
        pkg_vec_t *installed = pkg_vec_alloc();

        pkg_hash_fetch_all_installed(installed);

        if (opkg_config->combine) {
            err = opkg_upgrade_multiple_pkgs(installed);
        } else {
            for (j = 0; j < installed->len; j++) {
                pkg = installed->pkgs[j];
                r = opkg_upgrade_pkg(pkg);
                if (r != 0)
                    err = -1;
            }
        }
        pkg_vec_free(installed);
    }

    return err;
}

int opkg_solver_distupgrade(int num_pkgs, char **pkg_names)
{
    opkg_msg(ERROR, "Internal solver does not support dist-upgrade!\n");
    return -1;
}
