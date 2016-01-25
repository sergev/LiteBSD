/* vi: set expandtab sw=4 sts=4: */
/* opkg_conf.c - the opkg package management system

   Copyright (C) 2009 Ubiq Technologies <graham.gower@gmail.com>

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glob.h>
#include <unistd.h>
#include <stdlib.h>

#include "opkg_conf.h"
#include "pkg_vec.h"
#include "pkg.h"
#include "xregex.h"
#include "sprintf_alloc.h"
#include "opkg_message.h"
#include "file_util.h"
#include "xfuncs.h"

#ifndef GLOB_NOMATCH
#define GLOB_NOMATCH (-3) /* from "Error values returned by glob(3)" */
#endif

#ifdef F_TLOCK
#   define TRYLOCK_FILE(fd)  lockf(fd, F_TLOCK, (off_t) 0)
#   define UNLOCK_FILE(fd)   lockf(fd, F_ULOCK, (off_t) 0)
#else
#   define TRYLOCK_FILE(fd)  flock(fd, LOCK_EX | LOCK_NB)
#   define UNLOCK_FILE(fd)   flock(fd, LOCK_UN)
#endif

static int lock_fd;

static opkg_conf_t _conf;
opkg_conf_t *opkg_config = &_conf;

/*
 * Config file options
 */
static opkg_option_t options[] = {
    {"cache_dir", OPKG_OPT_TYPE_STRING, &_conf.cache_dir},
    {"lists_dir", OPKG_OPT_TYPE_STRING, &_conf.lists_dir},
    {"lock_file", OPKG_OPT_TYPE_STRING, &_conf.lock_file},
    {"info_dir", OPKG_OPT_TYPE_STRING, &_conf.info_dir},
    {"status_file", OPKG_OPT_TYPE_STRING, &_conf.status_file},
    {"force_defaults", OPKG_OPT_TYPE_BOOL, &_conf.force_defaults},
    {"force_maintainer", OPKG_OPT_TYPE_BOOL, &_conf.force_maintainer},
    {"ignore_maintainer", OPKG_OPT_TYPE_BOOL, &_conf.ignore_maintainer},
    {"ignore_uid", OPKG_OPT_TYPE_BOOL, &_conf.ignore_uid},
    {"force_depends", OPKG_OPT_TYPE_BOOL, &_conf.force_depends},
    {"force_overwrite", OPKG_OPT_TYPE_BOOL, &_conf.force_overwrite},
    {"force_downgrade", OPKG_OPT_TYPE_BOOL, &_conf.force_downgrade},
    {"force_reinstall", OPKG_OPT_TYPE_BOOL, &_conf.force_reinstall},
    {"force_space", OPKG_OPT_TYPE_BOOL, &_conf.force_space},
    {"force_postinstall", OPKG_OPT_TYPE_BOOL, &_conf.force_postinstall},
    {"autoremove", OPKG_OPT_TYPE_BOOL, &_conf.autoremove},
    {"check_signature", OPKG_OPT_TYPE_BOOL, &_conf.check_signature},
    {"check_pkg_signature", OPKG_OPT_TYPE_BOOL, &_conf.check_pkg_signature},
    {"signature_type", OPKG_OPT_TYPE_STRING, &_conf.signature_type},
    {"ftp_proxy", OPKG_OPT_TYPE_STRING, &_conf.ftp_proxy},
    {"http_proxy", OPKG_OPT_TYPE_STRING, &_conf.http_proxy},
    {"https_proxy", OPKG_OPT_TYPE_STRING, &_conf.https_proxy},
    {"no_proxy", OPKG_OPT_TYPE_STRING, &_conf.no_proxy},
    {"test", OPKG_OPT_TYPE_BOOL, &_conf.noaction},
    {"noaction", OPKG_OPT_TYPE_BOOL, &_conf.noaction},
    {"download_only", OPKG_OPT_TYPE_BOOL, &_conf.download_only},
    {"nodeps", OPKG_OPT_TYPE_BOOL, &_conf.nodeps},
    {"no_install_recommends", OPKG_OPT_TYPE_BOOL, &_conf.no_install_recommends},
    {"offline_root", OPKG_OPT_TYPE_STRING, &_conf.offline_root},
    {"overlay_root", OPKG_OPT_TYPE_STRING, &_conf.overlay_root},
    {"proxy_passwd", OPKG_OPT_TYPE_STRING, &_conf.proxy_passwd},
    {"proxy_user", OPKG_OPT_TYPE_STRING, &_conf.proxy_user},
    {"query-all", OPKG_OPT_TYPE_BOOL, &_conf.query_all},
    {"tmp_dir", OPKG_OPT_TYPE_STRING, &_conf.tmp_dir},
    {"volatile_cache", OPKG_OPT_TYPE_BOOL, &_conf.volatile_cache},
    {"verbosity", OPKG_OPT_TYPE_INT, &_conf.verbosity},
    {"overwrite_no_owner", OPKG_OPT_TYPE_BOOL, &_conf.overwrite_no_owner},
    {"combine", OPKG_OPT_TYPE_BOOL, &_conf.combine},
    {"cache_local_files", OPKG_OPT_TYPE_BOOL, &_conf.cache_local_files},
#if defined(HAVE_OPENSSL)
    {"signature_ca_file", OPKG_OPT_TYPE_STRING, &_conf.signature_ca_file},
    {"signature_ca_path", OPKG_OPT_TYPE_STRING, &_conf.signature_ca_path},
#endif
#if defined(HAVE_PATHFINDER)
    {"check_x509_path", OPKG_OPT_TYPE_BOOL, &_conf.check_x509_path},
#endif
#if defined(HAVE_CURL)
    {"connect_timeout_ms", OPKG_OPT_TYPE_INT, &_conf.connect_timeout_ms},
    {"transfer_timeout_ms", OPKG_OPT_TYPE_INT, &_conf.transfer_timeout_ms},
    {"follow_location", OPKG_OPT_TYPE_BOOL, &_conf.follow_location},
    {"http_auth", OPKG_OPT_TYPE_STRING, &_conf.http_auth},
#endif
#if defined(HAVE_SSLCURL) && defined(HAVE_CURL)
    {"ssl_engine", OPKG_OPT_TYPE_STRING, &_conf.ssl_engine},
    {"ssl_cert", OPKG_OPT_TYPE_STRING, &_conf.ssl_cert},
    {"ssl_cert_type", OPKG_OPT_TYPE_STRING, &_conf.ssl_cert_type},
    {"ssl_key", OPKG_OPT_TYPE_STRING, &_conf.ssl_key},
    {"ssl_key_type", OPKG_OPT_TYPE_STRING, &_conf.ssl_key_type},
    {"ssl_key_passwd", OPKG_OPT_TYPE_STRING, &_conf.ssl_key_passwd},
    {"ssl_ca_file", OPKG_OPT_TYPE_STRING, &_conf.ssl_ca_file},
    {"ssl_ca_path", OPKG_OPT_TYPE_STRING, &_conf.ssl_ca_path},
    {"ssl_dont_verify_peer", OPKG_OPT_TYPE_BOOL, &_conf.ssl_dont_verify_peer},
    {"ftp_explicit_ssl", OPKG_OPT_TYPE_BOOL, &_conf.ftp_explicit_ssl},
#endif
    {NULL, 0, NULL}
};

static int resolve_pkg_dest_list(void)
{
    nv_pair_list_elt_t *iter;
    nv_pair_t *nv_pair;
    pkg_dest_t *dest;
    char *root_dir;

    for (iter = nv_pair_list_first(&opkg_config->tmp_dest_list); iter;
            iter = nv_pair_list_next(&opkg_config->tmp_dest_list, iter)) {
        nv_pair = (nv_pair_t *) iter->data;

        if (opkg_config->offline_root) {
            sprintf_alloc(&root_dir, "%s%s", opkg_config->offline_root,
                          nv_pair->value);
        } else {
            root_dir = xstrdup(nv_pair->value);
        }

        dest = pkg_dest_list_append(&opkg_config->pkg_dest_list, nv_pair->name,
                                    root_dir);
        free(root_dir);

        if (opkg_config->default_dest == NULL)
            opkg_config->default_dest = dest;

        if (opkg_config->dest_str && !strcmp(dest->name, opkg_config->dest_str)) {
            opkg_config->default_dest = dest;
            opkg_config->restrict_to_default_dest = 1;
        }
    }

    if (opkg_config->dest_str && !opkg_config->restrict_to_default_dest) {
        opkg_msg(ERROR, "Unknown dest name: `%s'.\n", opkg_config->dest_str);
        return -1;
    }

    return 0;
}

static opkg_option_t *opkg_conf_find_option(const char *name)
{
    int i;

    for (i = 0; options[i].name; i++) {
        if (strcmp(options[i].name, name) == 0)
            return &options[i];
    }

    /* Not found. */
    return NULL;
}

int opkg_conf_get_option(char *name, void *value)
{
    opkg_option_t *o;

    o = opkg_conf_find_option(name);
    if (o == NULL) {
        /* Not found. */
        opkg_msg(ERROR, "Unrecognized option: %s\n", name);
        *(char **)value = NULL;
        return -1;
    }

    switch (o->type) {
    case OPKG_OPT_TYPE_BOOL:
    case OPKG_OPT_TYPE_INT:
        *(int *)value = *(int *)o->value;
        break;

    case OPKG_OPT_TYPE_STRING:
        *(char **)value = xstrdup(*(char **)o->value);
        break;
    }

    return 0;
}

int opkg_conf_set_option(const char *name, const char *value, int overwrite)
{
    opkg_option_t *o;

    o = opkg_conf_find_option(name);
    if (o == NULL) {
        /* Not found. */
        opkg_msg(ERROR, "Unrecognized option: %s=%s\n", name, value);
        return -1;
    }

    switch (o->type) {
    case OPKG_OPT_TYPE_BOOL:
        if (*(int *)o->value && !overwrite) {
            opkg_msg(ERROR,
                     "Duplicate boolean option %s, "
                     "leaving this option on.\n", name);
            return 0;
        }
        *((int *const)o->value) = 1;
        return 0;

    case OPKG_OPT_TYPE_INT:
        if (!value) {
            opkg_msg(ERROR, "Option %s needs an argument\n", name);
            return -1;
        }

        if (*(int *)o->value && !overwrite) {
            opkg_msg(ERROR,
                     "Duplicate option %s, " "using first seen value \"%d\".\n",
                     name, *((int *)o->value));
            return 0;
        }

        *((int *const)o->value) = atoi(value);
        return 0;

    case OPKG_OPT_TYPE_STRING:
        if (!value) {
            opkg_msg(ERROR, "Option %s needs an argument\n", name);
            return -1;
        }

        if (*(char **)o->value) {
            if (!overwrite) {
                opkg_msg(ERROR,
                         "Duplicate option %s, "
                         "using first seen value \"%s\".\n", name,
                         *((char **)o->value));
                return 0;
            } else {
                /* Let's not leak memory. */
                free(*((char **const)o->value));
            }
        }

        *((char **const)o->value) = xstrdup(value);
        return 0;
    }

    /* The compiler doesn't seem to notice that all cases return, this line
     * should never be reached but it stops a warning.
     */
    return -1;
}

static int opkg_conf_parse_file(const char *filename,
                                pkg_src_list_t * pkg_src_list,
                                pkg_src_list_t * dist_src_list)
{
    int line_num = 0;
    int err = 0;
    int r;
    FILE *file;
    regex_t valid_line_re, comment_re;
#define regmatch_size 14
    regmatch_t regmatch[regmatch_size];

    file = fopen(filename, "r");
    if (file == NULL) {
        opkg_perror(ERROR, "Failed to open %s", filename);
        err = -1;
        goto err0;
    }

    opkg_msg(INFO, "Loading conf file %s.\n", filename);

    err = xregcomp(&comment_re, "^[[:space:]]*(#.*|[[:space:]]*)$",
            REG_EXTENDED);
    if (err)
        goto err1;

    err = xregcomp(&valid_line_re,
                 "^[[:space:]]*(\"([^\"]*)\"|([^[:space:]]*))"
                 "[[:space:]]*(\"([^\"]*)\"|([^[:space:]]*))"
                 "[[:space:]]*(\"([^\"]*)\"|([^[:space:]]*))"
                 "([[:space:]]+([^[:space:]]+))?([[:space:]]+(.*))?[[:space:]]*$",
                 REG_EXTENDED);
    if (err)
        goto err2;

    while (1) {
        char *line;
        char *type, *name, *value, *extra;

        line_num++;

        line = file_read_line_alloc(file);
        if (line == NULL)
            break;

        r = regexec(&comment_re, line, 0, 0, 0);
        if (r == 0)
            goto NEXT_LINE;

        r = regexec(&valid_line_re, line, regmatch_size, regmatch, 0);
        if (r == REG_NOMATCH) {
            opkg_msg(ERROR, "%s:%d: Ignoring invalid line: `%s'\n", filename,
                     line_num, line);
            goto NEXT_LINE;
        }

        /* This has to be so ugly to deal with optional quotation marks */
        if (regmatch[2].rm_so > 0) {
            type = xstrndup(line + regmatch[2].rm_so,
                         regmatch[2].rm_eo - regmatch[2].rm_so);
        } else {
            type = xstrndup(line + regmatch[3].rm_so,
                         regmatch[3].rm_eo - regmatch[3].rm_so);
        }

        if (regmatch[5].rm_so > 0) {
            name = xstrndup(line + regmatch[5].rm_so,
                         regmatch[5].rm_eo - regmatch[5].rm_so);
        } else {
            name = xstrndup(line + regmatch[6].rm_so,
                         regmatch[6].rm_eo - regmatch[6].rm_so);
        }

        if (regmatch[8].rm_so > 0) {
            value = xstrndup(line + regmatch[8].rm_so,
                         regmatch[8].rm_eo - regmatch[8].rm_so);
        } else {
            value = xstrndup(line + regmatch[9].rm_so,
                         regmatch[9].rm_eo - regmatch[9].rm_so);
        }

        extra = NULL;
        if (regmatch[11].rm_so > 0) {
            if (regmatch[13].rm_so > 0 && regmatch[13].rm_so != regmatch[13].rm_eo)
                extra = xstrndup(line + regmatch[11].rm_so,
                             regmatch[13].rm_eo - regmatch[11].rm_so);
            else
                extra = xstrndup(line + regmatch[11].rm_so,
                             regmatch[11].rm_eo - regmatch[11].rm_so);
        }

        if (regmatch[13].rm_so != regmatch[13].rm_eo && strncmp(type, "dist", 4) != 0) {
            opkg_msg(ERROR,
                     "%s:%d: Ignoring config line with trailing garbage: `%s'\n",
                     filename, line_num, line);
        } else {
            /* We use the opkg_config->tmp_dest_list below instead of
             * opkg_config->pkg_dest_list because we might encounter an
             * offline_root option later and that would invalidate the
             * directories we would have computed in
             * pkg_dest_list_init. (We do a similar thing with
             * tmp_src_nv_pair_list for sake of symmetry.) */
            if (strcmp(type, "option") == 0) {
                opkg_conf_set_option(name, value, 0);
            } else if (strcmp(type, "dist") == 0) {
                if (!nv_pair_list_find((nv_pair_list_t *) dist_src_list, name)) {
                    pkg_src_list_append(dist_src_list, name, value, extra, 0);
                } else {
                    opkg_msg(ERROR,
                             "Duplicate dist declaration (%s %s). "
                             "Skipping.\n", name, value);
                }
            } else if (strcmp(type, "dist/gz") == 0) {
                if (!nv_pair_list_find((nv_pair_list_t *) dist_src_list, name)) {
                    pkg_src_list_append(dist_src_list, name, value, extra, 1);
                } else {
                    opkg_msg(ERROR,
                             "Duplicate dist declaration (%s %s). "
                             "Skipping.\n", name, value);
                }
            } else if (strcmp(type, "src") == 0) {
                if (!nv_pair_list_find((nv_pair_list_t *) pkg_src_list, name)) {
                    pkg_src_list_append(pkg_src_list, name, value, extra, 0);
                } else {
                    opkg_msg(ERROR,
                             "Duplicate src declaration (%s %s). "
                             "Skipping.\n", name, value);
                }
            } else if (strcmp(type, "src/gz") == 0) {
                if (!nv_pair_list_find((nv_pair_list_t *) pkg_src_list, name)) {
                    pkg_src_list_append(pkg_src_list, name, value, extra, 1);
                } else {
                    opkg_msg(ERROR,
                             "Duplicate src declaration (%s %s). "
                             "Skipping.\n", name, value);
                }
            } else if (strcmp(type, "dest") == 0) {
                nv_pair_list_append(&opkg_config->tmp_dest_list, name, value);
            } else if (strcmp(type, "arch") == 0) {
                opkg_msg(INFO, "Supported arch %s priority (%s)\n", name,
                         value);
                if (!value) {
                    opkg_msg(NOTICE,
                             "No priority given for architecture %s,"
                             "defaulting to 10\n", name);
                    value = xstrdup("10");
                }
                nv_pair_list_append(&opkg_config->arch_list, name, value);
            } else {
                opkg_msg(ERROR, "%s:%d: Ignoring invalid line: `%s'\n",
                         filename, line_num, line);
            }

        }

        free(type);
        free(name);
        free(value);
        free(extra);

 NEXT_LINE:
        free(line);
    }

    regfree(&valid_line_re);
 err2:
    regfree(&comment_re);
 err1:
    r = fclose(file);
    if (r == EOF) {
        opkg_perror(ERROR, "Couldn't close %s", filename);
        err = -1;
    }
 err0:
    return err;
}

int opkg_conf_write_status_files(void)
{
    pkg_dest_list_elt_t *iter;
    pkg_dest_t *dest;
    pkg_vec_t *all;
    pkg_t *pkg;
    unsigned int i;
    int ret = 0;
    int r;

    if (opkg_config->noaction)
        return 0;

    list_for_each_entry(iter, &opkg_config->pkg_dest_list.head, node) {
        dest = (pkg_dest_t *) iter->data;

        dest->status_fp = fopen(dest->status_file_name, "w");
        if (dest->status_fp == NULL && errno != EROFS) {
            opkg_perror(ERROR, "Can't open status file %s",
                        dest->status_file_name);
            ret = -1;
        }
    }

    all = pkg_vec_alloc();
    pkg_hash_fetch_available(all);

    for (i = 0; i < all->len; i++) {
        pkg = all->pkgs[i];
        /* We don't need most uninstalled packages in the status file */
        int is_not_wanted = (pkg->state_status == SS_NOT_INSTALLED
                && (pkg->state_want == SW_UNKNOWN
                    || (pkg->state_want == SW_DEINSTALL
                        && !(pkg->state_flag & SF_HOLD))
                    || pkg->state_want == SW_PURGE));
        if (is_not_wanted) {
            continue;
        }
        if (pkg->dest == NULL) {
            opkg_msg(ERROR, "Internal error: package %s has a NULL dest\n",
                     pkg->name);
            continue;
        }
        if (pkg->dest->status_fp)
            pkg_print_status(pkg, pkg->dest->status_fp);
    }

    pkg_vec_free(all);

    list_for_each_entry(iter, &opkg_config->pkg_dest_list.head, node) {
        dest = (pkg_dest_t *) iter->data;
        if (dest->status_fp) {
            r = fclose(dest->status_fp);
            if (r == EOF) {
                opkg_perror(ERROR, "Couldn't close %s", dest->status_file_name);
                ret = -1;
            }
        }
    }

    return ret;
}

char *root_filename_alloc(char *filename)
{
    char *root_filename;
    sprintf_alloc(&root_filename, "%s%s",
                  (opkg_config->offline_root ? opkg_config->offline_root : ""),
                  filename);
    return root_filename;
}

static int glob_errfunc(const char *epath, int eerrno)
{
    if (eerrno == ENOENT)
        /* If leading dir does not exist, we get GLOB_NOMATCH. */
        return 0;

    opkg_msg(ERROR, "glob failed for %s: %s\n", epath, strerror(eerrno));
    return 0;
}

int opkg_conf_init(void)
{
    pkg_src_list_init(&opkg_config->pkg_src_list);
    pkg_src_list_init(&opkg_config->dist_src_list);
    pkg_dest_list_init(&opkg_config->pkg_dest_list);
    pkg_dest_list_init(&opkg_config->tmp_dest_list);
    nv_pair_list_init(&opkg_config->arch_list);
    str_list_init(&opkg_config->exclude_list);

    return 0;
}

static int opkg_lock()
{
    int r;
    char *lock_dir;

    /* Ensure that the dir in which the lock file will be created exists. */
    lock_dir = xdirname(opkg_config->lock_file);
    if (!file_exists(lock_dir)) {
        r = file_mkdir_hier(lock_dir, 0755);
        if (r == -1) {
            opkg_perror(ERROR, "Could not create lock file directory %s",
                        lock_dir);
            free(lock_dir);
            return -1;
        }
    }
    free(lock_dir);

    lock_fd = creat(opkg_config->lock_file, S_IRUSR | S_IWUSR | S_IRGRP);
    if (lock_fd == -1) {
        opkg_perror(ERROR, "Could not create lock file %s",
                    opkg_config->lock_file);
        return -1;
    }

    r = TRYLOCK_FILE(lock_fd);
    if (r == -1) {
        opkg_perror(ERROR, "Could not lock %s", opkg_config->lock_file);
        r = close(lock_fd);
        if (r == -1)
            opkg_perror(ERROR, "Couldn't close descriptor %d (%s)", lock_fd,
                        opkg_config->lock_file);
        lock_fd = -1;
        return -1;
    }

    return 0;
}

static int opkg_unlock()
{
    int r;
    int err = 0;

    if (lock_fd != -1) {
        r = UNLOCK_FILE(lock_fd);
        if (r == -1) {
            opkg_perror(ERROR, "Couldn't unlock %s", opkg_config->lock_file);
            err = -1;
        }

        r = close(lock_fd);
        if (r == -1) {
            opkg_perror(ERROR, "Couldn't close descriptor %d (%s)", lock_fd,
                        opkg_config->lock_file);
            err = -1;
        }
    }

    if (opkg_config->lock_file && file_exists(opkg_config->lock_file)) {
        r = unlink(opkg_config->lock_file);
        if (r == -1) {
            opkg_perror(ERROR, "Couldn't unlink %s", opkg_config->lock_file);
            err = -1;
        }
    }

    return err;
}

int opkg_conf_load(void)
{
    unsigned int i;
    int r;
    int glob_ret;
    char *tmp, *tmp_dir_base, **tmp_val;
    glob_t globbuf;
    char *etc_opkg_conf_pattern;

    opkg_config->restrict_to_default_dest = 0;
    opkg_config->default_dest = NULL;
#if defined(HAVE_PATHFINDER)
    opkg_config->check_x509_path = 1;
#endif

    if (!opkg_config->offline_root)
        opkg_config->offline_root = xstrdup(getenv("OFFLINE_ROOT"));

    if (opkg_config->conf_file) {
        struct stat st;
        r = stat(opkg_config->conf_file, &st);
        if (r == -1) {
            opkg_perror(ERROR, "Couldn't stat %s", opkg_config->conf_file);
            goto err0;
        }
        r = opkg_conf_parse_file(opkg_config->conf_file,
                &opkg_config->pkg_src_list, &opkg_config->dist_src_list);
        if (r != 0)
            goto err1;
    } else {
        if (opkg_config->offline_root)
            sprintf_alloc(&etc_opkg_conf_pattern, "%s/etc/opkg/*.conf",
                          opkg_config->offline_root);
        else {
            const char *conf_file_dir = getenv("OPKG_CONF_DIR");
            if (conf_file_dir == NULL)
                conf_file_dir = OPKG_CONF_DEFAULT_CONF_FILE_DIR;
            sprintf_alloc(&etc_opkg_conf_pattern, "%s/*.conf", conf_file_dir);
        }

        memset(&globbuf, 0, sizeof(globbuf));
        glob_ret = glob(etc_opkg_conf_pattern, 0, glob_errfunc, &globbuf);
        if (glob_ret && glob_ret != GLOB_NOMATCH) {
            free(etc_opkg_conf_pattern);
            globfree(&globbuf);
            goto err1;
        }

        free(etc_opkg_conf_pattern);

        for (i = 0; i < globbuf.gl_pathc; i++) {
            int mismatch = globbuf.gl_pathv[i] && opkg_config->conf_file
                    && !strcmp(opkg_config->conf_file, globbuf.gl_pathv[i]);
            if (mismatch)
                continue;
            r = opkg_conf_parse_file (globbuf.gl_pathv[i],
                        &opkg_config->pkg_src_list,
                        &opkg_config->dist_src_list);
            if (r < 0) {
                globfree(&globbuf);
                goto err1;
            }
        }

        globfree(&globbuf);
    }

    if (opkg_config->lock_file == NULL)
        opkg_config->lock_file = xstrdup(OPKG_CONF_DEFAULT_LOCK_FILE);

    if (opkg_config->offline_root) {
        char *tmp;

        sprintf_alloc(&tmp, "%s/%s", opkg_config->offline_root,
                      opkg_config->lock_file);
        free(opkg_config->lock_file);
        opkg_config->lock_file = tmp;
    }

    r = opkg_lock();
    if (r < 0)
        goto err2;

    if (opkg_config->tmp_dir)
        tmp_dir_base = opkg_config->tmp_dir;
    else
        tmp_dir_base = getenv("TMPDIR");

    sprintf_alloc(&tmp, "%s/%s",
                  tmp_dir_base ? tmp_dir_base : OPKG_CONF_DEFAULT_TMP_DIR_BASE,
                  OPKG_CONF_TMP_DIR_SUFFIX);
    free(opkg_config->tmp_dir);
    opkg_config->tmp_dir = mkdtemp(tmp);
    if (opkg_config->tmp_dir == NULL) {
        opkg_perror(ERROR, "Creating temp dir %s failed", tmp);
        goto err3;
    }

    pkg_hash_init();
    hash_table_init("file-hash", &opkg_config->file_hash,
                    OPKG_CONF_DEFAULT_HASH_LEN);
    hash_table_init("obs-file-hash", &opkg_config->obs_file_hash,
                    OPKG_CONF_DEFAULT_HASH_LEN / 16);

    if (opkg_config->lists_dir == NULL)
        opkg_config->lists_dir = xstrdup(OPKG_CONF_DEFAULT_LISTS_DIR);

    if (opkg_config->cache_dir == NULL)
        opkg_config->cache_dir = xstrdup(OPKG_CONF_DEFAULT_CACHE_DIR);

    if (opkg_config->offline_root) {
        sprintf_alloc(&tmp, "%s/%s", opkg_config->offline_root,
                      opkg_config->lists_dir);
        free(opkg_config->lists_dir);
        opkg_config->lists_dir = tmp;

        if (!opkg_config->host_cache_dir) {
            sprintf_alloc(&tmp, "%s/%s", opkg_config->offline_root,
                          opkg_config->cache_dir);
            free(opkg_config->cache_dir);
            opkg_config->cache_dir = tmp;
        }
    }

    if (opkg_config->info_dir == NULL)
        opkg_config->info_dir = xstrdup(OPKG_CONF_DEFAULT_INFO_DIR);

    if (opkg_config->status_file == NULL)
        opkg_config->status_file = xstrdup(OPKG_CONF_DEFAULT_STATUS_FILE);

    if (opkg_config->signature_type == NULL)
        opkg_config->signature_type = xstrdup(OPKG_CONF_DEFAULT_SIGNATURE_TYPE);

    /* if no architectures were defined, then default all, noarch, and host architecture */
    if (nv_pair_list_empty(&opkg_config->arch_list)) {
        nv_pair_list_append(&opkg_config->arch_list, "all", "1");
        nv_pair_list_append(&opkg_config->arch_list, "noarch", "1");
        nv_pair_list_append(&opkg_config->arch_list, HOST_CPU_STR, "10");
    }

    /* Even if there is no conf file, we'll need at least one dest. */
    if (nv_pair_list_empty(&opkg_config->tmp_dest_list)) {
        nv_pair_list_append(&opkg_config->tmp_dest_list,
                            OPKG_CONF_DEFAULT_DEST_NAME,
                            OPKG_CONF_DEFAULT_DEST_ROOT_DIR);
    }

    if (opkg_config->volatile_cache) {
        sprintf_alloc(&tmp, "%s/%s", opkg_config->cache_dir, "volatile");
        free(opkg_config->cache_dir);
        opkg_config->cache_dir = tmp;
    }

    r = file_mkdir_hier(opkg_config->cache_dir, 0755);
    if (r != 0) {
        opkg_perror(ERROR, "Creating cache dir %s failed",
                    opkg_config->cache_dir);
        goto err4;
    }

    r = resolve_pkg_dest_list();
    if (r != 0)
        goto err4;

    nv_pair_list_deinit(&opkg_config->tmp_dest_list);

    return 0;

 err4:
    free(opkg_config->lists_dir);

    pkg_hash_deinit();
    hash_table_deinit(&opkg_config->file_hash);
    hash_table_deinit(&opkg_config->obs_file_hash);

    r = rmdir(opkg_config->tmp_dir);
    if (r == -1)
        opkg_perror(ERROR, "Couldn't remove dir %s", opkg_config->tmp_dir);
 err3:
    opkg_unlock();

 err2:
 err1:
    pkg_src_list_deinit(&opkg_config->pkg_src_list);
    pkg_src_list_deinit(&opkg_config->dist_src_list);
    pkg_dest_list_deinit(&opkg_config->pkg_dest_list);
    nv_pair_list_deinit(&opkg_config->arch_list);

    for (i = 0; options[i].name; i++) {
        if (options[i].type == OPKG_OPT_TYPE_STRING) {
            tmp_val = (char **)options[i].value;
            if (*tmp_val) {
                free(*tmp_val);
                *tmp_val = NULL;
            }
        }
    }
 err0:
    nv_pair_list_deinit(&opkg_config->tmp_dest_list);
    free(opkg_config->dest_str);
    free(opkg_config->conf_file);

    return -1;
}

void opkg_conf_deinit(void)
{
    int i;
    char **tmp;

    if (opkg_config->tmp_dir)
        rm_r(opkg_config->tmp_dir);

    if (opkg_config->volatile_cache)
        rm_r(opkg_config->cache_dir);

    free(opkg_config->dest_str);
    free(opkg_config->conf_file);

    pkg_src_list_deinit(&opkg_config->pkg_src_list);
    pkg_src_list_deinit(&opkg_config->dist_src_list);
    pkg_dest_list_deinit(&opkg_config->pkg_dest_list);
    nv_pair_list_deinit(&opkg_config->arch_list);
    str_list_deinit(&opkg_config->exclude_list);

    if (opkg_config->verbosity >= DEBUG) {
        hash_print_stats(&opkg_config->pkg_hash);
        hash_print_stats(&opkg_config->file_hash);
        hash_print_stats(&opkg_config->obs_file_hash);
    }

    pkg_hash_deinit();
    hash_table_deinit(&opkg_config->file_hash);
    hash_table_deinit(&opkg_config->obs_file_hash);

    opkg_unlock();

    for (i = 0; options[i].name; i++) {
        if (options[i].type == OPKG_OPT_TYPE_STRING) {
            tmp = (char **)options[i].value;
            if (*tmp) {
                free(*tmp);
                *tmp = NULL;
            }
        } else {
            int *val = (int *)options[i].value;
            *val = 0;
        }
    }
}
