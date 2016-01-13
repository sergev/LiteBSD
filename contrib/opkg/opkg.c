/* vi: set expandtab sw=4 sts=4: */
/* opkg.c - the opkg package management system

   Florian Boor
   Copyright (C) 2003 kernel concepts

   Carl D. Worth
   Copyright 2001 University of Southern California

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   opkg command line frontend using libopkg
*/

#include "config.h"

#include <stdio.h>
#include <getopt.h>
#include <malloc.h>
#include <stdlib.h>

#include "opkg_conf.h"
#include "opkg_cmd.h"
#include "file_util.h"
#include "opkg_message.h"
#include "opkg_download.h"
#include "xfuncs.h"

enum {
    ARGS_OPT_FORCE_MAINTAINER = 129,
    ARGS_OPT_IGNORE_MAINTAINER,
    ARGS_OPT_FORCE_DEPENDS,
    ARGS_OPT_FORCE_OVERWRITE,
    ARGS_OPT_FORCE_DOWNGRADE,
    ARGS_OPT_FORCE_REINSTALL,
    ARGS_OPT_FORCE_REMOVAL_OF_DEPENDENT_PACKAGES,
    ARGS_OPT_FORCE_REMOVAL_OF_ESSENTIAL_PACKAGES,
    ARGS_OPT_FORCE_SPACE,
    ARGS_OPT_FORCE_POSTINSTALL,
    ARGS_OPT_FORCE_REMOVE,
    ARGS_OPT_PREFER_ARCH_TO_VERSION,
    ARGS_OPT_ADD_ARCH,
    ARGS_OPT_ADD_DEST,
    ARGS_OPT_ADD_EXCLUDE,
    ARGS_OPT_NOACTION,
    ARGS_OPT_DOWNLOAD_ONLY,
    ARGS_OPT_NODEPS,
    ARGS_OPT_AUTOREMOVE,
    ARGS_OPT_VOLATILE_CACHE,
    ARGS_OPT_COMBINE,
    ARGS_OPT_NO_INSTALL_RECOMMENDS,
    ARGS_OPT_CACHE_DIR,
    ARGS_OPT_HOST_CACHE_DIR,
};

static struct option long_options[] = {
    {"query-all", 0, 0, 'A'},
    {"autoremove", 0, 0, ARGS_OPT_AUTOREMOVE},
    {"conf-file", 1, 0, 'f'},
    {"conf", 1, 0, 'f'},
    {"combine", 0, 0, ARGS_OPT_COMBINE},
    {"dest", 1, 0, 'd'},
    {"force-maintainer", 0, 0, ARGS_OPT_FORCE_MAINTAINER},
    {"force_maintainer", 0, 0, ARGS_OPT_FORCE_MAINTAINER},
    {"ignore-maintainer", 0, 0, ARGS_OPT_IGNORE_MAINTAINER},
    {"ignore_maintainer", 0, 0, ARGS_OPT_IGNORE_MAINTAINER},
    {"force-depends", 0, 0, ARGS_OPT_FORCE_DEPENDS},
    {"force_depends", 0, 0, ARGS_OPT_FORCE_DEPENDS},
    {"force-overwrite", 0, 0, ARGS_OPT_FORCE_OVERWRITE},
    {"force_overwrite", 0, 0, ARGS_OPT_FORCE_OVERWRITE},
    {"force_downgrade", 0, 0, ARGS_OPT_FORCE_DOWNGRADE},
    {"force-downgrade", 0, 0, ARGS_OPT_FORCE_DOWNGRADE},
    {"force-reinstall", 0, 0, ARGS_OPT_FORCE_REINSTALL},
    {"force_reinstall", 0, 0, ARGS_OPT_FORCE_REINSTALL},
    {"force-space", 0, 0, ARGS_OPT_FORCE_SPACE},
    {"force_space", 0, 0, ARGS_OPT_FORCE_SPACE},
    {"recursive", 0, 0, ARGS_OPT_FORCE_REMOVAL_OF_DEPENDENT_PACKAGES},
    {"force-removal-of-dependent-packages", 0, 0,
     ARGS_OPT_FORCE_REMOVAL_OF_DEPENDENT_PACKAGES},
    {"force_removal_of_dependent_packages", 0, 0,
     ARGS_OPT_FORCE_REMOVAL_OF_DEPENDENT_PACKAGES},
    {"force-removal-of-essential-packages", 0, 0,
     ARGS_OPT_FORCE_REMOVAL_OF_ESSENTIAL_PACKAGES},
    {"force_removal_of_essential_packages", 0, 0,
     ARGS_OPT_FORCE_REMOVAL_OF_ESSENTIAL_PACKAGES},
    {"force-postinstall", 0, 0, ARGS_OPT_FORCE_POSTINSTALL},
    {"force_postinstall", 0, 0, ARGS_OPT_FORCE_POSTINSTALL},
    {"force-remove", 0, 0, ARGS_OPT_FORCE_REMOVE},
    {"force_remove", 0, 0, ARGS_OPT_FORCE_REMOVE},
    {"prefer-arch-to-version", 0, 0, ARGS_OPT_PREFER_ARCH_TO_VERSION},
    {"prefer-arch-to-version", 0, 0, ARGS_OPT_PREFER_ARCH_TO_VERSION},
    {"noaction", 0, 0, ARGS_OPT_NOACTION},
    {"download-only", 0, 0, ARGS_OPT_DOWNLOAD_ONLY},
    {"nodeps", 0, 0, ARGS_OPT_NODEPS},
    {"no-install-recommends", 0, 0, ARGS_OPT_NO_INSTALL_RECOMMENDS},
    {"offline", 1, 0, 'o'},
    {"offline-root", 1, 0, 'o'},
    {"add-arch", 1, 0, ARGS_OPT_ADD_ARCH},
    {"add-dest", 1, 0, ARGS_OPT_ADD_DEST},
    {"add-exclude", 1, 0, ARGS_OPT_ADD_EXCLUDE},
    {"test", 0, 0, ARGS_OPT_NOACTION},
    {"tmp-dir", 1, 0, 't'},
    {"tmp_dir", 1, 0, 't'},
    {"cache-dir", 1, 0, ARGS_OPT_CACHE_DIR},
    {"host-cache-dir", 0, 0, ARGS_OPT_HOST_CACHE_DIR},
    {"volatile-cache", 0, 0, ARGS_OPT_VOLATILE_CACHE},
    {"verbosity", 2, 0, 'V'},
    {"version", 0, 0, 'v'},
    {0, 0, 0, 0}
};

static int args_parse(int argc, char *argv[])
{
    int c;
    int option_index = 0;
    int parse_err = 0;
    char *tuple, *targ;

    while (1) {
        c = getopt_long_only(argc, argv, "Ad:f:no:p:t:vV::", long_options,
                             &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'A':
            opkg_config->query_all = 1;
            break;
        case 'd':
            opkg_config->dest_str = xstrdup(optarg);
            break;
        case 'f':
            opkg_config->conf_file = xstrdup(optarg);
            break;
        case 'o':
            opkg_config->offline_root = xstrdup(optarg);
            break;
        case 't':
            opkg_config->tmp_dir = xstrdup(optarg);
            break;
        case 'v':
            printf("opkg version %s\n", VERSION);
            exit(0);
        case 'V':
            opkg_config->verbosity = INFO;
            if (optarg != NULL)
                opkg_config->verbosity = atoi(optarg);
            break;
        case ARGS_OPT_AUTOREMOVE:
            opkg_config->autoremove = 1;
            break;
        case ARGS_OPT_FORCE_MAINTAINER:
            opkg_config->force_maintainer = 1;
            break;
        case ARGS_OPT_IGNORE_MAINTAINER:
            opkg_config->ignore_maintainer = 1;
            break;
        case ARGS_OPT_FORCE_DEPENDS:
            opkg_config->force_depends = 1;
            break;
        case ARGS_OPT_FORCE_OVERWRITE:
            opkg_config->force_overwrite = 1;
            break;
        case ARGS_OPT_FORCE_DOWNGRADE:
            opkg_config->force_downgrade = 1;
            break;
        case ARGS_OPT_FORCE_REINSTALL:
            opkg_config->force_reinstall = 1;
            break;
        case ARGS_OPT_FORCE_REMOVAL_OF_ESSENTIAL_PACKAGES:
            opkg_config->force_removal_of_essential_packages = 1;
            break;
        case ARGS_OPT_FORCE_REMOVAL_OF_DEPENDENT_PACKAGES:
            opkg_config->force_removal_of_dependent_packages = 1;
            break;
        case ARGS_OPT_FORCE_SPACE:
            opkg_config->force_space = 1;
            break;
        case ARGS_OPT_FORCE_POSTINSTALL:
            opkg_config->force_postinstall = 1;
            break;
        case ARGS_OPT_FORCE_REMOVE:
            opkg_config->force_remove = 1;
            break;
        case ARGS_OPT_PREFER_ARCH_TO_VERSION:
            opkg_config->prefer_arch_to_version = 1;
            break;
        case ARGS_OPT_NODEPS:
            opkg_config->nodeps = 1;
            break;
        case ARGS_OPT_ADD_ARCH:
        case ARGS_OPT_ADD_DEST:
            tuple = xstrdup(optarg);
            if ((targ = strchr(tuple, ':')) != NULL) {
                *targ++ = 0;
                if ((strlen(tuple) > 0) && (strlen(targ) > 0)) {
                    nv_pair_list_append((c == ARGS_OPT_ADD_ARCH) ? &opkg_config->arch_list : &opkg_config->tmp_dest_list,
                                        tuple, targ);
                }
            }
            free(tuple);
            break;
        case ARGS_OPT_ADD_EXCLUDE:
            str_list_append(&opkg_config->exclude_list, optarg);
            break;
        case ARGS_OPT_NOACTION:
            opkg_config->noaction = 1;
            break;
        case ARGS_OPT_DOWNLOAD_ONLY:
            opkg_config->download_only = 1;
            break;
        case ARGS_OPT_CACHE_DIR:
            opkg_config->cache_dir = xstrdup(optarg);
            break;
        case ARGS_OPT_HOST_CACHE_DIR:
            opkg_config->host_cache_dir = 1;
            break;
        case ARGS_OPT_VOLATILE_CACHE:
            opkg_config->volatile_cache = 1;
            break;
        case ARGS_OPT_NO_INSTALL_RECOMMENDS:
            opkg_config->no_install_recommends = 1;
            break;
        case ARGS_OPT_COMBINE:
            opkg_config->combine = 1;
            break;
        case ':':
            parse_err = -1;
            break;
        case '?':
            parse_err = -1;
            break;
        default:
            printf("Confusion: getopt_long returned %d\n", c);
        }
    }

    if (parse_err)
        return parse_err;
    else
        return optind;
}

static void usage()
{
    printf("usage: opkg [options...] sub-command [arguments...]\n");
    printf("where sub-command is one of:\n");

    printf("\nPackage Manipulation:\n");
    printf("\tupdate                          Update list of available packages\n");
    printf("\tupgrade                         Upgrade installed packages\n");
    printf("\tinstall <pkgs>                  Install package(s)\n");
    printf("\tconfigure <pkgs>                Configure unpacked package(s)\n");
    printf("\tremove <pkgs|glob>              Remove package(s)\n");
    printf("\tclean                           Clean internal cache\n");
    printf("\tflag <flag> <pkgs>              Flag package(s)\n");
    printf("\t <flag>=hold|noprune|user|ok|installed|unpacked (one per invocation)\n");

    printf("\nInformational Commands:\n");
    printf("\tlist                            List available packages\n");
    printf("\tlist-installed                  List installed packages\n");
    printf("\tlist-upgradable                 List installed and upgradable packages\n");
    printf("\tlist-changed-conffiles          List user modified configuration files\n");
    printf("\tfiles <pkg>                     List files belonging to <pkg>\n");
    printf("\tsearch <file|glob>              List package providing <file>\n");
    printf("\tinfo [pkg|glob]                 Display all info for <pkg>\n");
    printf("\tstatus [pkg|glob]               Display all status for <pkg>\n");
    printf("\tdownload <pkg>                  Download <pkg> to current directory\n");
    printf("\tcompare-versions <v1> <op> <v2>\n");
    printf("\t                                compare versions using <= < > >= = << >>\n");
    printf("\tprint-architecture              List installable package architectures\n");
    printf("\tdepends [-A] [pkgname|glob]+\n");
    printf("\twhatdepends [-A] [pkgname|glob]+\n");
    printf("\twhatdependsrec [-A] [pkgname|glob]+\n");
    printf("\twhatrecommends[-A] [pkgname|glob]+\n");
    printf("\twhatsuggests[-A] [pkgname|glob]+\n");
    printf("\twhatprovides [-A] [pkgname|glob]+\n");
    printf("\twhatconflicts [-A] [pkgname|glob]+\n");
    printf("\twhatreplaces [-A] [pkgname|glob]+\n");

    printf("\nOptions:\n");
    printf("\t-A                              Query all packages not just those installed\n");
    printf("\t-V[<level>]                     Set verbosity level to <level>.\n");
    printf("\t--verbosity[=<level>]           Verbosity levels:\n");
    printf("\t                                  0 errors only\n");
    printf("\t                                  1 normal messages (default)\n");
    printf("\t                                  2 informative messages\n");
    printf("\t                                  3 debug\n");
    printf("\t                                  4 debug level 2\n");
    printf("\t-f <conf_file>                  Use <conf_file> as the opkg configuration file\n");
    printf("\t--conf <conf_file>\n");
    printf("\t-d <dest_name>                  Use <dest_name> as the the root directory for\n");
    printf("\t--dest <dest_name>              package installation, removal, upgrading.\n");
    printf("\t                                <dest_name> should be a defined dest name from\n");
    printf("\t                                the configuration file, (but can also be a\n");
    printf("\t                                directory name in a pinch).\n");
    printf("\t-o <dir>                        Use <dir> as the root directory for\n");
    printf("\t--offline-root <dir>            offline installation of packages.\n");
    printf("\t--add-arch <arch>:<prio>        Register architecture with given priority\n");
    printf("\t--add-dest <name>:<path>        Register destination with given path\n");
    printf("\t--add-exclude <name>            Register package to be excluded from install\n");
    printf("\t--prefer-arch-to-version        Use the architecture priority package rather\n");
    printf("\t                                than the higher version one if more\n");
    printf("\t                                than one candidate is found.\n");
    printf("\t--combine                       Combine upgrade and install operations, this\n");
    printf("\t                                may be needed to resolve dependency issues.\n");

    printf("\nForce Options:\n");
    printf("\t--force-depends                 Install/remove despite failed dependencies\n");
    printf("\t--force-maintainer              Overwrite preexisting config files\n");
    printf("\t--force-reinstall               Reinstall package(s)\n");
    printf("\t--force-overwrite               Overwrite files from other package(s)\n");
    printf("\t--force-downgrade               Allow opkg to downgrade packages\n");
    printf("\t--force-space                   Disable free space checks\n");
    printf("\t--force-postinstall             Run postinstall scripts even in offline mode\n");
    printf("\t--force-remove                  Remove package even if prerm script fails\n");
    printf("\t--noaction                      No action -- test only\n");
    printf("\t--download-only                 No action -- download only\n");
    printf("\t--nodeps                        Do not follow dependencies\n");
    printf("\t--no-install-recommends         Do not install any recommended packages\n");
    printf("\t--force-removal-of-dependent-packages\n");
    printf("\t                                Remove package and all dependencies\n");
    printf("\t--autoremove                    Remove packages that were installed\n");
    printf("\t                                automatically to satisfy dependencies\n");
    printf("\t-t                              Specify tmp-dir.\n");
    printf("\t--tmp-dir                       Specify tmp-dir.\n");
    printf("\t--cache-dir <path>              Specify cache directory.\n");
    printf("\t--host-cache-dir                Don't place cache in offline root dir.\n");
    printf("\t--volatile-cache                Use volatile cache.\n");
    printf("\t                                Volatile cache will be cleared on exit\n");

    printf("\n");

    printf(" glob could be something like 'pkgname*' '*file*' or similar\n");
    printf(" e.g. opkg info 'libstd*' or opkg search '*libop*' or opkg remove 'libncur*'\n");

    /* --force-removal-of-essential-packages        Let opkg remove essential packages.
     * Using this option is almost guaranteed to break your system, hence this option
     * is not even advertised in the usage statement. */

    exit(1);
}

int main(int argc, char *argv[])
{
    int opts, err = -1;
    char *cmd_name = NULL;
    opkg_cmd_t *cmd;
    int nocheckfordirorfile;
    int noreadfeedsfile;

    if (opkg_conf_init())
        goto err0;

    opkg_config->verbosity = NOTICE;

    opts = args_parse(argc, argv);
    if (opts == argc || opts < 0) {
        fprintf(stderr, "opkg must have one sub-command argument\n");
        usage();
    }

    cmd_name = argv[opts++];

    nocheckfordirorfile = !strcmp(cmd_name, "print-architecture")
        || !strcmp(cmd_name, "print_architecture")
        || !strcmp(cmd_name, "print-installation-architecture")
        || !strcmp(cmd_name, "print_installation_architecture");

    noreadfeedsfile = !strcmp(cmd_name, "flag")
        || !strcmp(cmd_name, "configure")
        || !strcmp(cmd_name, "remove")
        || !strcmp(cmd_name, "files")
        || !strcmp(cmd_name, "search")
        || !strcmp(cmd_name, "compare_versions")
        || !strcmp(cmd_name, "compare-versions")
        || !strcmp(cmd_name, "list_installed")
        || !strcmp(cmd_name, "list-installed")
        || !strcmp(cmd_name, "list_changed_conffiles")
        || !strcmp(cmd_name, "list-changed-conffiles")
        || !strcmp(cmd_name, "status");

    cmd = opkg_cmd_find(cmd_name);
    if (cmd == NULL) {
        fprintf(stderr, "%s: unknown sub-command %s\n", argv[0], cmd_name);
        usage();
    }

    opkg_config->pfm = cmd->pfm;

    if (opkg_conf_load())
        goto err0;

    if (!nocheckfordirorfile) {
        if (!noreadfeedsfile) {
            if (pkg_hash_load_feeds())
                goto err1;
        }

        if (pkg_hash_load_status_files())
            goto err1;
    }

    if (cmd->requires_args && opts == argc) {
        fprintf(stderr, "%s: the ``%s'' command requires at least one argument\n",
                argv[0], cmd_name);
        usage();
    }

    err = opkg_cmd_exec(cmd, argc - opts, (const char **)(argv + opts));

    opkg_download_cleanup();
 err1:
    opkg_conf_deinit();

 err0:
    print_error_list();
    free_error_list();

    return err;
}
