/*
 * Copyright (c) 2002 Juli Mallett.  All rights reserved.
 *
 * This software was written by Juli Mallett <jmallett@FreeBSD.org> for the
 * FreeBSD project.  Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef __LIBUFS_H__
#define __LIBUFS_H__

#include <stdio.h>
#include "fs.h"

/*
 * libufs structures.
 */

/*
 * userland ufs disk.
 */
typedef struct {
    const char *d_name;         /* disk name */
    int d_ufs;                  /* decimal UFS version */
    int d_fd;                   /* raw device file descriptor */
    long d_secsize;             /* disk sector size in bytes */
    ufs1_daddr_t d_sblock;      /* superblock location */
    struct csum *d_sbcsum;      /* Superblock summary info */
    union {
        struct fs d_fs;         /* filesystem information */
        char d_sb[MAXBSIZE];
                                /* superblock as buffer */
    } d_sbunion;
    union {
        struct cg d_cg;         /* cylinder group */
        char d_buf[MAXBSIZE];
                                /* cylinder group storage */
    } d_cgunion;
    int d_ccg;                  /* current cylinder group */
    int d_lcg;                  /* last cylinder group (in d_cg) */
    int d_writable;             /* open for write */
    int d_part_type;            /* partition type */
    unsigned d_part_nsectors;   /* partition size in sectors */
    off_t d_part_offset;        /* partition offset in bytes */

#define d_fs    d_sbunion.d_fs
#define d_sb    d_sbunion.d_sb
#define d_cg    d_cgunion.d_cg
} ufs_t;

/*
 * Userland inode.
 */
typedef struct {
    ufs_t           *disk;
    unsigned        number;
    int             dirty;              /* save needed */

    u_int16_t       mode;               /* file type and access mode */
    int16_t         nlink;              /* directory entries */
    u_int32_t       uid;                /* owner */
    u_int32_t       gid;                /* group */
    u_int64_t       size;               /* size */
    int32_t         blocks;             /* blocks actually held */
    ufs1_daddr_t    daddr [NDADDR];     /* direct device addresses constituting file */
    ufs1_daddr_t    iaddr [NIADDR];     /* indirect device addresses */
    u_int32_t       flags;              /* user defined flags */
    int32_t         atime;              /* time last accessed */
    int32_t         mtime;              /* time last modified */
    int32_t         ctime;              /* time created */
} ufs_inode_t;

typedef void (*ufs_directory_scanner_t) (ufs_inode_t *dir,
    ufs_inode_t *file, const char *dirname, const char *filename, void *arg);

/*
 * Inode flags.
 */
#ifndef UF_NODUMP
# define UF_NODUMP      0x00000001
#endif
#ifndef UF_IMMUTABLE
# define UF_IMMUTABLE   0x00000002
#endif
#ifndef UF_APPEND
# define UF_APPEND      0x00000004
#endif
#ifndef UF_NOUNLINK
# define UF_NOUNLINK    0x00000010
#endif
#ifndef SF_ARCHIVED
# define SF_ARCHIVED    0x00010000
#endif
#ifndef SF_IMMUTABLE
# define SF_IMMUTABLE   0x00020000
#endif
#ifndef SF_APPEND
# define SF_APPEND      0x00040000
#endif
#ifndef SF_NOUNLINK
# define SF_NOUNLINK    0x00080000
#endif

__BEGIN_DECLS

/*
 * libufs prototypes.
 */

/*
 * block.c
 */
ssize_t ufs_sector_read(ufs_t *, ufs1_daddr_t, void *, size_t);
ssize_t ufs_sector_write(ufs_t *, ufs1_daddr_t, const void *, size_t);
int     ufs_sector_erase(ufs_t *, ufs1_daddr_t, ufs1_daddr_t);
int     ufs_block_alloc (ufs_t *disk, ufs1_daddr_t bpref, ufs1_daddr_t *bno);
void    ufs_block_free (ufs_t *disk, ufs1_daddr_t bno);

/*
 * cgroup.c
 */
int     ufs_cgroup_read_next(ufs_t *disk);
int     ufs_cgroup_read(ufs_t *disk, int cg);
int     ufs_cgroup_write_last(ufs_t *disk);
int     ufs_cgroup_write(ufs_t *disk, int cg);
void    ufs_print_cg(struct cg *cgr, FILE *out);
ufs1_daddr_t ufs_cgroup_hashalloc(ufs_t *disk, int cg, ufs1_daddr_t pref, int param,
            ufs1_daddr_t (*allocator)());

/*
 * disk.c
 */
int     ufs_disk_close(ufs_t *);
int     ufs_disk_open(ufs_t *, const char *, unsigned);
int     ufs_disk_open_blank(ufs_t *, const char *);
int     ufs_disk_reopen_writable(ufs_t *);
int     ufs_disk_set_partition (ufs_t *, unsigned);

/*
 * inode.c
 */
int     ufs_inode_get (ufs_t *disk, ufs_inode_t *inode, unsigned inum);
void    ufs_inode_print (ufs_inode_t *inode, FILE *out);
void    ufs_inode_print_path (ufs_inode_t *inode, const char *dirname,
            const char *filename, FILE *out);
void    ufs_inode_print_blocks (ufs_inode_t *inode, FILE *out);
int     ufs_inode_read (ufs_inode_t *inode, unsigned long offset,
            unsigned char *data, unsigned long bytes);
int     ufs_inode_write (ufs_inode_t *inode, unsigned long offset,
            unsigned char *data, unsigned long bytes);
void    ufs_directory_scan (ufs_inode_t *inode, const char *dirname,
            ufs_directory_scanner_t scanner, void *arg);
int     ufs_inode_lookup (ufs_t *disk, ufs_inode_t *inode, const char *name);
int     ufs_inode_create (ufs_t *disk, ufs_inode_t *inode, const char *name, int mode);
int     ufs_inode_delete (ufs_t *disk, ufs_inode_t *inode, const char *name);
int     ufs_inode_link (ufs_t *disk, ufs_inode_t *inode, const char *name, int mode, int linktype);
int     ufs_inode_save (ufs_inode_t *inode, int force);
void    ufs_inode_clear (ufs_inode_t *inode);
void    ufs_inode_truncate (ufs_inode_t *inode, unsigned long size);
int     ufs_inode_alloc (ufs_inode_t *dir, int mode, ufs_inode_t *inode);

/*
 * sblock.c
 */
int     ufs_superblock_read(ufs_t *);
int     ufs_superblock_write(ufs_t *, int);
void    ufs_print(ufs_t *disk, FILE *out);

/*
 * bitmap.c
 */
void    ffs_clrblock(struct fs *, u_char *, ufs1_daddr_t);
void    ffs_clusteracct(struct fs *, struct cg *, ufs1_daddr_t, int);
void    ffs_fragacct(struct fs *, int, int32_t [], int);
int     ffs_isblock(struct fs *, u_char *, ufs1_daddr_t);
int     ffs_isfreeblock(struct fs *, u_char *, ufs1_daddr_t);
void    ffs_setblock(struct fs *, u_char *, ufs1_daddr_t);
ufs1_daddr_t ffs_mapsearch(struct fs *fs, struct cg *cgp, ufs1_daddr_t bpref, int allocsiz);

/*
 * check.c
 */
void    ufs_check(ufs_t *disk, const char *filesys, int verbose, int fix);

extern int      check_debug;            /* output debugging info */

/*
 * mount.c
 */
int     ufs_mount(ufs_t *disk, char *dirname);

/*
 * mkfs.c
 */
void    mkfs (ufs_t *disk, const char *fsys);

extern int64_t  mkfs_fssize;		/* file system size in sectors */
extern int64_t  mkfs_mediasize;         /* device size in bytes */
extern int	mkfs_bsize;		/* block size in bytes */
extern int	mkfs_fsize;		/* fragment size in bytes */
extern int	mkfs_sectorsize;	/* bytes/sector */
extern int	mkfs_realsectorsize;	/* bytes/sector in hardware */
extern int      mkfs_nflag;             /* do not create .snap directory */

__END_DECLS

#endif  /* __LIBUFS_H__ */
