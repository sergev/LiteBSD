/* vi: set expandtab sw=4 sts=4: */
/* file_util.c - convenience routines for common stat operations

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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#include <string.h>

#include "opkg_message.h"
#include "opkg_archive.h"
#include "sprintf_alloc.h"
#include "file_util.h"
#include "md5.h"
#include "xfuncs.h"

#if defined HAVE_SHA256
#include "sha256.h"
#endif

int file_exists(const char *file_name)
{
    struct stat st;
    int r;

    r = stat(file_name, &st);
    if (r == -1)
        return 0;

    return 1;
}

int file_is_dir(const char *file_name)
{
    struct stat st;
    int r;

    r = stat(file_name, &st);
    if (r == -1)
        return 0;

    return S_ISDIR(st.st_mode);
}

int file_is_symlink(const char *file_name)
{
    struct stat st;
    int r;

    r = lstat(file_name, &st);
    if (r == -1)
        return 0;

    return S_ISLNK(st.st_mode);
}

/* read a single line from a file, stopping at a newline or EOF.
   If a newline is read, it will appear in the resulting string.
   Return value is a malloc'ed char * which should be freed at
   some point by the caller.

   Return value is NULL if the file is at EOF when called.
*/
char *file_read_line_alloc(FILE * fp)
{
    char buf[BUFSIZ];
    unsigned int buf_len;
    char *line = NULL;
    unsigned int line_size = 0;
    int got_nl = 0;

    buf[0] = '\0';

    while (fgets(buf, BUFSIZ, fp)) {
        buf_len = strlen(buf);
        if (buf[buf_len - 1] == '\n') {
            buf_len--;
            buf[buf_len] = '\0';
            got_nl = 1;
        }
        if (line) {
            line_size += buf_len;
            line = xrealloc(line, line_size + 1);
            strncat(line, buf, line_size);
        } else {
            line_size = buf_len + 1;
            line = xstrdup(buf);
        }
        if (got_nl)
            break;
    }

    return line;
}

int file_move(const char *src, const char *dest)
{
    int err;

    err = rename(src, dest);
    if (err == -1) {
        if (errno == EXDEV) {
            /* src & dest live on different file systems */
            err = file_copy(src, dest);
            if (err == 0)
                unlink(src);
        } else {
            opkg_perror(ERROR, "Failed to rename %s to %s", src, dest);
        }
    }

    return err;
}

static int copy_file_data(FILE * src_file, FILE * dst_file)
{
    size_t nread, nwritten;
    char buffer[BUFSIZ];

    while (1) {
        nread = fread(buffer, 1, BUFSIZ, src_file);

        if (nread != BUFSIZ && ferror(src_file)) {
            opkg_perror(ERROR, "read");
            return -1;
        }

        /* Check for EOF. */
        if (nread == 0)
            return 0;

        nwritten = fwrite(buffer, 1, nread, dst_file);

        if (nwritten != nread) {
            if (ferror(dst_file))
                opkg_perror(ERROR, "write");
            else
                opkg_msg(ERROR, "Unable to write all data.\n");
            return -1;
        }
    }
}

int file_link(const char *src, const char *dest)
{
    struct stat dest_stat;
    int r;

    r = stat(dest, &dest_stat);
    if (r == 0) {
        r = unlink(dest);
        if (r < 0) {
            opkg_perror(ERROR, "unable to remove `%s'", dest);
            return -1;
        }
    } else if (errno != ENOENT) {
        opkg_perror(ERROR, "unable to stat `%s'", dest);
        return -1;
    }

    r = symlink(src, dest);
    if (r < 0) {
        opkg_perror(DEBUG, "unable to create symlink '%s', falling back to copy",
                    dest);
        return file_copy(src, dest);
    }

    return r;
}

int file_copy(const char *src, const char *dest)
{
    struct stat src_stat;
    struct stat dest_stat;
    int dest_exists = 1;
    int status = 0;
    int r;

    r = stat(src, &src_stat);
    if (r < 0) {
        opkg_perror(ERROR, "%s", src);
        return -1;
    }

    r = stat(dest, &dest_stat);
    if (r < 0) {
        if (errno != ENOENT) {
            opkg_perror(ERROR, "unable to stat `%s'", dest);
            return -1;
        }
        dest_exists = 0;
    } else {
        int is_same_file = (src_stat.st_rdev == dest_stat.st_rdev
                && src_stat.st_ino == dest_stat.st_ino);
        if (is_same_file) {
            opkg_msg(ERROR, "`%s' and `%s' are the same file.\n", src, dest);
            return -1;
        }
    }

    if (S_ISREG(src_stat.st_mode)) {
        FILE *sfp, *dfp;
        struct utimbuf times;

        if (dest_exists) {
            dfp = fopen(dest, "w");
            if (dfp == NULL) {
                r = unlink(dest);
                if (r < 0) {
                    opkg_perror(ERROR, "unable to remove `%s'", dest);
                    return -1;
                }
            }
        } else {
            int fd;

            fd = open(dest, O_WRONLY | O_CREAT, src_stat.st_mode);
            if (fd < 0) {
                opkg_perror(ERROR, "unable to open `%s'", dest);
                return -1;
            }
            dfp = fdopen(fd, "w");
            if (dfp == NULL) {
                if (fd >= 0)
                    close(fd);
                opkg_perror(ERROR, "unable to open `%s'", dest);
                return -1;
            }
        }

        sfp = fopen(src, "r");
        if (sfp) {
            r = copy_file_data(sfp, dfp);
            if (r < 0)
                status = -1;

            r = fclose(sfp);
            if (r < 0) {
                opkg_perror(ERROR, "unable to close `%s'", src);
                status = -1;
            }
        } else {
            opkg_perror(ERROR, "unable to open `%s'", src);
            status = -1;
        }

        r = fclose(dfp);
        if (r < 0) {
            opkg_perror(ERROR, "unable to close `%s'", dest);
            status = -1;
        }

        times.actime = src_stat.st_atime;
        times.modtime = src_stat.st_mtime;
        r = utime(dest, &times);
        if (r < 0)
            opkg_perror(ERROR, "unable to preserve times of `%s'", dest);

        r = chown(dest, src_stat.st_uid, src_stat.st_gid);
        if (r < 0) {
            src_stat.st_mode &= ~(S_ISUID | S_ISGID);
            opkg_perror(ERROR, "unable to preserve ownership of `%s'", dest);
        }

        r = chmod(dest, src_stat.st_mode);
        if (r < 0)
            opkg_perror(ERROR, "unable to preserve permissions of `%s'", dest);

        return status;
    } else if (S_ISBLK(src_stat.st_mode) || S_ISCHR(src_stat.st_mode)
               || S_ISSOCK(src_stat.st_mode)) {
        r = mknod(dest, src_stat.st_mode, src_stat.st_rdev);
        if (r < 0) {
            opkg_perror(ERROR, "unable to create `%s'", dest);
            return -1;
        }
    } else if (S_ISFIFO(src_stat.st_mode)) {
        r = mkfifo(dest, src_stat.st_mode);
        if (r < 0) {
            opkg_perror(ERROR, "cannot create fifo `%s'", dest);
            return -1;
        }
    } else if (S_ISDIR(src_stat.st_mode)) {
        opkg_msg(ERROR, "%s: omitting directory.\n", src);
        return -1;
    }

    opkg_msg(ERROR, "internal error: unrecognized file type.\n");
    return -1;
}

int file_mkdir_hier(const char *path, long mode)
{
    struct stat st;
    int r;

    r = stat(path, &st);
    if (r < 0 && errno == ENOENT) {
        int status;
        char *parent;

        parent = xdirname(path);
        status = file_mkdir_hier(parent, mode | 0300);
        free(parent);

        if (status < 0)
            return -1;

        r = mkdir(path, 0777);
        if (r < 0) {
            opkg_perror(ERROR, "Cannot create directory `%s'", path);
            return -1;
        }

        if (mode != -1) {
            r = chmod(path, mode);
            if (r < 0) {
                opkg_perror(ERROR, "Cannot set permissions of directory `%s'",
                            path);
                return -1;
            }
        }
    }

    return 0;
}

char *file_md5sum_alloc(const char *file_name)
{
    int err;
    FILE *file;
    unsigned char md5sum_bin[16];

    file = fopen(file_name, "r");
    if (file == NULL) {
        opkg_perror(ERROR, "Failed to open file %s", file_name);
        return NULL;
    }

    err = md5_stream(file, md5sum_bin);
    if (err) {
        opkg_msg(ERROR, "Could't compute md5sum for %s.\n", file_name);
        fclose(file);
        return NULL;
    }

    fclose(file);

    return md5_to_string(md5sum_bin);
}

#ifdef HAVE_SHA256
char *file_sha256sum_alloc(const char *file_name)
{
    int err;
    FILE *file;
    unsigned char sha256sum_bin[32];

    file = fopen(file_name, "r");
    if (file == NULL) {
        opkg_perror(ERROR, "Failed to open file %s", file_name);
        return NULL;
    }

    err = sha256_stream(file, sha256sum_bin);
    if (err) {
        opkg_msg(ERROR, "Could't compute sha256sum for %s.\n", file_name);
        fclose(file);
        return NULL;
    }

    fclose(file);

    return sha256_to_string(sha256sum_bin);
}

#endif

int rm_r(const char *path)
{
    int ret = 0;
    DIR *dir;
    struct dirent *dent;
    int r;

    if (path == NULL) {
        opkg_perror(ERROR, "Missing directory parameter");
        return -1;
    }

    dir = opendir(path);
    if (dir == NULL) {
        opkg_perror(ERROR, "Failed to open dir %s", path);
        return -1;
    }

    r = fchdir(dirfd(dir));
    if (r == -1) {
        opkg_perror(ERROR, "Failed to change to dir %s", path);
        closedir(dir);
        return -1;
    }

    while (1) {
        errno = 0;
        dent = readdir(dir);
        if (dent == NULL) {
            if (errno) {
                opkg_perror(ERROR, "Failed to read dir %s", path);
                ret = -1;
            }
            break;
        }

        if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, ".."))
            continue;

#ifdef _BSD_SOURCE
        if (dent->d_type == DT_DIR) {
            ret = rm_r(dent->d_name);
            if (ret == -1)
                break;
            continue;
        } else if (dent->d_type == DT_UNKNOWN)
#endif
        {
            struct stat st;
            ret = lstat(dent->d_name, &st);
            if (ret == -1) {
                opkg_perror(ERROR, "Failed to lstat %s", dent->d_name);
                break;
            }
            if (S_ISDIR(st.st_mode)) {
                ret = rm_r(dent->d_name);
                if (ret == -1)
                    break;
                continue;
            }
        }

        ret = unlink(dent->d_name);
        if (ret == -1) {
            opkg_perror(ERROR, "Failed to unlink %s", dent->d_name);
            break;
        }
    }

    r = chdir("..");
    if (r == -1) {
        ret = -1;
        opkg_perror(ERROR, "Failed to change to dir %s/..", path);
    }

    r = rmdir(path);
    if (r == -1) {
        ret = -1;
        opkg_perror(ERROR, "Failed to remove dir %s", path);
    }

    r = closedir(dir);
    if (r == -1) {
        ret = -1;
        opkg_perror(ERROR, "Failed to close dir %s", path);
    }

    return ret;
}

int file_decompress(const char *in, const char *out)
{
    int r;
    struct opkg_ar *ar;
    FILE *f;

    ar = ar_open_compressed_file(in);
    if (!ar)
        return -1;

    /* Open output file. */
    f = fopen(out, "w");
    if (!f) {
        opkg_msg(ERROR, "Failed to open output file '%s': %s\n", out,
                 strerror(errno));
        r = -1;
        goto cleanup;
    }

    /* Copy decompressed data. */
    r = ar_copy_to_stream(ar, f);
    if (r < 0)
        goto cleanup;

    /* Success */
    r = 0;

 cleanup:
    ar_close(ar);
    if (f)
        fclose(f);

    return r;
}
