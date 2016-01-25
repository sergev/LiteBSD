/* vi: set expandtab sw=4 sts=4: */
/* opkg_solver_internal.c - the opkg package management system

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
#include <stdlib.h>

#include "opkg_message.h"
#include "pkg.h"
#include "xfuncs.h"

/*
 * Returns number of the number of packages depending on the packages provided by this package.
 * Every package implicitly provides itself.
 */
int pkg_has_installed_dependents(pkg_t *pkg, abstract_pkg_t ***pdependents)
{
    int nprovides = pkg->provides_count;
    abstract_pkg_t **provides = pkg->provides;
    unsigned int n_installed_dependents = 0;
    unsigned int n_deps;
    int i, j;
    for (i = 0; i < nprovides; i++) {
        abstract_pkg_t *providee = provides[i];
        abstract_pkg_t *dep_ab_pkg;

        n_deps = providee->depended_upon_by->len;
        for (j = 0; j < n_deps; j++) {
            dep_ab_pkg = providee->depended_upon_by->pkgs[j];
            int dep_installed = (dep_ab_pkg->state_status == SS_INSTALLED)
                    || (dep_ab_pkg->state_status == SS_UNPACKED);
            if (dep_installed)
                n_installed_dependents++;
        }
    }
    /* if caller requested the set of installed dependents */
    if (pdependents) {
        int p = 0;
        abstract_pkg_t **dependents =
            xcalloc((n_installed_dependents + 1), sizeof(abstract_pkg_t *));

        *pdependents = dependents;
        for (i = 0; i < nprovides; i++) {
            abstract_pkg_t *providee = provides[i];
            abstract_pkg_t *dep_ab_pkg;

            n_deps = providee->depended_upon_by->len;
            for (j = 0; j < n_deps; j++) {
                dep_ab_pkg = providee->depended_upon_by->pkgs[j];
                int installed_not_marked =
                        (dep_ab_pkg->state_status == SS_INSTALLED
                         && !(dep_ab_pkg->state_flag & SF_MARKED));
                if (installed_not_marked) {
                    dependents[p++] = dep_ab_pkg;
                    dep_ab_pkg->state_flag |= SF_MARKED;
                }
            }
        }
        dependents[p] = NULL;
        /* now clear the marks */
        for (i = 0; i < p; i++) {
            abstract_pkg_t *dep_ab_pkg = dependents[i];
            dep_ab_pkg->state_flag &= ~SF_MARKED;
        }
    }
    return n_installed_dependents;
}

int opkg_get_dependent_pkgs(pkg_t *pkg, abstract_pkg_t **dependents, pkg_vec_t *dependent_pkgs)
{
    unsigned int i;
    unsigned int a;
    int count;
    abstract_pkg_t *ab_pkg;

    ab_pkg = pkg->parent;
    if (ab_pkg == NULL) {
        opkg_msg(ERROR, "Internal error: pkg %s isn't in hash table\n",
                 pkg->name);
        return -1;
    }

    if (dependents == NULL)
        return 0;

    /* here i am using the dependencies_checked */
    if (ab_pkg->dependencies_checked == 2)      /* variable to make out whether this package */
        return 0;               /* has already been encountered in the process */
    /* of marking packages for removal - Karthik */
    ab_pkg->dependencies_checked = 2;

    count = 1;

    for (i = 0; dependents[i] != NULL; i++) {
        abstract_pkg_t *dep_ab_pkg = dependents[i];

        if (dep_ab_pkg->dependencies_checked == 2)
            continue;
        if (dep_ab_pkg->state_status == SS_INSTALLED) {
            for (a = 0; a < dep_ab_pkg->pkgs->len; a++) {
                pkg_t *dep_pkg = dep_ab_pkg->pkgs->pkgs[a];
                if (dep_pkg->state_status == SS_INSTALLED) {
                    pkg_vec_insert(dependent_pkgs, dep_pkg);
                    count++;
                }
            }
        }
        /* 1 - to keep track of visited ab_pkgs when checking for possiblility
         * of a broken removal of pkgs.
         * 2 - to keep track of pkgs whose deps have been checked alrdy  -
         * Karthik */
    }

    return 0;
}

/*
* Find  packages that were autoinstalled and will be orphaned by the removal of pkg.
*/
int opkg_get_autoinstalled_pkgs(pkg_t *pkg, pkg_vec_t *dependent_pkgs)
{
    int i, j;
    int err = 0;
    int n_deps;
    pkg_t *p;
    struct compound_depend *cdep;

    int count = pkg->pre_depends_count + pkg->depends_count
        + pkg->recommends_count + pkg->suggests_count;

    for (i = 0; i < count; i++) {
        cdep = &pkg->depends[i];
        /* Only remove dependency types which would normally be autoinstalled,
         * namely the types PREDEPEND, DEPEND and RECOMMEND. We don't consider
         * SUGGEST which are not normally autoinstalled and we don't consider
         * CONFLICTS or GREEDY_DEPEND either.
         */
        int uninteresting = cdep->type != PREDEPEND
                && cdep->type != DEPEND
                && cdep->type != RECOMMEND;
        if (uninteresting)
            continue;
        for (j = 0; j < cdep->possibility_count; j++) {
            p = pkg_hash_fetch_installed_by_name(cdep->possibilities[j]->pkg->
                                                 name);

            /* If the package is not installed, this could have
             * been a circular dependency and the package has
             * already been removed.
             */
            if (!p)
                return -1;

            if (!p->auto_installed)
                continue;

            n_deps = pkg_has_installed_dependents(p, NULL);
            if (n_deps == 1) {
                opkg_msg(NOTICE,
                         "%s was autoinstalled and is "
                         "now orphaned, will remove.\n", p->name);
                pkg_vec_insert(dependent_pkgs, p);
            } else
                opkg_msg(INFO,
                         "%s was autoinstalled and is " "still required by %d "
                         "installed packages.\n", p->name, n_deps);
        }
    }

    return err;
}
