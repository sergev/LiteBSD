/* vi: set expandtab sw=4 sts=4: */
/* opkg_solver.h - handle package installation and removal actions
   using an external solver.

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


#ifndef OPKG_SOLVER_H
#define OPKG_SOLVER_H

#ifdef __cplusplus
extern "C" {
#endif

int opkg_solver_install(int num_pkgs, char **pkg_names);
int opkg_solver_remove(int num_pkgs, char **pkg_names);
int opkg_solver_upgrade(int num_pkgs, char **pkg_names);
int opkg_solver_distupgrade(int num_pkgs, char **pkg_names);

#ifdef __cplusplus
}
#endif
#endif    /* OPKG_SOLVER_H */
