/* vi: set expandtab sw=4 sts=4: */
/* opkg_conf.h - the opkg package management system

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

#ifndef OPKG_CONF_H
#define OPKG_CONF_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "hash_table.h"
#include "pkg_src_list.h"
#include "pkg_dest_list.h"
#include "nv_pair_list.h"
#include "str_list.h"

#define OPKG_CONF_DEFAULT_TMP_DIR_BASE "/tmp"
#define OPKG_CONF_TMP_DIR_SUFFIX "opkg-XXXXXX"
#define OPKG_BACKUP_SUFFIX "-opkg.backup"

#define OPKG_CONF_DEFAULT_LISTS_DIR     "/var/lib/opkg/lists"
#define OPKG_CONF_DEFAULT_INFO_DIR      "/var/lib/opkg/info"
#define OPKG_CONF_DEFAULT_STATUS_FILE   "/var/lib/opkg/status"
#define OPKG_CONF_DEFAULT_CACHE_DIR     "/var/cache/opkg"
#define OPKG_CONF_DEFAULT_CONF_FILE_DIR "/etc/opkg"
#define OPKG_CONF_DEFAULT_LOCK_FILE     "/var/run/opkg.lock"

/* In case the config file defines no dest */
#define OPKG_CONF_DEFAULT_DEST_NAME "root"
#define OPKG_CONF_DEFAULT_DEST_ROOT_DIR "/"

#define OPKG_CONF_DEFAULT_HASH_LEN 1024

#define OPKG_CONF_DEFAULT_SIGNATURE_TYPE "gpg"

typedef struct opkg_conf {
    pkg_src_list_t pkg_src_list;
    pkg_src_list_t dist_src_list;
    pkg_dest_list_t pkg_dest_list;
    pkg_dest_list_t tmp_dest_list;
    nv_pair_list_t arch_list;
    str_list_t exclude_list;

    int restrict_to_default_dest;
    pkg_dest_t *default_dest;
    char *dest_str;

    char *conf_file;

    char *tmp_dir;
    char *lists_dir;
    char *cache_dir;
    char *lock_file;
    char *info_dir;
    char *status_file;

    unsigned int pfm;       /* package field mask */

    /* For libopkg users to capture messages. */
    void (*opkg_vmessage) (int, const char *fmt, va_list ap);

    /* options */
    int autoremove;
    int ignore_uid;
    int force_depends;
    int force_defaults;
    int force_maintainer;
    int ignore_maintainer;
    int force_overwrite;
    int force_downgrade;
    int force_reinstall;
    int force_space;
    int force_removal_of_dependent_packages;
    int force_removal_of_essential_packages;
    int force_postinstall;
    int force_remove;
    int prefer_arch_to_version;
    int check_signature;
    int check_pkg_signature;
    char *signature_type;
    int nodeps;             /* do not follow dependencies */
    int no_install_recommends;
    char *offline_root;
    char *overlay_root;
    int query_all;
    int verbosity;
    int noaction;
    int download_only;
    int overwrite_no_owner;
    int volatile_cache;
    int combine;
    int cache_local_files;
    int host_cache_dir;

    /* ssl options: used only when opkg is configured with '--enable-curl',
     * otherwise always NULL or 0.
     */
    int connect_timeout_ms;
    int transfer_timeout_ms;
    int follow_location;

    /* ssl-curl options: used only when opkg is configured with
     * '--enable-ssl-curl', otherwise always NULL or 0.
     *
     * some options could be used by
     * wget if curl support isn't builtin
     * If someone want to try...
     */
    char *ssl_engine;
    char *ssl_cert;
    char *ssl_cert_type;
    char *ssl_key;
    char *ssl_key_type;
    char *ssl_key_passwd;
    char *ssl_ca_file;
    char *ssl_ca_path;
    int ssl_dont_verify_peer;
    int ftp_explicit_ssl;

    /* username and password for curl basic http authentication
     * the format is: [user name]:[password]
     */
    char *http_auth;

    /* pathfinder options: used only when opkg is configured with
     * '--enable-pathfinder', otherwise always NULL or 0 as above.
     */
    int check_x509_path;

    /* proxy options */
    char *http_proxy;
    char *https_proxy;
    char *ftp_proxy;
    char *no_proxy;
    char *proxy_user;
    char *proxy_passwd;

    char *signature_ca_file;
    char *signature_ca_path;

    hash_table_t pkg_hash;
    hash_table_t file_hash;
    hash_table_t obs_file_hash;
} opkg_conf_t;

enum opkg_option_type {
    OPKG_OPT_TYPE_BOOL,
    OPKG_OPT_TYPE_INT,
    OPKG_OPT_TYPE_STRING
};
typedef enum opkg_option_type opkg_option_type_t;

typedef struct opkg_option opkg_option_t;
struct opkg_option {
    const char *name;
    const opkg_option_type_t type;
    void *const value;
};

extern opkg_conf_t *opkg_config;

int opkg_conf_init(void);
int opkg_conf_load(void);
void opkg_conf_deinit(void);

int opkg_conf_write_status_files(void);
char *root_filename_alloc(char *filename);

int opkg_conf_get_option(char *option, void *value);
int opkg_conf_set_option(const char *name, const char *value,
                         int overwrite);

#ifdef __cplusplus
}
#endif
#endif                          /* OPKG_CONF_H */
