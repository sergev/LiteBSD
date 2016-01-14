/* vi: set expandtab sw=4 sts=4: */
/* opkg_solver_internal.h - the opkg package management system

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

#ifndef OPKG_SOLVER_INTERNAL_H
#define OPKG_SOLVER_INTERNAL_H

#include "pkg.h"

#ifdef __cplusplus
extern "C" {
#endif

int pkg_has_installed_dependents(pkg_t *pkg, abstract_pkg_t ***pdependents);
int opkg_get_dependent_pkgs(pkg_t *pkg, abstract_pkg_t **dependents, pkg_vec_t *dependent_pkgs);
int opkg_get_autoinstalled_pkgs(pkg_t *pkg, pkg_vec_t *dependent_pkgs);

#ifdef __cplusplus
}
#endif
#endif    /* OPKG_SOLVER_INTERNAL_H */
