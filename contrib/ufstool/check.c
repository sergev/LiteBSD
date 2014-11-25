/*
 * Copyright (c) 1980, 1986, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>

#include "dir.h"
#include "fs.h"
#include "libufs.h"
#include "internal.h"

#define	MAXDUP		10	/* limit on dup blks (per inode) */
#define	MAXBAD		10	/* limit on bad blks (per inode) */
#define	MINBUFS		10	/* minimum number of buffers required */
#define	MAXBUFS		40	/* maximum space to allocate to buffers */
#define	INOBUFSIZE	64*1024	/* size of buffer to read inodes in pass1 */
#define	ZEROBUFSIZE	(dev_bsize * 128) /* size of zero buffer used by -Z */

int check_lfmode = 0700;
off_t check_part_offset;

static struct bufarea asblk;

#define altsblock       (*asblk.b_un.b_fs)
#define POWEROF2(num)   (((num) & ((num) - 1)) == 0)

static struct bufarea cgblk;                        /* backup buffer for cylinder group blocks */
static TAILQ_HEAD(buflist, bufarea) bufhead;        /* head of buffer cache list */
static int numbufs;                                 /* size of buffer cache */
static char *buftype[BT_NUMBUFTYPES] = BT_NAMES;

static ufs_ino_t startinum;

static struct bufarea *cgbufs;  /* header for cylinder group cache */
static int flushtries;          /* number of tries to reclaim memory */

static const char   *lfname = "lost+found";

static struct   dirtemplate emptydir = {
    0, DIRBLKSIZ, DT_UNKNOWN, 0, "",
    0, 0, DT_UNKNOWN, 0, ""
};
static struct   dirtemplate dirhead = {
    0, 12, DT_DIR, 1, ".",
    0, DIRBLKSIZ - 12, DT_DIR, 2, ".."
};

static int pass1check(struct inodesc *idesc);
static int pass4check(struct inodesc *idesc);
static int linkup(ufs_ino_t orphan, ufs_ino_t parentdir, char *name);

/* Inode cache data structures. */
static struct inoinfo **inphead, **inpsort;

static struct bufarea *pdirbp;	/* current directory contents */
static struct bufarea *pbp;	/* current inode block */

static long numdirs, dirhash, listmax, inplast;
static long countdirs;		/* number of directories we actually found */

static long	dev_bsize;	/* computed value of DEV_BSIZE */
static char	havesb;		/* superblock has been read */
static char	*blockmap;	/* ptr to primary blk allocation map */
static ufs_ino_t lfdir;		/* lost & found directory inode number */

/*
 * Calculate a prototype superblock based on information in the disk label.
 * When done the cgsblock macro can be calculated and the fs_ncg field
 * can be used. Do NOT attempt to use other macros without verifying that
 * their needed information is available!
 */
static int
calcsb(int part_num, int devfd, struct fs *fs)
{
    int i, nspf;
    struct stat statb;
    unsigned disk_nsectors, disk_ntracks, disk_secpercyl;
    unsigned part_size, part_fsize, part_frag, part_cpg;

    /* Get disk parameters. */
    fstat(devfd, &statb);
    disk_nsectors = statb.st_size / DEV_BSIZE;
    disk_ntracks = 1;
    disk_secpercyl = 2048;  /* 1 Mbyte per cylinder */

    if (part_num == 0) {
        /* Use whole disk. */
        part_size = disk_nsectors;
    } else {
        /* Get a partition. */
        part_size = 0; // TODO: get size from partition table
    }
    part_fsize = DEV_BSIZE;
    part_frag = 1;
    part_cpg = 204800;  /* 100 Mbytes per group */

    memset(fs, 0, sizeof(struct fs));
    fs->fs_fsize = part_fsize;
    fs->fs_frag = part_frag;
    fs->fs_size = part_size;
    fs->fs_sblkno = 0;

    nspf = fs->fs_fsize / DEV_BSIZE;
    for (fs->fs_fsbtodb = 0, i = nspf; i > 1; i >>= 1)
        fs->fs_fsbtodb++;

    dev_bsize = DEV_BSIZE;
    if (fs->fs_magic == FS_UFS2_MAGIC) {
        /* Never used? */
        fs->fs_fpg = part_cpg;
        fs->fs_ncg = howmany(fs->fs_size, fs->fs_fpg);
    } else /* if (fs->fs_magic == FS_UFS1_MAGIC) */ {
        fs->fs_old_cpg = part_cpg;
        fs->fs_old_cgmask = 0xffffffff;
        for (i = disk_ntracks; i > 1; i >>= 1)
            fs->fs_old_cgmask <<= 1;
        if (!POWEROF2(disk_ntracks))
            fs->fs_old_cgmask <<= 1;
        fs->fs_old_cgoffset = roundup(howmany(disk_nsectors, nspf),
            fs->fs_frag);
        fs->fs_fpg = (fs->fs_old_cpg * disk_secpercyl) / nspf;
        fs->fs_ncg = howmany(fs->fs_size / disk_secpercyl,
            fs->fs_old_cpg);
    }
    return (1);
}

/*
 * Malloc buffers and set up cache.
 */
static void
bufinit(void)
{
    struct bufarea *bp;
    long bufcnt, i;
    char *bufp;

    pbp = pdirbp = (struct bufarea *)0;
    bufp = Malloc((unsigned int)check_sblk.b_un.b_fs->fs_bsize);
    if (bufp == 0)
        errx(EEXIT, "cannot allocate buffer pool");
    cgblk.b_un.b_buf = bufp;
    initbarea(&cgblk, BT_CYLGRP);
    TAILQ_INIT(&bufhead);
    bufcnt = MAXBUFS;
    if (bufcnt < MINBUFS)
        bufcnt = MINBUFS;
    for (i = 0; i < bufcnt; i++) {
        bp = (struct bufarea *)Malloc(sizeof(struct bufarea));
        bufp = Malloc((unsigned int)check_sblk.b_un.b_fs->fs_bsize);
        if (bp == NULL || bufp == NULL) {
            if (i >= MINBUFS)
                break;
            errx(EEXIT, "cannot allocate buffer pool");
        }
        bp->b_un.b_buf = bufp;
        TAILQ_INSERT_HEAD(&bufhead, bp, b_list);
        initbarea(bp, BT_UNKNOWN);
    }
    numbufs = i;    /* save number of buffers */
}

/*
 * An unexpected inconsistency occurred.
 * Die if preening or file system is running with soft dependency protocol,
 * otherwise just print message and continue.
 */
void
check_fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (!check_preen) {
        (void)vfprintf(stdout, fmt, ap);
        va_end(ap);
        if (check_usedsoftdep)
            (void)fprintf(stdout,
                "\nUNEXPECTED SOFT UPDATE INCONSISTENCY\n");
        /*
         * Force foreground fsck to clean up inconsistency.
         */
        return;
    }
    if (check_filename == NULL)
        check_filename = strdup("fsck");
    (void)fprintf(stdout, "%s: ", check_filename);
    (void)vfprintf(stdout, fmt, ap);
    (void)fprintf(stdout,
        "\n%s: UNEXPECTED%sINCONSISTENCY; RUN fsck MANUALLY.\n",
        check_filename, check_usedsoftdep ? " SOFT UPDATE " : " ");
    /*
     * Force foreground fsck to clean up inconsistency.
     */
    check_finish(0);
    exit(EEXIT);
}

/*
 * Pwarn just prints a message when not preening or running soft dependency
 * protocol, or a warning (preceded by filename) when preening.
 */
void
check_warn(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (check_preen)
        (void)fprintf(stdout, "%s: ", check_filename);
    (void)vfprintf(stdout, fmt, ap);
    va_end(ap);
}

/*
 * Stub for routines from kernel.
 */
static void
rwerror(const char *mesg, ufs2_daddr_t blk)
{
    if (check_preen == 0)
        printf("\n");
    check_fatal("CANNOT %s: %ld\n", mesg, (long)blk);
    if (check_reply("CONTINUE") == 0)
        exit(EEXIT);
}

void
check_blwrite(int fd, char *buf, ufs2_daddr_t blk, ssize_t size)
{
    int i;
    char *cp;
    int64_t offset;

    if (fd < 0)
        return;
    offset = blk;
    offset *= dev_bsize;
    offset += check_part_offset;
    if (lseek(fd, offset, 0) < 0)
        rwerror("SEEK BLK", blk);
    else if (write(fd, buf, size) == size) {
        check_fsmodified = 1;
        return;
    }
    check_resolved = 0;
    rwerror("WRITE BLK", blk);
    if (lseek(fd, offset, 0) < 0)
        rwerror("SEEK BLK", blk);
    printf("THE FOLLOWING SECTORS COULD NOT BE WRITTEN:");
    for (cp = buf, i = 0; i < size; i += dev_bsize, cp += dev_bsize)
        if (write(fd, cp, dev_bsize) != dev_bsize) {
            (void)lseek(fd, offset + i + dev_bsize, 0);
            printf(" %jd,", (intmax_t)blk + i / dev_bsize);
        }
    printf("\n");
    return;
}

static void
flush(int fd, struct bufarea *bp)
{
    int i, j;

    if (!bp->b_dirty)
        return;
    bp->b_dirty = 0;
    if (check_fswritefd < 0) {
        check_fatal("WRITING IN READ_ONLY MODE.\n");
        return;
    }
    if (bp->b_errs != 0)
        check_fatal("WRITING %sZERO'ED BLOCK %jd TO DISK\n",
            (bp->b_errs == bp->b_size / dev_bsize) ? "" : "PARTIALLY ",
            (intmax_t)bp->b_bno);
    bp->b_errs = 0;
    check_blwrite(fd, bp->b_un.b_buf, bp->b_bno, bp->b_size);
    if (bp != &check_sblk)
        return;
    for (i = 0, j = 0; i < check_sblk.b_un.b_fs->fs_cssize; i += check_sblk.b_un.b_fs->fs_bsize, j++) {
        check_blwrite(check_fswritefd, (char *)check_sblk.b_un.b_fs->fs_csp + i,
            fsbtodb(check_sblk.b_un.b_fs, check_sblk.b_un.b_fs->fs_csaddr + j * check_sblk.b_un.b_fs->fs_frag),
            check_sblk.b_un.b_fs->fs_cssize - i < check_sblk.b_un.b_fs->fs_bsize ?
            check_sblk.b_un.b_fs->fs_cssize - i : check_sblk.b_un.b_fs->fs_bsize);
    }
}

void
check_finish(int markclean)
{
    struct bufarea *bp, *nbp;
    int ofsmodified, cnt;

    if (check_fswritefd < 0) {
        (void)close(check_fsreadfd);
        return;
    }
    flush(check_fswritefd, &check_sblk);
    if (havesb && check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC &&
        check_sblk.b_bno != check_sblk.b_un.b_fs->fs_sblockloc / dev_bsize &&
        !check_preen && check_reply("UPDATE STANDARD SUPERBLOCK")) {
        check_sblk.b_bno = check_sblk.b_un.b_fs->fs_sblockloc / dev_bsize;
        dirty(&check_sblk);
        flush(check_fswritefd, &check_sblk);
    }
    flush(check_fswritefd, &cgblk);
    free(cgblk.b_un.b_buf);
    cnt = 0;
    TAILQ_FOREACH_REVERSE_SAFE(bp, &bufhead, buflist, b_list, nbp) {
        TAILQ_REMOVE(&bufhead, bp, b_list);
        cnt++;
        flush(check_fswritefd, bp);
        free(bp->b_un.b_buf);
        free((char *)bp);
    }
    if (numbufs != cnt)
        errx(EEXIT, "panic: lost %d buffers", numbufs - cnt);
    for (cnt = 0; cnt < check_sblk.b_un.b_fs->fs_ncg; cnt++) {
        if (cgbufs[cnt].b_un.b_cg == NULL)
            continue;
        flush(check_fswritefd, &cgbufs[cnt]);
        free(cgbufs[cnt].b_un.b_cg);
    }
    free(cgbufs);
    pbp = pdirbp = (struct bufarea *)0;
    if (check_sblk.b_un.b_fs->fs_clean != markclean) {
        if ((check_sblk.b_un.b_fs->fs_clean = markclean) != 0) {
            check_sblk.b_un.b_fs->fs_flags &= ~(FS_UNCLEAN | FS_NEEDSFSCK);
            check_sblk.b_un.b_fs->fs_pendingblocks = 0;
            check_sblk.b_un.b_fs->fs_pendinginodes = 0;
        }
        dirty(&check_sblk);
        ofsmodified = check_fsmodified;
        flush(check_fswritefd, &check_sblk);
        check_fsmodified = ofsmodified;
        if (!check_preen) {
            printf("\n***** FILE SYSTEM MARKED %s *****\n",
                markclean ? "CLEAN" : "DIRTY");
            if (!markclean)
                check_rerun = 1;
        }
    } else if (!check_preen) {
        if (markclean) {
            printf("\n***** FILE SYSTEM IS CLEAN *****\n");
        } else {
            printf("\n***** FILE SYSTEM STILL DIRTY *****\n");
            check_rerun = 1;
        }
    }
    (void)close(check_fsreadfd);
    (void)close(check_fswritefd);
}

/*
 * Read in a superblock finding an alternate if necessary.
 * Return 1 if successful, 0 if unsuccessful, -1 if file system
 * is already clean (check_clean and preen mode only).
 */
int
check_setup(const char *dev, int part_num)
{
    long cg, asked, i, j;
    long bmapsize;
    struct stat statb;
    struct fs proto;
    size_t size;

    havesb = 0;
    check_fswritefd = -1;
    if (stat(dev, &statb) < 0) {
        printf("Can't stat %s: %s\n", dev, strerror(errno));
        return (0);
    }
    if ((statb.st_mode & S_IFMT) != S_IFREG) {
        check_fatal("%s is not a regular file\n", dev);
        if (check_reply("CONTINUE") == 0) {
            return (0);
        }
    }
    if ((check_fsreadfd = open(dev, O_RDONLY)) < 0) {
        printf("Can't open %s: %s\n", dev, strerror(errno));
        return (0);
    }
    if (check_preen == 0)
        printf("** %s", dev);
    if (check_nflag || (check_fswritefd = open(dev, O_WRONLY)) < 0) {
        check_fswritefd = -1;
        if (check_preen)
            check_fatal("NO WRITE ACCESS\n");
        printf(" (NO WRITE)");
    }
    if (check_preen == 0)
        printf("\n");
    /*
     * Read in the superblock, looking for alternates if necessary
     */
    if (check_readsb(1) == 0) {
        check_skipclean = 0;
        if (check_bflag || check_preen || calcsb(part_num, check_fsreadfd, &proto) == 0)
            return(0);
        if (check_reply("LOOK FOR ALTERNATE SUPERBLOCKS") == 0)
            return (0);
        for (cg = 0; cg < proto.fs_ncg; cg++) {
            check_bflag = fsbtodb(&proto, cgsblock(&proto, cg));
            if (check_readsb(0) != 0)
                break;
        }
        if (cg >= proto.fs_ncg) {
            printf("%s %s\n%s %s\n%s %s\n",
                "SEARCH FOR ALTERNATE SUPER-BLOCK",
                "FAILED. YOU MUST USE THE",
                "-b OPTION TO FSCK TO SPECIFY THE",
                "LOCATION OF AN ALTERNATE",
                "SUPER-BLOCK TO SUPPLY NEEDED",
                "INFORMATION; SEE fsck_ffs(8).");
            check_bflag = 0;
            return(0);
        }
        check_warn("USING ALTERNATE SUPERBLOCK AT %d\n", check_bflag);
        check_bflag = 0;
    }
    if (check_skipclean && check_clean && check_sblk.b_un.b_fs->fs_clean) {
        check_warn("FILE SYSTEM CLEAN; SKIPPING CHECKS\n");
        return (-1);
    }
    check_maxfsblock = check_sblk.b_un.b_fs->fs_size;
    check_maxino = check_sblk.b_un.b_fs->fs_ncg * check_sblk.b_un.b_fs->fs_ipg;

    /*
     * Check and potentially fix certain fields in the super block.
     */
    if (check_sblk.b_un.b_fs->fs_optim != FS_OPTTIME && check_sblk.b_un.b_fs->fs_optim != FS_OPTSPACE) {
        check_fatal("UNDEFINED OPTIMIZATION IN SUPERBLOCK\n");
        if (check_reply("SET TO DEFAULT") == 1) {
            check_sblk.b_un.b_fs->fs_optim = FS_OPTTIME;
            dirty(&check_sblk);
        }
    }
    if ((check_sblk.b_un.b_fs->fs_minfree < 0 || check_sblk.b_un.b_fs->fs_minfree > 99)) {
        check_fatal("IMPOSSIBLE MINFREE=%d IN SUPERBLOCK\n",
            check_sblk.b_un.b_fs->fs_minfree);
        if (check_reply("SET TO DEFAULT") == 1) {
            check_sblk.b_un.b_fs->fs_minfree = 10;
            dirty(&check_sblk);
        }
    }
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC &&
        check_sblk.b_un.b_fs->fs_old_inodefmt < FS_44INODEFMT) {
        check_warn("Format of file system is too old.\n");
        check_warn("Must update to modern format using a version of fsck\n");
        check_fatal("from before 2002 with the command ``fsck -c 2''\n");
        exit(EEXIT);
    }
    if (asblk.b_dirty && !check_bflag) {
        memmove(&altsblock, check_sblk.b_un.b_fs, (size_t)check_sblk.b_un.b_fs->fs_sbsize);
        flush(check_fswritefd, &asblk);
    }
    /*
     * read in the summary info.
     */
    asked = 0;
    check_sblk.b_un.b_fs->fs_csp = Calloc(1, check_sblk.b_un.b_fs->fs_cssize);
    if (check_sblk.b_un.b_fs->fs_csp == NULL) {
        printf("cannot alloc %u bytes for cg summary info\n",
            (unsigned)check_sblk.b_un.b_fs->fs_cssize);
        goto badsb;
    }
    for (i = 0, j = 0; i < check_sblk.b_un.b_fs->fs_cssize; i += check_sblk.b_un.b_fs->fs_bsize, j++) {
        size = check_sblk.b_un.b_fs->fs_cssize - i < check_sblk.b_un.b_fs->fs_bsize ?
            check_sblk.b_un.b_fs->fs_cssize - i : check_sblk.b_un.b_fs->fs_bsize;
        if (check_blread(check_fsreadfd, (char *)check_sblk.b_un.b_fs->fs_csp + i,
            fsbtodb(check_sblk.b_un.b_fs, check_sblk.b_un.b_fs->fs_csaddr + j * check_sblk.b_un.b_fs->fs_frag),
            size) != 0 && !asked) {
            check_fatal("BAD SUMMARY INFORMATION\n");
            if (check_reply("CONTINUE") == 0) {
                check_finish(0);
                exit(EEXIT);
            }
            asked++;
        }
    }
    /*
     * allocate and initialize the necessary maps
     */
    bmapsize = roundup(howmany(check_maxfsblock, CHAR_BIT), sizeof(short));
    blockmap = Calloc((unsigned)bmapsize, sizeof (char));
    if (blockmap == NULL) {
        printf("cannot alloc %u bytes for blockmap\n",
            (unsigned)bmapsize);
        goto badsb;
    }
    check_inostathead = Calloc((unsigned)(check_sblk.b_un.b_fs->fs_ncg),
        sizeof(struct inostatlist));
    if (check_inostathead == NULL) {
        printf("cannot alloc %u bytes for inostathead\n",
            (unsigned)(sizeof(struct inostatlist) * (check_sblk.b_un.b_fs->fs_ncg)));
        goto badsb;
    }
    numdirs = MAX(check_sblk.b_un.b_fs->fs_cstotal.cs_ndir, 128);
    dirhash = numdirs;
    inplast = 0;
    listmax = numdirs + 10;
    inpsort = (struct inoinfo **)Calloc((unsigned)listmax,
        sizeof(struct inoinfo *));
    inphead = (struct inoinfo **)Calloc((unsigned)numdirs,
        sizeof(struct inoinfo *));
    if (inpsort == NULL || inphead == NULL) {
        printf("cannot alloc %ju bytes for inphead\n",
            (uintmax_t)numdirs * sizeof(struct inoinfo *));
        goto badsb;
    }
    bufinit();
    if (check_sblk.b_un.b_fs->fs_flags & FS_DOSOFTDEP)
        check_usedsoftdep = 1;
    else
        check_usedsoftdep = 0;
    return (1);

badsb:
    check_finish(0);
    return (0);
}

static void
badsb(int listerr, const char *s)
{

    if (!listerr)
        return;
    if (check_preen)
        printf("%s: ", check_filename);
    check_fatal("BAD SUPER BLOCK: %s\n", s);
}

/*
 * Possible superblock locations ordered from most to least likely.
 */
static const int sblock_try[] = SBLOCKSEARCH;

#define BAD_MAGIC_MSG \
"The previous newfs operation on this volume did not complete.\n" \
"You must complete newfs before mounting this volume.\n"

/*
 * Read in the super block and its summary info.
 */
int
check_readsb(int listerr)
{
    ufs2_daddr_t super;
    int i;

    if (check_bflag) {
        super = check_bflag;
        if ((check_blread(check_fsreadfd, (char *)check_sblk.b_un.b_fs, super, (long)SBLOCKSIZE)))
            return (0);
        if (check_sblk.b_un.b_fs->fs_magic == FS_BAD_MAGIC) {
            fprintf(stderr, BAD_MAGIC_MSG);
            exit(11);
        }
        if (check_sblk.b_un.b_fs->fs_magic != FS_UFS1_MAGIC &&
            check_sblk.b_un.b_fs->fs_magic != FS_UFS2_MAGIC) {
            fprintf(stderr, "%d is not a file system superblock\n",
                check_bflag);
            return (0);
        }
    } else {
        for (i = 0; sblock_try[i] != -1; i++) {
            super = sblock_try[i] / dev_bsize;
            if ((check_blread(check_fsreadfd, (char *)check_sblk.b_un.b_fs, super,
                (long)SBLOCKSIZE)))
                return (0);
            if (check_sblk.b_un.b_fs->fs_magic == FS_BAD_MAGIC) {
                fprintf(stderr, BAD_MAGIC_MSG);
                exit(11);
            }
            if ((check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC ||
                 (check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC &&
                  check_sblk.b_un.b_fs->fs_sblockloc == sblock_try[i])) &&
                check_sblk.b_un.b_fs->fs_ncg >= 1 &&
                check_sblk.b_un.b_fs->fs_bsize >= MINBSIZE &&
                check_sblk.b_un.b_fs->fs_sbsize >= roundup(sizeof(struct fs), dev_bsize))
                break;
        }
        if (sblock_try[i] == -1) {
            fprintf(stderr, "Cannot find file system superblock\n");
            return (0);
        }
    }
    /*
     * Compute block size that the file system is based on,
     * according to fsbtodb, and adjust superblock block number
     * so we can tell if this is an alternate later.
     */
    super *= dev_bsize;
    dev_bsize = check_sblk.b_un.b_fs->fs_fsize / fsbtodb(check_sblk.b_un.b_fs, 1);
    check_sblk.b_bno = super / dev_bsize;
    check_sblk.b_size = SBLOCKSIZE;
    if (check_bflag)
        goto out;
    /*
     * Compare all fields that should not differ in alternate super block.
     * When an alternate super-block is specified this check is skipped.
     */
    check_getblk(&asblk, cgsblock(check_sblk.b_un.b_fs, check_sblk.b_un.b_fs->fs_ncg - 1), check_sblk.b_un.b_fs->fs_sbsize);
    if (asblk.b_errs)
        return (0);
    if (altsblock.fs_sblkno != check_sblk.b_un.b_fs->fs_sblkno ||
        altsblock.fs_cblkno != check_sblk.b_un.b_fs->fs_cblkno ||
        altsblock.fs_iblkno != check_sblk.b_un.b_fs->fs_iblkno ||
        altsblock.fs_dblkno != check_sblk.b_un.b_fs->fs_dblkno ||
        altsblock.fs_ncg != check_sblk.b_un.b_fs->fs_ncg ||
        altsblock.fs_bsize != check_sblk.b_un.b_fs->fs_bsize ||
        altsblock.fs_fsize != check_sblk.b_un.b_fs->fs_fsize ||
        altsblock.fs_frag != check_sblk.b_un.b_fs->fs_frag ||
        altsblock.fs_bmask != check_sblk.b_un.b_fs->fs_bmask ||
        altsblock.fs_fmask != check_sblk.b_un.b_fs->fs_fmask ||
        altsblock.fs_bshift != check_sblk.b_un.b_fs->fs_bshift ||
        altsblock.fs_fshift != check_sblk.b_un.b_fs->fs_fshift ||
        altsblock.fs_fragshift != check_sblk.b_un.b_fs->fs_fragshift ||
        altsblock.fs_fsbtodb != check_sblk.b_un.b_fs->fs_fsbtodb ||
        altsblock.fs_sbsize != check_sblk.b_un.b_fs->fs_sbsize ||
        altsblock.fs_nindir != check_sblk.b_un.b_fs->fs_nindir ||
        altsblock.fs_inopb != check_sblk.b_un.b_fs->fs_inopb ||
        altsblock.fs_cssize != check_sblk.b_un.b_fs->fs_cssize ||
        altsblock.fs_ipg != check_sblk.b_un.b_fs->fs_ipg ||
        altsblock.fs_fpg != check_sblk.b_un.b_fs->fs_fpg ||
        altsblock.fs_magic != check_sblk.b_un.b_fs->fs_magic) {
        badsb(listerr,
        "VALUES IN SUPER BLOCK DISAGREE WITH THOSE IN FIRST ALTERNATE");
        return (0);
    }
out:
    /*
     * If not yet done, update UFS1 superblock with new wider fields.
     */
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC &&
        check_sblk.b_un.b_fs->fs_maxbsize != check_sblk.b_un.b_fs->fs_bsize) {
        check_sblk.b_un.b_fs->fs_maxbsize = check_sblk.b_un.b_fs->fs_bsize;
        check_sblk.b_un.b_fs->fs_time = check_sblk.b_un.b_fs->fs_old_time;
        check_sblk.b_un.b_fs->fs_size = check_sblk.b_un.b_fs->fs_old_size;
        check_sblk.b_un.b_fs->fs_dsize = check_sblk.b_un.b_fs->fs_old_dsize;
        check_sblk.b_un.b_fs->fs_csaddr = check_sblk.b_un.b_fs->fs_old_csaddr;
        check_sblk.b_un.b_fs->fs_cstotal.cs_ndir = check_sblk.b_un.b_fs->fs_old_cstotal.cs_ndir;
        check_sblk.b_un.b_fs->fs_cstotal.cs_nbfree = check_sblk.b_un.b_fs->fs_old_cstotal.cs_nbfree;
        check_sblk.b_un.b_fs->fs_cstotal.cs_nifree = check_sblk.b_un.b_fs->fs_old_cstotal.cs_nifree;
        check_sblk.b_un.b_fs->fs_cstotal.cs_nffree = check_sblk.b_un.b_fs->fs_old_cstotal.cs_nffree;
    }
    havesb = 1;
    return (1);
}

void
check_sblock_init(void)
{
    check_fswritefd = -1;
    check_fsmodified = 0;
    lfdir = 0;
    initbarea(&check_sblk, BT_SUPERBLK);
    initbarea(&asblk, BT_SUPERBLK);
    check_sblk.b_un.b_buf = Malloc(SBLOCKSIZE);
    asblk.b_un.b_buf = Malloc(SBLOCKSIZE);
    if (check_sblk.b_un.b_buf == NULL || asblk.b_un.b_buf == NULL)
        errx(EEXIT, "cannot allocate space for superblock");
    dev_bsize = check_secsize = DEV_BSIZE;
}

static int
ftypeok(union dinode *dp)
{
    switch (DIP(dp, di_mode) & IFMT) {

    case IFDIR:
    case IFREG:
    case IFBLK:
    case IFCHR:
    case IFLNK:
    case IFSOCK:
    case IFIFO:
        return (1);

    default:
        if (check_debug)
            printf("bad file type 0%o\n", DIP(dp, di_mode));
        return (0);
    }
}

int
check_reply(const char *question)
{
    int persevere;
    char c;

    if (check_preen)
        check_fatal("INTERNAL ERROR: GOT TO reply()\n");
    persevere = !strcmp(question, "CONTINUE");
    printf("\n");
    if (!persevere && (check_nflag || check_fswritefd < 0)) {
        printf("%s? no\n\n", question);
        check_resolved = 0;
        return (0);
    }
    if (check_yflag || (persevere && check_nflag)) {
        printf("%s? yes\n\n", question);
        return (1);
    }
    do  {
        printf("%s? [yn] ", question);
        (void) fflush(stdout);
        c = getc(stdin);
        while (c != '\n' && getc(stdin) != '\n') {
            if (feof(stdin)) {
                check_resolved = 0;
                return (0);
            }
        }
    } while (c != 'y' && c != 'Y' && c != 'n' && c != 'N');
    printf("\n");
    if (c == 'y' || c == 'Y')
        return (1);
    check_resolved = 0;
    return (0);
}

/*
 * Look up state information for an inode.
 */
static struct inostat *
inoinfo(ufs_ino_t inum)
{
    static struct inostat unallocated = { USTATE, 0, 0 };
    struct inostatlist *ilp;
    int iloff;

    if (inum > check_maxino)
        errx(EEXIT, "inoinfo: inumber %u out of range", inum);
    ilp = &check_inostathead[inum / check_sblk.b_un.b_fs->fs_ipg];
    iloff = inum % check_sblk.b_un.b_fs->fs_ipg;
    if (iloff >= ilp->il_numalloced)
        return (&unallocated);
    return (&ilp->il_stat[iloff]);
}

/*
 * Manage cylinder group buffers.
 */
struct bufarea *
check_cgget(int cg)
{
    struct bufarea *cgbp;
    struct cg *cgp;

    if (cgbufs == NULL) {
        cgbufs = Calloc(check_sblk.b_un.b_fs->fs_ncg, sizeof(struct bufarea));
        if (cgbufs == NULL)
            errx(EEXIT, "cannot allocate cylinder group buffers");
    }
    cgbp = &cgbufs[cg];
    if (cgbp->b_un.b_cg != NULL)
        return (cgbp);
    cgp = NULL;
    if (flushtries == 0)
        cgp = malloc((unsigned int)check_sblk.b_un.b_fs->fs_cgsize);
    if (cgp == NULL) {
        check_getblk(&cgblk, cgtod(check_sblk.b_un.b_fs, cg), check_sblk.b_un.b_fs->fs_cgsize);
        return (&cgblk);
    }
    cgbp->b_un.b_cg = cgp;
    initbarea(cgbp, BT_CYLGRP);
    check_getblk(cgbp, cgtod(check_sblk.b_un.b_fs, cg), check_sblk.b_un.b_fs->fs_cgsize);
    return (cgbp);
}

/*
 * Attempt to flush a cylinder group cache entry.
 * Return whether the flush was successful.
 */
int
check_flushentry(void)
{
    struct bufarea *cgbp;

    cgbp = &cgbufs[flushtries++];
    if (cgbp->b_un.b_cg == NULL)
        return (0);
    flush(check_fswritefd, cgbp);
    free(cgbp->b_un.b_buf);
    cgbp->b_un.b_buf = NULL;
    return (1);
}

/*
 * Manage a cache of directory blocks.
 */
struct bufarea *
check_getdatablk(ufs2_daddr_t blkno, long size, int type)
{
    struct bufarea *bp;

    TAILQ_FOREACH(bp, &bufhead, b_list)
        if (bp->b_bno == fsbtodb(check_sblk.b_un.b_fs, blkno))
            goto foundit;
    TAILQ_FOREACH_REVERSE(bp, &bufhead, buflist, b_list)
        if ((bp->b_flags & B_INUSE) == 0)
            break;
    if (bp == NULL)
        errx(EEXIT, "deadlocked buffer pool");
    bp->b_type = type;
    check_getblk(bp, blkno, size);
    /* fall through */
foundit:
    if (check_debug && bp->b_type != type)
        printf("Buffer type changed from %s to %s\n",
            buftype[bp->b_type], buftype[type]);
    TAILQ_REMOVE(&bufhead, bp, b_list);
    TAILQ_INSERT_HEAD(&bufhead, bp, b_list);
    bp->b_flags |= B_INUSE;
    return (bp);
}

/*
 * Timespec operations (from <sys/time.h>).
 */
#define timespecsub(vvp, uvp)                       \
    do {                                            \
        (vvp)->tv_sec -= (uvp)->tv_sec;             \
        (vvp)->tv_nsec -= (uvp)->tv_nsec;           \
        if ((vvp)->tv_nsec < 0) {                   \
            (vvp)->tv_sec--;                        \
            (vvp)->tv_nsec += 1000000000;           \
        }                                           \
    } while (0)
#define timespecadd(vvp, uvp)                       \
    do {                                            \
        (vvp)->tv_sec += (uvp)->tv_sec;             \
        (vvp)->tv_nsec += (uvp)->tv_nsec;           \
        if ((vvp)->tv_nsec >= 1000000000) {         \
            (vvp)->tv_sec++;                        \
            (vvp)->tv_nsec -= 1000000000;           \
        }                                           \
    } while (0)

void
check_getblk(struct bufarea *bp, ufs2_daddr_t blk, long size)
{
    ufs2_daddr_t dblk;

    dblk = fsbtodb(check_sblk.b_un.b_fs, blk);
    if (bp->b_bno != dblk) {
        flush(check_fswritefd, bp);
        bp->b_errs = check_blread(check_fsreadfd, bp->b_un.b_buf, dblk, size);
        bp->b_bno = dblk;
        bp->b_size = size;
    }
}

int
check_blread(int fd, char *buf, ufs2_daddr_t blk, long size)
{
    char *cp;
    int i, errs;
    int64_t offset;

    offset = blk;
    offset *= dev_bsize;
    offset += check_part_offset;
//printf("--- %s(blk = %llu) dev_bsize=%lu, part_offset=%llu -> offset=%llu \n", __func__, blk, dev_bsize, check_part_offset, offset);
    if (lseek(fd, offset, 0) < 0)
        rwerror("SEEK BLK", blk);
    else if (read(fd, buf, (int)size) == size) {
        return (0);
    }

    /*
     * This is handled specially here instead of in rwerror because
     * rwerror is used for all sorts of errors, not just true read/write
     * errors.  It should be refactored and fixed.
     */
    if (check_surrender) {
        check_fatal("CANNOT READ_BLK: %ld\n", (long)blk);
        errx(EEXIT, "ABORTING DUE TO READ ERRORS");
    } else
        rwerror("READ BLK", blk);

    if (lseek(fd, offset, 0) < 0)
        rwerror("SEEK BLK", blk);
    errs = 0;
    memset(buf, 0, (size_t)size);
    printf("THE FOLLOWING DISK SECTORS COULD NOT BE READ:");
    for (cp = buf, i = 0; i < size; i += check_secsize, cp += check_secsize) {
        if (read(fd, cp, (int)check_secsize) != check_secsize) {
            (void)lseek(fd, offset + i + check_secsize, 0);
            if (check_secsize != dev_bsize && dev_bsize != 1)
                printf(" %jd (%jd),",
                    (intmax_t)(blk * dev_bsize + i) / check_secsize,
                    (intmax_t)blk + i / dev_bsize);
            else
                printf(" %jd,", (intmax_t)blk + i / dev_bsize);
            errs++;
        }
    }
    printf("\n");
    if (errs)
        check_resolved = 0;
    return (errs);
}

static void
blerase(int fd, ufs2_daddr_t blk, long size)
{
    if (fd < 0)
        return;

#ifdef DIOCGDELETE
    off_t ioarg[2];
    ioarg[0] = blk * dev_bsize;
    ioarg[1] = size;
    ioctl(fd, DIOCGDELETE, ioarg);
    /* we don't really care if we succeed or not */
#endif
}

/*
 * Fill a contiguous region with all-zeroes.  Note ZEROBUFSIZE is by
 * definition a multiple of dev_bsize.
 */
static void
blzero(int fd, ufs2_daddr_t blk, long size)
{
    static char *zero;
    int64_t offset, len;

    if (fd < 0)
        return;
    if (zero == NULL) {
        zero = calloc(ZEROBUFSIZE, 1);
        if (zero == NULL)
            errx(EEXIT, "cannot allocate buffer pool");
    }
    offset = blk * dev_bsize;
    offset += check_part_offset;
    if (lseek(fd, offset, 0) < 0)
        rwerror("SEEK BLK", blk);
    while (size > 0) {
        len = size > ZEROBUFSIZE ? ZEROBUFSIZE : size;
        if (write(fd, zero, len) != len)
            rwerror("WRITE BLK", blk);
        blk += len / dev_bsize;
        size -= len;
    }
}

/*
 * Verify cylinder group's magic number and other parameters.  If the
 * test fails, offer an option to rebuild the whole cylinder group.
 */
static int
check_cgmagic(int cg, struct bufarea *cgbp)
{
    struct cg *cgp = cgbp->b_un.b_cg;

    /*
     * Extended cylinder group checks.
     */
    if (cg_chkmagic(cgp) &&
        ((check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC &&
          cgp->cg_old_niblk == check_sblk.b_un.b_fs->fs_ipg &&
          cgp->cg_ndblk <= check_sblk.b_un.b_fs->fs_fpg &&
          cgp->cg_old_ncyl <= check_sblk.b_un.b_fs->fs_old_cpg) ||
         (check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC &&
          cgp->cg_niblk == check_sblk.b_un.b_fs->fs_ipg &&
          cgp->cg_ndblk <= check_sblk.b_un.b_fs->fs_fpg &&
          cgp->cg_initediblk <= check_sblk.b_un.b_fs->fs_ipg))) {
        return (1);
    }
    check_fatal("CYLINDER GROUP %d: BAD MAGIC NUMBER\n", cg);
    if (!check_reply("REBUILD CYLINDER GROUP")) {
        printf("YOU WILL NEED TO RERUN FSCK.\n");
        check_rerun = 1;
        return (1);
    }
    /*
     * Zero out the cylinder group and then initialize critical fields.
     * Bit maps and summaries will be recalculated by later passes.
     */
    memset(cgp, 0, (size_t)check_sblk.b_un.b_fs->fs_cgsize);
    cgp->cg_magic = CG_MAGIC;
    cgp->cg_cgx = cg;
    cgp->cg_niblk = check_sblk.b_un.b_fs->fs_ipg;
    cgp->cg_initediblk = check_sblk.b_un.b_fs->fs_ipg < 2 * INOPB(check_sblk.b_un.b_fs) ?
        check_sblk.b_un.b_fs->fs_ipg : 2 * INOPB(check_sblk.b_un.b_fs);
    if (cgbase(check_sblk.b_un.b_fs, cg) + check_sblk.b_un.b_fs->fs_fpg < check_sblk.b_un.b_fs->fs_size)
        cgp->cg_ndblk = check_sblk.b_un.b_fs->fs_fpg;
    else
        cgp->cg_ndblk = check_sblk.b_un.b_fs->fs_size - cgbase(check_sblk.b_un.b_fs, cg);
    cgp->cg_iusedoff = &cgp->cg_space[0] - (u_char *)(&cgp->cg_firstfield);
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) {
        cgp->cg_niblk = 0;
        cgp->cg_initediblk = 0;
        cgp->cg_old_ncyl = 0;
        cgp->cg_old_niblk = check_sblk.b_un.b_fs->fs_ipg;
        cgp->cg_old_btotoff = cgp->cg_iusedoff;
        cgp->cg_old_boff = cgp->cg_old_btotoff +
            check_sblk.b_un.b_fs->fs_old_cpg * sizeof(int32_t);
        cgp->cg_iusedoff = cgp->cg_old_boff +
            check_sblk.b_un.b_fs->fs_old_cpg * sizeof(u_int16_t);
    }
    cgp->cg_freeoff = cgp->cg_iusedoff + howmany(check_sblk.b_un.b_fs->fs_ipg, CHAR_BIT);
    cgp->cg_nextfreeoff = cgp->cg_freeoff + howmany(check_sblk.b_un.b_fs->fs_fpg,CHAR_BIT);
    if (check_sblk.b_un.b_fs->fs_contigsumsize > 0) {
        cgp->cg_nclusterblks = cgp->cg_ndblk / check_sblk.b_un.b_fs->fs_frag;
        cgp->cg_clustersumoff =
            roundup(cgp->cg_nextfreeoff, sizeof(u_int32_t));
        cgp->cg_clustersumoff -= sizeof(u_int32_t);
        cgp->cg_clusteroff = cgp->cg_clustersumoff +
            (check_sblk.b_un.b_fs->fs_contigsumsize + 1) * sizeof(u_int32_t);
        cgp->cg_nextfreeoff = cgp->cg_clusteroff +
            howmany(fragstoblks(check_sblk.b_un.b_fs, check_sblk.b_un.b_fs->fs_fpg), CHAR_BIT);
    }
    dirty(cgbp);
    return (0);
}

/*
 * allocate a data block with the specified number of fragments
 */
static ufs2_daddr_t
allocblk(long frags)
{
    int i, j, k, cg, baseblk;
    struct bufarea *cgbp;
    struct cg *cgp;

    if (frags <= 0 || frags > check_sblk.b_un.b_fs->fs_frag)
        return (0);
    for (i = 0; i < check_maxfsblock - check_sblk.b_un.b_fs->fs_frag; i += check_sblk.b_un.b_fs->fs_frag) {
        for (j = 0; j <= check_sblk.b_un.b_fs->fs_frag - frags; j++) {
            if (testbmap(i + j))
                continue;
            for (k = 1; k < frags; k++)
                if (testbmap(i + j + k))
                    break;
            if (k < frags) {
                j += k;
                continue;
            }
            cg = dtog(check_sblk.b_un.b_fs, i + j);
            cgbp = check_cgget(cg);
            cgp = cgbp->b_un.b_cg;
            if (!check_cgmagic(cg, cgbp))
                return (0);
            baseblk = dtogd(check_sblk.b_un.b_fs, i + j);
            for (k = 0; k < frags; k++) {
                setbmap(i + j + k);
                clrbit(cg_blksfree(cgp), baseblk + k);
            }
            check_n_blks += frags;
            if (frags == check_sblk.b_un.b_fs->fs_frag)
                cgp->cg_cs.cs_nbfree--;
            else
                cgp->cg_cs.cs_nffree -= frags;
            dirty(cgbp);
            return (i + j);
        }
    }
    return (0);
}

int
check_findino(struct inodesc *idesc)
{
    struct direct *dirp = idesc->id_dirp;

    if (dirp->d_ino == 0)
        return (KEEPON);
    if (strcmp(dirp->d_name, idesc->id_name) == 0 &&
        dirp->d_ino >= ROOTINO && dirp->d_ino <= check_maxino) {
        idesc->id_parent = dirp->d_ino;
        return (STOP|FOUND);
    }
    return (KEEPON);
}

static int
findname(struct inodesc *idesc)
{
    struct direct *dirp = idesc->id_dirp;

    if (dirp->d_ino != idesc->id_parent || idesc->id_entryno < 2) {
        idesc->id_entryno++;
        return (KEEPON);
    }
    memmove(idesc->id_name, dirp->d_name, (size_t)dirp->d_namlen + 1);
    return (STOP|FOUND);
}

/*
 * Find a pathname
 */
static void
getpathname(char *namebuf, ufs_ino_t curdir, ufs_ino_t ino)
{
    int len;
    char *cp;
    struct inodesc idesc;
    static int busy = 0;

    if (curdir == ino && ino == ROOTINO) {
        (void)strcpy(namebuf, "/");
        return;
    }
    if (busy || !INO_IS_DVALID(curdir)) {
        (void)strcpy(namebuf, "?");
        return;
    }
    busy = 1;
    memset(&idesc, 0, sizeof(struct inodesc));
    idesc.id_type = DATA;
    idesc.id_fix = IGNORE;
    cp = &namebuf[MAXPATHLEN - 1];
    *cp = '\0';
    if (curdir != ino) {
        idesc.id_parent = curdir;
        goto namelookup;
    }
    while (ino != ROOTINO) {
        idesc.id_number = ino;
        idesc.id_func = check_findino;
        idesc.id_name = strdup("..");
        if ((check_inode(check_ginode(ino), &idesc) & FOUND) == 0)
            break;
    namelookup:
        idesc.id_number = idesc.id_parent;
        idesc.id_parent = ino;
        idesc.id_func = findname;
        idesc.id_name = namebuf;
        if ((check_inode(check_ginode(idesc.id_number), &idesc)&FOUND) == 0)
            break;
        len = strlen(namebuf);
        cp -= len;
        memmove(cp, namebuf, (size_t)len);
        *--cp = '/';
        if (cp < &namebuf[MAXNAMLEN])
            break;
        ino = idesc.id_number;
    }
    busy = 0;
    if (ino != ROOTINO)
        *--cp = '?';
    memmove(namebuf, cp, (size_t)(&namebuf[MAXPATHLEN] - cp));
}

void
check_catch(int sig)
{
    check_finish(0);
    exit(12);
}

/*
 * When preening, allow a single quit to signal
 * a special exit after file system checks complete
 * so that reboot sequence may be interrupted.
 */
void
check_catchquit(int sig)
{
    printf("returning to single-user after file system check\n");
    check_returntosingle = 1;
    (void)signal(SIGQUIT, SIG_DFL);
}

static void
pinode(ufs_ino_t ino)
{
    union dinode *dp;
    char *p;
    struct passwd *pw;
    time_t t;

    printf(" I=%lu ", (u_long)ino);
    if (ino < ROOTINO || ino > check_maxino)
        return;
    dp = check_ginode(ino);
    printf(" OWNER=");
    if ((pw = getpwuid((int)DIP(dp, di_uid))) != 0)
        printf("%s ", pw->pw_name);
    else
        printf("%u ", (unsigned)DIP(dp, di_uid));
    printf("MODE=%o\n", DIP(dp, di_mode));
    if (check_preen)
        printf("%s: ", check_filename);
    printf("SIZE=%ju ", (uintmax_t)DIP(dp, di_size));
    t = DIP(dp, di_mtime);
    p = ctime(&t);
    printf("MTIME=%12.12s %4.4s ", &p[4], &p[20]);
}

static void
fileerror(ufs_ino_t cwd, ufs_ino_t ino, const char *errmesg)
{
    union dinode *dp;
    char pathbuf[MAXPATHLEN + 1];

    check_warn("%s ", errmesg);
    pinode(ino);
    printf("\n");
    getpathname(pathbuf, cwd, ino);
    if (ino < ROOTINO || ino > check_maxino) {
        check_fatal("NAME=%s\n", pathbuf);
        return;
    }
    dp = check_ginode(ino);
    if (ftypeok(dp))
        check_fatal("%s=%s\n",
            (DIP(dp, di_mode) & IFMT) == IFDIR ? "DIR" : "FILE",
            pathbuf);
    else
        check_fatal("NAME=%s\n", pathbuf);
}

static void
direrror(ufs_ino_t ino, const char *errmesg)
{
    fileerror(ino, ino, errmesg);
}

/*
 * determine whether an inode should be fixed.
 */
static int
dofix(struct inodesc *idesc, const char *msg)
{

    switch (idesc->id_fix) {

    case DONTKNOW:
        if (idesc->id_type == DATA)
            direrror(idesc->id_number, msg);
        else
            check_warn("%s", msg);
        if (check_preen) {
            printf(" (SALVAGED)\n");
            idesc->id_fix = FIX;
            return (ALTERED);
        }
        if (check_reply("SALVAGE") == 0) {
            idesc->id_fix = NOFIX;
            return (0);
        }
        idesc->id_fix = FIX;
        return (ALTERED);

    case FIX:
        return (ALTERED);

    case NOFIX:
    case IGNORE:
        return (0);

    default:
        errx(EEXIT, "UNKNOWN INODESC FIX MODE %d", idesc->id_fix);
    }
    /* NOTREACHED */
    return (0);
}

/*
 * Get a directory block.
 * Insure that it is held until another is requested.
 */
static struct bufarea *
getdirblk(ufs2_daddr_t blkno, long size)
{
    if (pdirbp != 0)
        pdirbp->b_flags &= ~B_INUSE;
    pdirbp = check_getdatablk(blkno, size, BT_DIRDATA);
    return (pdirbp);
}

/*
 * Verify that a directory entry is valid.
 * This is a superset of the checks made in the kernel.
 */
static int
dircheck(struct inodesc *idesc, struct direct *dp)
{
    size_t size;
    char *cp;
    u_char type;
    u_int8_t namlen;
    int spaceleft;

    spaceleft = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
    if (dp->d_reclen == 0 ||
        dp->d_reclen > spaceleft ||
        (dp->d_reclen & 0x3) != 0)
        goto bad;
    if (dp->d_ino == 0)
        return (1);
    size = DIRSIZ(dp);
    namlen = dp->d_namlen;
    type = dp->d_type;
    if (dp->d_reclen < size ||
        idesc->id_filesize < size ||
        namlen == 0 ||
        type > 15)
        goto bad;
    for (cp = dp->d_name, size = 0; size < namlen; size++)
        if (*cp == '\0' || (*cp++ == '/'))
            goto bad;
    if (*cp != '\0')
        goto bad;
    return (1);
bad:
    if (check_debug)
        printf("Bad dir: ino %d reclen %d namlen %d type %d name %s\n",
            dp->d_ino, dp->d_reclen, dp->d_namlen, dp->d_type,
            dp->d_name);
    return (0);
}

/*
 * get next entry in a directory.
 */
static struct direct *
fsck_readdir(struct inodesc *idesc)
{
    struct direct *dp, *ndp;
    struct bufarea *bp;
    long size, blksiz, fix, dploc;

    blksiz = idesc->id_numfrags * check_sblk.b_un.b_fs->fs_fsize;
    bp = getdirblk(idesc->id_blkno, blksiz);
    if (idesc->id_loc % DIRBLKSIZ == 0 && idesc->id_filesize > 0 &&
        idesc->id_loc < blksiz) {
        dp = (struct direct *)(bp->b_un.b_buf + idesc->id_loc);
        if (dircheck(idesc, dp))
            goto dpok;
        if (idesc->id_fix == IGNORE)
            return (0);
        fix = dofix(idesc, "DIRECTORY CORRUPTED");
        bp = getdirblk(idesc->id_blkno, blksiz);
        dp = (struct direct *)(bp->b_un.b_buf + idesc->id_loc);
        dp->d_reclen = DIRBLKSIZ;
        dp->d_ino = 0;
        dp->d_type = 0;
        dp->d_namlen = 0;
        dp->d_name[0] = '\0';
        if (fix)
            dirty(bp);
        idesc->id_loc += DIRBLKSIZ;
        idesc->id_filesize -= DIRBLKSIZ;
        return (dp);
    }
dpok:
    if (idesc->id_filesize <= 0 || idesc->id_loc >= blksiz)
        return NULL;
    dploc = idesc->id_loc;
    dp = (struct direct *)(bp->b_un.b_buf + dploc);
    idesc->id_loc += dp->d_reclen;
    idesc->id_filesize -= dp->d_reclen;
    if ((idesc->id_loc % DIRBLKSIZ) == 0)
        return (dp);
    ndp = (struct direct *)(bp->b_un.b_buf + idesc->id_loc);
    if (idesc->id_loc < blksiz && idesc->id_filesize > 0 &&
        dircheck(idesc, ndp) == 0) {
        size = DIRBLKSIZ - (idesc->id_loc % DIRBLKSIZ);
        idesc->id_loc += size;
        idesc->id_filesize -= size;
        if (idesc->id_fix == IGNORE)
            return (0);
        fix = dofix(idesc, "DIRECTORY CORRUPTED");
        bp = getdirblk(idesc->id_blkno, blksiz);
        dp = (struct direct *)(bp->b_un.b_buf + dploc);
        dp->d_reclen += size;
        if (fix)
            dirty(bp);
    }
    return (dp);
}

/*
 * Check that a block in a legal block number.
 * Return 0 if in range, 1 if out of range.
 */
static int
chkrange(ufs2_daddr_t blk, int cnt)
{
    int c;

    if (cnt <= 0 || blk <= 0 || blk > check_maxfsblock ||
        cnt - 1 > check_maxfsblock - blk)
        return (1);
    if (cnt > check_sblk.b_un.b_fs->fs_frag ||
        fragnum(check_sblk.b_un.b_fs, blk) + cnt > check_sblk.b_un.b_fs->fs_frag) {
        if (check_debug)
            printf("bad size: blk %ld, offset %i, size %d\n",
                (long)blk, (int)fragnum(check_sblk.b_un.b_fs, blk), cnt);
        return (1);
    }
    c = dtog(check_sblk.b_un.b_fs, blk);
    if (blk < cgdmin(check_sblk.b_un.b_fs, c)) {
        if ((blk + cnt) > cgsblock(check_sblk.b_un.b_fs, c)) {
            if (check_debug) {
                printf("blk %ld < cgdmin %ld;",
                    (long)blk, (long)cgdmin(check_sblk.b_un.b_fs, c));
                printf(" blk + cnt %ld > cgsbase %ld\n",
                    (long)(blk + cnt),
                    (long)cgsblock(check_sblk.b_un.b_fs, c));
            }
            return (1);
        }
    } else {
        if ((blk + cnt) > cgbase(check_sblk.b_un.b_fs, c+1)) {
            if (check_debug)  {
                printf("blk %ld >= cgdmin %ld;",
                    (long)blk, (long)cgdmin(check_sblk.b_un.b_fs, c));
                printf(" blk + cnt %ld > check_sblk.b_un.b_fs->fs_fpg %ld\n",
                    (long)(blk + cnt), (long)check_sblk.b_un.b_fs->fs_fpg);
            }
            return (1);
        }
    }
    return (0);
}

/*
 * Scan each entry in a directory block.
 */
static int
dirscan(struct inodesc *idesc)
{
    struct direct *dp;
    struct bufarea *bp;
    u_int dsize, n;
    long blksiz;
    char dbuf[DIRBLKSIZ];

    if (idesc->id_type != DATA)
        errx(EEXIT, "wrong type to dirscan %d", idesc->id_type);
    if (idesc->id_entryno == 0 &&
        (idesc->id_filesize & (DIRBLKSIZ - 1)) != 0)
        idesc->id_filesize = roundup(idesc->id_filesize, DIRBLKSIZ);
    blksiz = idesc->id_numfrags * check_sblk.b_un.b_fs->fs_fsize;
    if (chkrange(idesc->id_blkno, idesc->id_numfrags)) {
        idesc->id_filesize -= blksiz;
        return (SKIP);
    }
    idesc->id_loc = 0;
    for (dp = fsck_readdir(idesc); dp != NULL; dp = fsck_readdir(idesc)) {
        dsize = dp->d_reclen;
        if (dsize > sizeof(dbuf))
            dsize = sizeof(dbuf);
        memmove(dbuf, dp, (size_t)dsize);
        idesc->id_dirp = (struct direct *)dbuf;
        if ((n = (*idesc->id_func)(idesc)) & ALTERED) {
            bp = getdirblk(idesc->id_blkno, blksiz);
            memmove(bp->b_un.b_buf + idesc->id_loc - dsize, dbuf,
                (size_t)dsize);
            dirty(bp);
            dirty(&check_sblk);
        }
        if (n & STOP)
            return (n);
    }
    return (idesc->id_filesize > 0 ? KEEPON : STOP);
}

void
check_inodirty(void)
{
    dirty(pbp);
}

static int
iblock(struct inodesc *idesc, long ilevel, int64_t isize, int type)
{
    struct bufarea *bp;
    int i, n, (*func)(struct inodesc *), nif;
    int64_t sizepb;
    char buf[BUFSIZ];
    char pathbuf[MAXPATHLEN + 1];
    union dinode *dp;

    if (idesc->id_type != DATA) {
        func = idesc->id_func;
        if (((n = (*func)(idesc)) & KEEPON) == 0)
            return (n);
    } else
        func = dirscan;
    if (chkrange(idesc->id_blkno, idesc->id_numfrags))
        return (SKIP);
    bp = check_getdatablk(idesc->id_blkno, check_sblk.b_un.b_fs->fs_bsize, type);
    ilevel--;
    for (sizepb = check_sblk.b_un.b_fs->fs_bsize, i = 0; i < ilevel; i++)
        sizepb *= NINDIR(check_sblk.b_un.b_fs);
    if (howmany(isize, sizepb) > NINDIR(check_sblk.b_un.b_fs))
        nif = NINDIR(check_sblk.b_un.b_fs);
    else
        nif = howmany(isize, sizepb);
    if (idesc->id_func == pass1check && nif < NINDIR(check_sblk.b_un.b_fs)) {
        for (i = nif; i < NINDIR(check_sblk.b_un.b_fs); i++) {
            if (IBLK(bp, i) == 0)
                continue;
            (void)sprintf(buf, "PARTIALLY TRUNCATED INODE I=%lu",
                (u_long)idesc->id_number);
            if (check_preen) {
                check_fatal("%s\n", buf);
            } else if (dofix(idesc, buf)) {
                IBLK_SET(bp, i, 0);
                dirty(bp);
            }
        }
        flush(check_fswritefd, bp);
    }
    for (i = 0; i < nif; i++) {
        if (ilevel == 0)
            idesc->id_lbn++;
        if (IBLK(bp, i)) {
            idesc->id_blkno = IBLK(bp, i);
            if (ilevel == 0)
                n = (*func)(idesc);
            else
                n = iblock(idesc, ilevel, isize, type);
            if (n & STOP) {
                bp->b_flags &= ~B_INUSE;
                return (n);
            }
        } else {
            if (idesc->id_type == DATA && isize > 0) {
                /* An empty block in a directory XXX */
                getpathname(pathbuf, idesc->id_number,
                        idesc->id_number);
                check_fatal("DIRECTORY %s: CONTAINS EMPTY BLOCKS\n",
                    pathbuf);
                if (check_reply("ADJUST LENGTH") == 1) {
                    dp = check_ginode(idesc->id_number);
                    DIP_SET(dp, di_size,
                        DIP(dp, di_size) - isize);
                    isize = 0;
                    printf(
                        "YOU MUST RERUN FSCK AFTERWARDS\n");
                    check_rerun = 1;
                    check_inodirty();
                    bp->b_flags &= ~B_INUSE;
                    return(STOP);
                }
            }
        }
        isize -= sizepb;
    }
    bp->b_flags &= ~B_INUSE;
    return (KEEPON);
}

int
check_inode(union dinode *dp, struct inodesc *idesc)
{
    int64_t remsize, sizepb;
    int i, offset, ret;
    union dinode dino;
    ufs2_daddr_t ndb;
    mode_t mode;
    char pathbuf[MAXPATHLEN + 1];

    if (idesc->id_fix != IGNORE)
        idesc->id_fix = DONTKNOW;
    idesc->id_lbn = -1;
    idesc->id_entryno = 0;
    idesc->id_filesize = DIP(dp, di_size);
    mode = DIP(dp, di_mode) & IFMT;
    if (mode == IFBLK || mode == IFCHR || (mode == IFLNK &&
        DIP(dp, di_size) < (unsigned)check_sblk.b_un.b_fs->fs_maxsymlinklen))
        return (KEEPON);
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC)
        dino.dp1 = dp->dp1;
    else
        dino.dp2 = dp->dp2;
    ndb = howmany(DIP(&dino, di_size), check_sblk.b_un.b_fs->fs_bsize);
    for (i = 0; i < NDADDR; i++) {
        idesc->id_lbn++;
        if (--ndb == 0 &&
            (offset = blkoff(check_sblk.b_un.b_fs, DIP(&dino, di_size))) != 0)
            idesc->id_numfrags =
                numfrags(check_sblk.b_un.b_fs, fragroundup(check_sblk.b_un.b_fs, offset));
        else
            idesc->id_numfrags = check_sblk.b_un.b_fs->fs_frag;
        if (DIP(&dino, di_db[i]) == 0) {
            if (idesc->id_type == DATA && ndb >= 0) {
                /* An empty block in a directory XXX */
                getpathname(pathbuf, idesc->id_number,
                        idesc->id_number);
                check_fatal("DIRECTORY %s: CONTAINS EMPTY BLOCKS\n",
                    pathbuf);
                if (check_reply("ADJUST LENGTH") == 1) {
                    dp = check_ginode(idesc->id_number);
                    DIP_SET(dp, di_size,
                        i * check_sblk.b_un.b_fs->fs_bsize);
                    printf(
                        "YOU MUST RERUN FSCK AFTERWARDS\n");
                    check_rerun = 1;
                    check_inodirty();

                }
            }
            continue;
        }
        idesc->id_blkno = DIP(&dino, di_db[i]);
        if (idesc->id_type != DATA)
            ret = (*idesc->id_func)(idesc);
        else
            ret = dirscan(idesc);
        if (ret & STOP)
            return (ret);
    }
    idesc->id_numfrags = check_sblk.b_un.b_fs->fs_frag;
    remsize = DIP(&dino, di_size) - check_sblk.b_un.b_fs->fs_bsize * NDADDR;
    sizepb = check_sblk.b_un.b_fs->fs_bsize;
    for (i = 0; i < NIADDR; i++) {
        sizepb *= NINDIR(check_sblk.b_un.b_fs);
        if (DIP(&dino, di_ib[i])) {
            idesc->id_blkno = DIP(&dino, di_ib[i]);
            ret = iblock(idesc, i + 1, remsize, BT_LEVEL1 + i);
            if (ret & STOP)
                return (ret);
        } else {
            idesc->id_lbn += sizepb / check_sblk.b_un.b_fs->fs_bsize;
            if (idesc->id_type == DATA && remsize > 0) {
                /* An empty block in a directory XXX */
                getpathname(pathbuf, idesc->id_number,
                        idesc->id_number);
                check_fatal("DIRECTORY %s: CONTAINS EMPTY BLOCKS\n",
                    pathbuf);
                if (check_reply("ADJUST LENGTH") == 1) {
                    dp = check_ginode(idesc->id_number);
                    DIP_SET(dp, di_size,
                        DIP(dp, di_size) - remsize);
                    remsize = 0;
                    printf(
                        "YOU MUST RERUN FSCK AFTERWARDS\n");
                    check_rerun = 1;
                    check_inodirty();
                    break;
                }
            }
        }
        remsize -= sizepb;
    }
    return (KEEPON);
}

/*
 * General purpose interface for reading inodes.
 */
union dinode *
check_ginode(ufs_ino_t inumber)
{
    ufs2_daddr_t iblk;

    if (inumber < ROOTINO || inumber > check_maxino)
        errx(EEXIT, "bad inode number %u to ginode", inumber);
    if (startinum == 0 ||
        inumber < startinum || inumber >= startinum + INOPB(check_sblk.b_un.b_fs)) {
        iblk = ino_to_fsba(check_sblk.b_un.b_fs, inumber);
        if (pbp != 0)
            pbp->b_flags &= ~B_INUSE;
        pbp = check_getdatablk(iblk, check_sblk.b_un.b_fs->fs_bsize, BT_INODES);
        startinum = (inumber / INOPB(check_sblk.b_un.b_fs)) * INOPB(check_sblk.b_un.b_fs);
    }
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC)
        return ((union dinode *)
            &pbp->b_un.b_dinode1[inumber % INOPB(check_sblk.b_un.b_fs)]);
    return ((union dinode *)&pbp->b_un.b_dinode2[inumber % INOPB(check_sblk.b_un.b_fs)]);
}

/*
 * Special purpose version of ginode used to optimize first pass
 * over all the inodes in numerical order.
 */
static ufs_ino_t nextino, lastinum, lastvalidinum;
static long readcount, readpercg, fullcnt, inobufsize, partialcnt, partialsize;
static struct bufarea inobuf;

static union dinode *
getnextinode(ufs_ino_t inumber, int rebuildcg)
{
    int j;
    long size;
    mode_t mode;
    ufs2_daddr_t ndb, blk;
    union dinode *dp;
    static caddr_t nextinop;

    if (inumber != nextino++ || inumber > lastvalidinum)
        errx(EEXIT, "bad inode number %u to nextinode", inumber);
    if (inumber >= lastinum) {
        readcount++;
        blk = ino_to_fsba(check_sblk.b_un.b_fs, lastinum);
        if (readcount % readpercg == 0) {
            size = partialsize;
            lastinum += partialcnt;
        } else {
            size = inobufsize;
            lastinum += fullcnt;
        }
        /*
         * If getblk encounters an error, it will already have zeroed
         * out the buffer, so we do not need to do so here.
         */
        check_getblk(&inobuf, blk, size);
        nextinop = inobuf.b_un.b_buf;
    }
    dp = (union dinode *)nextinop;
    if (rebuildcg && nextinop == inobuf.b_un.b_buf) {
        /*
         * Try to determine if we have reached the end of the
         * allocated inodes.
         */
        mode = DIP(dp, di_mode) & IFMT;
        if (mode == 0) {
            if (memcmp(dp->dp2.di_db, ufs2_zino.di_db,
                NDADDR * sizeof(ufs2_daddr_t)) ||
                  memcmp(dp->dp2.di_ib, ufs2_zino.di_ib,
                NIADDR * sizeof(ufs2_daddr_t)) ||
                  dp->dp2.di_mode || dp->dp2.di_size)
                return (NULL);
            goto inodegood;
        }
        if (!ftypeok(dp))
            return (NULL);
        ndb = howmany(DIP(dp, di_size), check_sblk.b_un.b_fs->fs_bsize);
        if (ndb < 0)
            return (NULL);
        if (mode == IFBLK || mode == IFCHR)
            ndb++;
        if (mode == IFLNK) {
            /*
             * Fake ndb value so direct/indirect block checks below
             * will detect any garbage after symlink string.
             */
            if (DIP(dp, di_size) < (int64_t)check_sblk.b_un.b_fs->fs_maxsymlinklen) {
                ndb = howmany(DIP(dp, di_size),
                    sizeof(ufs2_daddr_t));
                if (ndb > NDADDR) {
                    j = ndb - NDADDR;
                    for (ndb = 1; j > 1; j--)
                        ndb *= NINDIR(check_sblk.b_un.b_fs);
                    ndb += NDADDR;
                }
            }
        }
        for (j = ndb; ndb < NDADDR && j < NDADDR; j++)
            if (DIP(dp, di_db[j]) != 0)
                return (NULL);
        for (j = 0, ndb -= NDADDR; ndb > 0; j++)
            ndb /= NINDIR(check_sblk.b_un.b_fs);
        for (; j < NIADDR; j++)
            if (DIP(dp, di_ib[j]) != 0)
                return (NULL);
    }
inodegood:
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC)
        nextinop += sizeof(struct ufs1_dinode);
    else
        nextinop += sizeof(struct ufs2_dinode);
    return (dp);
}

static void
setinodebuf(ufs_ino_t inum)
{
    if (inum % check_sblk.b_un.b_fs->fs_ipg != 0)
        errx(EEXIT, "bad inode number %u to setinodebuf", inum);
    lastvalidinum = inum + check_sblk.b_un.b_fs->fs_ipg - 1;
    startinum = 0;
    nextino = inum;
    lastinum = inum;
    readcount = 0;
    if (inobuf.b_un.b_buf != NULL)
        return;
    inobufsize = blkroundup(check_sblk.b_un.b_fs, INOBUFSIZE);
    fullcnt = inobufsize / ((check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) ?
        sizeof(struct ufs1_dinode) : sizeof(struct ufs2_dinode));
    readpercg = check_sblk.b_un.b_fs->fs_ipg / fullcnt;
    partialcnt = check_sblk.b_un.b_fs->fs_ipg % fullcnt;
    partialsize = partialcnt * ((check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) ?
        sizeof(struct ufs1_dinode) : sizeof(struct ufs2_dinode));
    if (partialcnt != 0) {
        readpercg++;
    } else {
        partialcnt = fullcnt;
        partialsize = inobufsize;
    }
    initbarea(&inobuf, BT_INODES);
    if ((inobuf.b_un.b_buf = Malloc((unsigned)inobufsize)) == NULL)
        errx(EEXIT, "cannot allocate space for inode buffer");
}

static void
freeinodebuf(void)
{

    if (inobuf.b_un.b_buf != NULL)
        free((char *)inobuf.b_un.b_buf);
    inobuf.b_un.b_buf = NULL;
}

/*
 * Routines to maintain information about directory inodes.
 * This is built during the first pass and used during the
 * second and third passes.
 *
 * Enter inodes into the cache.
 */
static void
cacheino(union dinode *dp, ufs_ino_t inumber)
{
    struct inoinfo *inp, **inpp;
    int i, blks;

    if (howmany(DIP(dp, di_size), check_sblk.b_un.b_fs->fs_bsize) > NDADDR)
        blks = NDADDR + NIADDR;
    else
        blks = howmany(DIP(dp, di_size), check_sblk.b_un.b_fs->fs_bsize);
    inp = (struct inoinfo *)
        Malloc(sizeof(*inp) + (blks - 1) * sizeof(ufs2_daddr_t));
    if (inp == NULL)
        errx(EEXIT, "cannot increase directory list");
    inpp = &inphead[inumber % dirhash];
    inp->i_nexthash = *inpp;
    *inpp = inp;
    inp->i_parent = (inumber == ROOTINO) ? ROOTINO : 0;
    inp->i_dotdot = 0;
    inp->i_number = inumber;
    inp->i_isize = DIP(dp, di_size);
    inp->i_numblks = blks;
    for (i = 0; i < (blks < NDADDR ? blks : NDADDR); i++)
        inp->i_blks[i] = DIP(dp, di_db[i]);
    if (blks > NDADDR)
        for (i = 0; i < NIADDR; i++)
            inp->i_blks[NDADDR + i] = DIP(dp, di_ib[i]);
    if (inplast == listmax) {
        listmax += 100;
        inpsort = (struct inoinfo **)realloc((char *)inpsort,
            (unsigned)listmax * sizeof(struct inoinfo *));
        if (inpsort == NULL)
            errx(EEXIT, "cannot increase directory list");
    }
    inpsort[inplast++] = inp;
}

/*
 * Look up an inode cache structure.
 */
static struct inoinfo *
getinoinfo(ufs_ino_t inumber)
{
    struct inoinfo *inp;

    for (inp = inphead[inumber % dirhash]; inp; inp = inp->i_nexthash) {
        if (inp->i_number != inumber)
            continue;
        return (inp);
    }
    errx(EEXIT, "cannot find inode %u", inumber);
    return ((struct inoinfo *)0);
}

/*
 * Clean up all the inode cache structure.
 */
void
check_inocleanup(void)
{
    struct inoinfo **inpp;

    if (inphead == NULL)
        return;
    for (inpp = &inpsort[inplast - 1]; inpp >= inpsort; inpp--)
        free((char *)(*inpp));
    free((char *)inphead);
    free((char *)inpsort);
    inphead = inpsort = NULL;
}

static void
clri(struct inodesc *idesc, const char *type, int flag)
{
    union dinode *dp;

    dp = check_ginode(idesc->id_number);
    if (flag == 1) {
        check_warn("%s %s", type,
            (DIP(dp, di_mode) & IFMT) == IFDIR ? "DIR" : "FILE");
        pinode(idesc->id_number);
    }
    if (check_preen || check_reply("CLEAR") == 1) {
        if (check_preen)
            printf(" (CLEARED)\n");
        check_n_files--;
        (void)check_inode(dp, idesc);
        inoinfo(idesc->id_number)->ino_state = USTATE;
        clearinode(dp);
        check_inodirty();
    }
}

static int
clearentry(struct inodesc *idesc)
{
    struct direct *dirp = idesc->id_dirp;

    if (dirp->d_ino != idesc->id_parent || idesc->id_entryno < 2) {
        idesc->id_entryno++;
        return (KEEPON);
    }
    dirp->d_ino = 0;
    return (STOP|FOUND|ALTERED);
}

static void
blkerror(ufs_ino_t ino, const char *type, ufs2_daddr_t blk)
{
    check_fatal("%jd %s I=%u\n", (intmax_t)blk, type, ino);
    printf("\n");
    switch (inoinfo(ino)->ino_state) {

    case FSTATE:
    case FZLINK:
        inoinfo(ino)->ino_state = FCLEAR;
        return;

    case DSTATE:
    case DZLINK:
        inoinfo(ino)->ino_state = DCLEAR;
        return;

    case FCLEAR:
    case DCLEAR:
        return;

    default:
        errx(EEXIT, "BAD STATE %d TO BLKERR", inoinfo(ino)->ino_state);
        /* NOTREACHED */
    }
}

/*
 * allocate an unused inode
 */
static ufs_ino_t
allocino(ufs_ino_t request, int type)
{
    ufs_ino_t ino;
    union dinode *dp;
    struct bufarea *cgbp;
    struct cg *cgp;
    int cg;

    if (request == 0)
        request = ROOTINO;
    else if (inoinfo(request)->ino_state != USTATE)
        return (0);
    for (ino = request; ino < check_maxino; ino++)
        if (inoinfo(ino)->ino_state == USTATE)
            break;
    if (ino == check_maxino)
        return (0);
    cg = ino_to_cg(check_sblk.b_un.b_fs, ino);
    cgbp = check_cgget(cg);
    cgp = cgbp->b_un.b_cg;
    if (!check_cgmagic(cg, cgbp))
        return (0);
    setbit(cg_inosused(cgp), ino % check_sblk.b_un.b_fs->fs_ipg);
    cgp->cg_cs.cs_nifree--;
    switch (type & IFMT) {
    case IFDIR:
        inoinfo(ino)->ino_state = DSTATE;
        cgp->cg_cs.cs_ndir++;
        break;
    case IFREG:
    case IFLNK:
        inoinfo(ino)->ino_state = FSTATE;
        break;
    default:
        return (0);
    }
    dirty(cgbp);
    dp = check_ginode(ino);
    DIP_SET(dp, di_db[0], allocblk((long)1));
    if (DIP(dp, di_db[0]) == 0) {
        inoinfo(ino)->ino_state = USTATE;
        return (0);
    }
    DIP_SET(dp, di_mode, type);
    DIP_SET(dp, di_flags, 0);
    DIP_SET(dp, di_atime, time(NULL));
    DIP_SET(dp, di_ctime, DIP(dp, di_atime));
    DIP_SET(dp, di_mtime, DIP(dp, di_ctime));
    DIP_SET(dp, di_mtimensec, 0);
    DIP_SET(dp, di_ctimensec, 0);
    DIP_SET(dp, di_atimensec, 0);
    DIP_SET(dp, di_size, check_sblk.b_un.b_fs->fs_fsize);
    DIP_SET(dp, di_blocks, bytes_to_sectors(check_sblk.b_un.b_fs->fs_fsize));
    check_n_files++;
    check_inodirty();
    inoinfo(ino)->ino_type = IFTODT(type);
    return (ino);
}

/*
 * deallocate an inode
 */
static void
freeino(ufs_ino_t ino)
{
    struct inodesc idesc;
    union dinode *dp;

    memset(&idesc, 0, sizeof(struct inodesc));
    idesc.id_type = ADDR;
    idesc.id_func = pass4check;
    idesc.id_number = ino;
    dp = check_ginode(ino);
    (void)check_inode(dp, &idesc);
    clearinode(dp);
    check_inodirty();
    inoinfo(ino)->ino_state = USTATE;
    check_n_files--;
}

/*
 * Propagate connected state through the tree.
 */
static void
propagate(void)
{
    struct inoinfo **inpp, *inp;
    struct inoinfo **inpend;
    long change;

    inpend = &inpsort[inplast];
    do {
        change = 0;
        for (inpp = inpsort; inpp < inpend; inpp++) {
            inp = *inpp;
            if (inp->i_parent == 0)
                continue;
            if (inoinfo(inp->i_parent)->ino_state == DFOUND &&
                INO_IS_DUNFOUND(inp->i_number)) {
                inoinfo(inp->i_number)->ino_state = DFOUND;
                change++;
            }
        }
    } while (change > 0);
}

static void
adjust(struct inodesc *idesc, int lcnt)
{
    union dinode *dp;
    int saveresolved;

    dp = check_ginode(idesc->id_number);
    if (DIP(dp, di_nlink) == lcnt) {
        /*
         * If we have not hit any unresolved problems, are running
         * in preen mode, and are on a file system using soft updates,
         * then just toss any partially allocated files.
         */
        if (check_resolved && check_preen && check_usedsoftdep) {
            clri(idesc, "UNREF", 1);
            return;
        } else {
            /*
             * The file system can be marked clean even if
             * a file is not linked up, but is cleared.
             * Hence, resolved should not be cleared when
             * linkup is answered no, but clri is answered yes.
             */
            saveresolved = check_resolved;
            if (linkup(idesc->id_number, 0, NULL) == 0) {
                check_resolved = saveresolved;
                clri(idesc, "UNREF", 0);
                return;
            }
            /*
             * Account for the new reference created by linkup().
             */
            dp = check_ginode(idesc->id_number);
            lcnt--;
        }
    }
    if (lcnt != 0) {
        check_warn("LINK COUNT %s", (lfdir == idesc->id_number) ? lfname :
            ((DIP(dp, di_mode) & IFMT) == IFDIR ? "DIR" : "FILE"));
        pinode(idesc->id_number);
        printf(" COUNT %d SHOULD BE %d",
            DIP(dp, di_nlink), DIP(dp, di_nlink) - lcnt);
        if (check_preen || check_usedsoftdep) {
            if (lcnt < 0) {
                printf("\n");
                check_fatal("LINK COUNT INCREASING\n");
            }
            if (check_preen)
                printf(" (ADJUSTED)\n");
        }
        if (check_preen || check_reply("ADJUST") == 1) {
            DIP_SET(dp, di_nlink, DIP(dp, di_nlink) - lcnt);
            check_inodirty();
        }
    }
}

static int
mkentry(struct inodesc *idesc)
{
    struct direct *dirp = idesc->id_dirp;
    struct direct newent;
    int newlen, oldlen;

    newent.d_namlen = strlen(idesc->id_name);
    newlen = DIRSIZ(&newent);
    if (dirp->d_ino != 0)
        oldlen = DIRSIZ(dirp);
    else
        oldlen = 0;
    if (dirp->d_reclen - oldlen < newlen)
        return (KEEPON);
    newent.d_reclen = dirp->d_reclen - oldlen;
    dirp->d_reclen = oldlen;
    dirp = (struct direct *)(((char *)dirp) + oldlen);
    dirp->d_ino = idesc->id_parent; /* ino to be entered is in id_parent */
    dirp->d_reclen = newent.d_reclen;
    dirp->d_type = inoinfo(idesc->id_parent)->ino_type;
    dirp->d_namlen = newent.d_namlen;
    memmove(dirp->d_name, idesc->id_name, (size_t)newent.d_namlen + 1);
    return (ALTERED|STOP);
}

static int
chgino(struct inodesc *idesc)
{
    struct direct *dirp = idesc->id_dirp;

    if (memcmp(dirp->d_name, idesc->id_name, (int)dirp->d_namlen + 1))
        return (KEEPON);
    dirp->d_ino = idesc->id_parent;
    dirp->d_type = inoinfo(idesc->id_parent)->ino_type;
    return (ALTERED|STOP);
}

/*
 * free a directory inode
 */
static void
freedir(ufs_ino_t ino, ufs_ino_t parent)
{
    union dinode *dp;

    if (ino != parent) {
        dp = check_ginode(parent);
        DIP_SET(dp, di_nlink, DIP(dp, di_nlink) - 1);
        check_inodirty();
    }
    freeino(ino);
}

/*
 * generate a temporary name for the lost+found directory.
 */
static int
lftempname(char *bufp, ufs_ino_t ino)
{
    ufs_ino_t in;
    char *cp;
    int namlen;

    cp = bufp + 2;
    for (in = check_maxino; in > 0; in /= 10)
        cp++;
    *--cp = 0;
    namlen = cp - bufp;
    in = ino;
    while (cp > bufp) {
        *--cp = (in % 10) + '0';
        in /= 10;
    }
    *cp = '#';
    return (namlen);
}

/*
 * allocate a new directory
 */
static ufs_ino_t
allocdir(ufs_ino_t parent, ufs_ino_t request, int mode)
{
    ufs_ino_t ino;
    char *cp;
    union dinode *dp;
    struct bufarea *bp;
    struct inoinfo *inp;
    struct dirtemplate *dirp;

    ino = allocino(request, IFDIR|mode);
    dirp = &dirhead;
    dirp->dot_ino = ino;
    dirp->dotdot_ino = parent;
    dp = check_ginode(ino);
    bp = getdirblk(DIP(dp, di_db[0]), check_sblk.b_un.b_fs->fs_fsize);
    if (bp->b_errs) {
        freeino(ino);
        return (0);
    }
    memmove(bp->b_un.b_buf, dirp, sizeof(struct dirtemplate));
    for (cp = &bp->b_un.b_buf[DIRBLKSIZ];
         cp < &bp->b_un.b_buf[check_sblk.b_un.b_fs->fs_fsize];
         cp += DIRBLKSIZ)
        memmove(cp, &emptydir, sizeof emptydir);
    dirty(bp);
    DIP_SET(dp, di_nlink, 2);
    check_inodirty();
    if (ino == ROOTINO) {
        inoinfo(ino)->ino_linkcnt = DIP(dp, di_nlink);
        cacheino(dp, ino);
        return(ino);
    }
    if (!INO_IS_DVALID(parent)) {
        freeino(ino);
        return (0);
    }
    cacheino(dp, ino);
    inp = getinoinfo(ino);
    inp->i_parent = parent;
    inp->i_dotdot = parent;
    inoinfo(ino)->ino_state = inoinfo(parent)->ino_state;
    if (inoinfo(ino)->ino_state == DSTATE) {
        inoinfo(ino)->ino_linkcnt = DIP(dp, di_nlink);
        inoinfo(parent)->ino_linkcnt++;
    }
    dp = check_ginode(parent);
    DIP_SET(dp, di_nlink, DIP(dp, di_nlink) + 1);
    check_inodirty();
    return (ino);
}

/*
 * fix an entry in a directory.
 */
int
check_changeino(ufs_ino_t dir, const char *name, ufs_ino_t newnum)
{
    struct inodesc idesc;

    memset(&idesc, 0, sizeof(struct inodesc));
    idesc.id_type = DATA;
    idesc.id_func = chgino;
    idesc.id_number = dir;
    idesc.id_fix = DONTKNOW;
    idesc.id_name = strdup(name);
    idesc.id_parent = newnum;   /* new value for name */
    return (check_inode(check_ginode(dir), &idesc));
}

static int
linkup(ufs_ino_t orphan, ufs_ino_t parentdir, char *name)
{
    union dinode *dp;
    int lostdir;
    ufs_ino_t oldlfdir;
    struct inodesc idesc;
    char tempname[BUFSIZ];

    memset(&idesc, 0, sizeof(struct inodesc));
    dp = check_ginode(orphan);
    lostdir = (DIP(dp, di_mode) & IFMT) == IFDIR;
    check_warn("UNREF %s ", lostdir ? "DIR" : "FILE");
    pinode(orphan);
    if (check_preen && DIP(dp, di_size) == 0)
        return (0);

    if (check_preen)
        printf(" (RECONNECTED)\n");
    else
        if (check_reply("RECONNECT") == 0)
            return (0);

    if (lfdir == 0) {
        dp = check_ginode(ROOTINO);
        idesc.id_name = strdup(lfname);
        idesc.id_type = DATA;
        idesc.id_func = check_findino;
        idesc.id_number = ROOTINO;
        if ((check_inode(dp, &idesc) & FOUND) != 0) {
            lfdir = idesc.id_parent;
        } else {
            check_warn("NO lost+found DIRECTORY");
            if (check_preen || check_reply("CREATE")) {
                lfdir = allocdir(ROOTINO, 0, check_lfmode);
                if (lfdir != 0) {
                    if (check_makeentry(ROOTINO, lfdir, lfname) != 0) {
                        numdirs++;
                        if (check_preen)
                            printf(" (CREATED)\n");
                    } else {
                        freedir(lfdir, ROOTINO);
                        lfdir = 0;
                        if (check_preen)
                            printf("\n");
                    }
                }
            }
        }
        if (lfdir == 0) {
            check_fatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n");
            printf("\n");
            return (0);
        }
    }
    dp = check_ginode(lfdir);
    if ((DIP(dp, di_mode) & IFMT) != IFDIR) {
        check_fatal("lost+found IS NOT A DIRECTORY\n");
        if (check_reply("REALLOCATE") == 0)
            return (0);
        oldlfdir = lfdir;
        if ((lfdir = allocdir(ROOTINO, 0, check_lfmode)) == 0) {
            check_fatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
            return (0);
        }
        if ((check_changeino(ROOTINO, lfname, lfdir) & ALTERED) == 0) {
            check_fatal("SORRY. CANNOT CREATE lost+found DIRECTORY\n\n");
            return (0);
        }
        check_inodirty();
        idesc.id_type = ADDR;
        idesc.id_func = pass4check;
        idesc.id_number = oldlfdir;
        adjust(&idesc, inoinfo(oldlfdir)->ino_linkcnt + 1);
        inoinfo(oldlfdir)->ino_linkcnt = 0;
        dp = check_ginode(lfdir);
    }
    if (inoinfo(lfdir)->ino_state != DFOUND) {
        check_fatal("SORRY. NO lost+found DIRECTORY\n\n");
        return (0);
    }
    (void)lftempname(tempname, orphan);
    if (check_makeentry(lfdir, orphan, (name ? name : tempname)) == 0) {
        check_fatal("SORRY. NO SPACE IN lost+found DIRECTORY\n");
        printf("\n");
        return (0);
    }
    inoinfo(orphan)->ino_linkcnt--;
    if (lostdir) {
        if ((check_changeino(orphan, "..", lfdir) & ALTERED) == 0 &&
            parentdir != (ufs_ino_t)-1)
            (void)check_makeentry(orphan, lfdir, "..");
        dp = check_ginode(lfdir);
        DIP_SET(dp, di_nlink, DIP(dp, di_nlink) + 1);
        check_inodirty();
        inoinfo(lfdir)->ino_linkcnt++;
        check_warn("DIR I=%lu CONNECTED. ", (u_long)orphan);
        if (parentdir != (ufs_ino_t)-1) {
            printf("PARENT WAS I=%lu\n", (u_long)parentdir);
            /*
             * The parent directory, because of the ordering
             * guarantees, has had the link count incremented
             * for the child, but no entry was made.  This
             * fixes the parent link count so that fsck does
             * not need to be rerun.
             */
            inoinfo(parentdir)->ino_linkcnt++;
        }
        if (check_preen == 0)
            printf("\n");
    }
    return (1);
}

/*
 * Free a previously allocated block
 */
static void
freeblk(ufs2_daddr_t blkno, long frags)
{
    struct inodesc idesc;

    idesc.id_blkno = blkno;
    idesc.id_numfrags = frags;
    (void)pass4check(&idesc);
}

/*
 * Attempt to expand the size of a directory
 */
static int
expanddir(union dinode *dp, char *name)
{
    ufs2_daddr_t lastbn, newblk;
    struct bufarea *bp;
    char *cp, firstblk[DIRBLKSIZ];

    lastbn = lblkno(check_sblk.b_un.b_fs, DIP(dp, di_size));
    if (lastbn >= NDADDR - 1 || DIP(dp, di_db[lastbn]) == 0 ||
        DIP(dp, di_size) == 0)
        return (0);
    if ((newblk = allocblk(check_sblk.b_un.b_fs->fs_frag)) == 0)
        return (0);
    DIP_SET(dp, di_db[lastbn + 1], DIP(dp, di_db[lastbn]));
    DIP_SET(dp, di_db[lastbn], newblk);
    DIP_SET(dp, di_size, DIP(dp, di_size) + check_sblk.b_un.b_fs->fs_bsize);
    DIP_SET(dp, di_blocks, DIP(dp, di_blocks) + bytes_to_sectors(check_sblk.b_un.b_fs->fs_bsize));
    bp = getdirblk(DIP(dp, di_db[lastbn + 1]),
        sblksize(check_sblk.b_un.b_fs, DIP(dp, di_size), lastbn + 1));
    if (bp->b_errs)
        goto bad;
    memmove(firstblk, bp->b_un.b_buf, DIRBLKSIZ);
    bp = getdirblk(newblk, check_sblk.b_un.b_fs->fs_bsize);
    if (bp->b_errs)
        goto bad;
    memmove(bp->b_un.b_buf, firstblk, DIRBLKSIZ);
    for (cp = &bp->b_un.b_buf[DIRBLKSIZ];
         cp < &bp->b_un.b_buf[check_sblk.b_un.b_fs->fs_bsize];
         cp += DIRBLKSIZ)
        memmove(cp, &emptydir, sizeof emptydir);
    dirty(bp);
    bp = getdirblk(DIP(dp, di_db[lastbn + 1]),
        sblksize(check_sblk.b_un.b_fs, DIP(dp, di_size), lastbn + 1));
    if (bp->b_errs)
        goto bad;
    memmove(bp->b_un.b_buf, &emptydir, sizeof emptydir);
    check_warn("NO SPACE LEFT IN %s", name);
    if (check_preen)
        printf(" (EXPANDED)\n");
    else if (check_reply("EXPAND") == 0)
        goto bad;
    dirty(bp);
    check_inodirty();
    return (1);
bad:
    DIP_SET(dp, di_db[lastbn], DIP(dp, di_db[lastbn + 1]));
    DIP_SET(dp, di_db[lastbn + 1], 0);
    DIP_SET(dp, di_size, DIP(dp, di_size) - check_sblk.b_un.b_fs->fs_bsize);
    DIP_SET(dp, di_blocks, DIP(dp, di_blocks) - bytes_to_sectors(check_sblk.b_un.b_fs->fs_bsize));
    freeblk(newblk, check_sblk.b_un.b_fs->fs_frag);
    return (0);
}

/*
 * make an entry in a directory
 */
int
check_makeentry(ufs_ino_t parent, ufs_ino_t ino, const char *name)
{
    union dinode *dp;
    struct inodesc idesc;
    char pathbuf[MAXPATHLEN + 1];

    if (parent < ROOTINO || parent >= check_maxino ||
        ino < ROOTINO || ino >= check_maxino)
        return (0);
    memset(&idesc, 0, sizeof(struct inodesc));
    idesc.id_type = DATA;
    idesc.id_func = mkentry;
    idesc.id_number = parent;
    idesc.id_parent = ino;  /* this is the inode to enter */
    idesc.id_fix = DONTKNOW;
    idesc.id_name = strdup(name);
    dp = check_ginode(parent);
    if (DIP(dp, di_size) % DIRBLKSIZ) {
        DIP_SET(dp, di_size, roundup(DIP(dp, di_size), DIRBLKSIZ));
        check_inodirty();
    }
    if ((check_inode(dp, &idesc) & ALTERED) != 0)
        return (1);
    getpathname(pathbuf, parent, parent);
    dp = check_ginode(parent);
    if (expanddir(dp, pathbuf) == 0)
        return (0);
    return (check_inode(dp, &idesc) & ALTERED);
}

/*--------------------------------------------------------------
 * Pass 1.
 */
static ufs2_daddr_t badblk;
static ufs2_daddr_t dupblk;
static ufs_ino_t lastino;       /* last inode in use */

/*
 * Scan each entry in an ea block.
 */
static int
eascan(struct inodesc *idesc, struct ufs2_dinode *dp)
{
    struct bufarea *bp;
    u_int n;
    u_char *cp;
    long blksiz;

    printf("Inode %u extsize %ju\n",
        idesc->id_number, (intmax_t)dp->di_extsize);
    if (dp->di_extsize == 0)
        return 0;
    if (dp->di_extsize <= check_sblk.b_un.b_fs->fs_fsize)
        blksiz = check_sblk.b_un.b_fs->fs_fsize;
    else
        blksiz = check_sblk.b_un.b_fs->fs_bsize;
    printf("blksiz = %ju\n", (intmax_t)blksiz);
    bp = check_getdatablk(dp->di_extb[0], blksiz, BT_EXTATTR);
    cp = (u_char *)bp->b_un.b_buf;
    for (n = 0; n < blksiz; n++) {
        printf("%02x", cp[n]);
        if ((n & 31) == 31)
            printf("\n");
    }
    return (STOP);
}

static int
ckinode(ufs_ino_t inumber, struct inodesc *idesc, int rebuildcg)
{
    union dinode *dp;
    int64_t kernmaxfilesize;
    ufs2_daddr_t ndb;
    mode_t mode;
    int j, ret, offset;

    if ((dp = getnextinode(inumber, rebuildcg)) == NULL)
        return (0);
    mode = DIP(dp, di_mode) & IFMT;
    if (mode == 0) {
        if ((check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC &&
             (memcmp(dp->dp1.di_db, ufs1_zino.di_db,
            NDADDR * sizeof(ufs1_daddr_t)) ||
              memcmp(dp->dp1.di_ib, ufs1_zino.di_ib,
            NIADDR * sizeof(ufs1_daddr_t)) ||
              dp->dp1.di_mode || dp->dp1.di_size)) ||
            (check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC &&
             (memcmp(dp->dp2.di_db, ufs2_zino.di_db,
            NDADDR * sizeof(ufs2_daddr_t)) ||
              memcmp(dp->dp2.di_ib, ufs2_zino.di_ib,
            NIADDR * sizeof(ufs2_daddr_t)) ||
              dp->dp2.di_mode || dp->dp2.di_size))) {
            check_fatal("PARTIALLY ALLOCATED INODE I=%lu\n",
                (u_long)inumber);
            if (check_reply("CLEAR") == 1) {
                dp = check_ginode(inumber);
                clearinode(dp);
                check_inodirty();
            }
        }
        inoinfo(inumber)->ino_state = USTATE;
        return (1);
    }
    lastino = inumber;
    /* This should match the file size limit in ffs_mountfs(). */
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC)
        kernmaxfilesize = (int64_t)0x40000000 * check_sblk.b_un.b_fs->fs_bsize - 1;
    else
        kernmaxfilesize = check_sblk.b_un.b_fs->fs_maxfilesize;
    if (DIP(dp, di_size) > kernmaxfilesize ||
        DIP(dp, di_size) > check_sblk.b_un.b_fs->fs_maxfilesize ||
        (mode == IFDIR && DIP(dp, di_size) > MAXDIRSIZE)) {
        if (check_debug)
            printf("bad size %ju:", (uintmax_t)DIP(dp, di_size));
        goto unknown;
    }
    if (!check_preen && mode == IFMT && check_reply("HOLD BAD BLOCK") == 1) {
        dp = check_ginode(inumber);
        DIP_SET(dp, di_size, check_sblk.b_un.b_fs->fs_fsize);
        DIP_SET(dp, di_mode, IFREG|0600);
        check_inodirty();
    }
    if ((mode == IFBLK || mode == IFCHR || mode == IFIFO ||
         mode == IFSOCK) && DIP(dp, di_size) != 0) {
        if (check_debug)
            printf("bad special-file size %ju:",
                (uintmax_t)DIP(dp, di_size));
        goto unknown;
    }
    if ((mode == IFBLK || mode == IFCHR) &&
        (dev_t)DIP(dp, di_rdev) == (dev_t)-1) {
        if (check_debug)
            printf("bad special-file rdev NODEV:");
        goto unknown;
    }
    ndb = howmany(DIP(dp, di_size), check_sblk.b_un.b_fs->fs_bsize);
    if (ndb < 0) {
        if (check_debug)
            printf("bad size %ju ndb %ju:",
                (uintmax_t)DIP(dp, di_size), (uintmax_t)ndb);
        goto unknown;
    }
    if (mode == IFBLK || mode == IFCHR)
        ndb++;
    if (mode == IFLNK) {
        /*
         * Fake ndb value so direct/indirect block checks below
         * will detect any garbage after symlink string.
         */
        if (DIP(dp, di_size) < (int64_t)check_sblk.b_un.b_fs->fs_maxsymlinklen) {
            if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC)
                ndb = howmany(DIP(dp, di_size),
                    sizeof(ufs1_daddr_t));
            else
                ndb = howmany(DIP(dp, di_size),
                    sizeof(ufs2_daddr_t));
            if (ndb > NDADDR) {
                j = ndb - NDADDR;
                for (ndb = 1; j > 1; j--)
                    ndb *= NINDIR(check_sblk.b_un.b_fs);
                ndb += NDADDR;
            }
        }
    }
    for (j = ndb; ndb < NDADDR && j < NDADDR; j++)
        if (DIP(dp, di_db[j]) != 0) {
            if (check_debug)
                printf("bad direct addr[%d]: %ju\n", j,
                    (uintmax_t)DIP(dp, di_db[j]));
            goto unknown;
        }
    for (j = 0, ndb -= NDADDR; ndb > 0; j++)
        ndb /= NINDIR(check_sblk.b_un.b_fs);
    for (; j < NIADDR; j++)
        if (DIP(dp, di_ib[j]) != 0) {
            if (check_debug)
                printf("bad indirect addr: %ju\n",
                    (uintmax_t)DIP(dp, di_ib[j]));
            goto unknown;
        }
    if (ftypeok(dp) == 0)
        goto unknown;
    check_n_files++;
    inoinfo(inumber)->ino_linkcnt = DIP(dp, di_nlink);
    if (mode == IFDIR) {
        if (DIP(dp, di_size) == 0)
            inoinfo(inumber)->ino_state = DCLEAR;
        else if (DIP(dp, di_nlink) <= 0)
            inoinfo(inumber)->ino_state = DZLINK;
        else
            inoinfo(inumber)->ino_state = DSTATE;
        cacheino(dp, inumber);
        countdirs++;
    } else if (DIP(dp, di_nlink) <= 0)
        inoinfo(inumber)->ino_state = FZLINK;
    else
        inoinfo(inumber)->ino_state = FSTATE;
    inoinfo(inumber)->ino_type = IFTODT(mode);
    badblk = dupblk = 0;
    idesc->id_number = inumber;
#ifdef SF_SNAPSHOT
    if (DIP(dp, di_flags) & SF_SNAPSHOT)
        idesc->id_type = SNAP;
    else
#endif
        idesc->id_type = ADDR;
    (void)check_inode(dp, idesc);
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC && dp->dp2.di_extsize > 0) {
        idesc->id_type = ADDR;
        ndb = howmany(dp->dp2.di_extsize, check_sblk.b_un.b_fs->fs_bsize);
        for (j = 0; j < NXADDR; j++) {
            if (--ndb == 0 &&
                (offset = blkoff(check_sblk.b_un.b_fs, dp->dp2.di_extsize)) != 0)
                idesc->id_numfrags = numfrags(check_sblk.b_un.b_fs,
                    fragroundup(check_sblk.b_un.b_fs, offset));
            else
                idesc->id_numfrags = check_sblk.b_un.b_fs->fs_frag;
            if (dp->dp2.di_extb[j] == 0)
                continue;
            idesc->id_blkno = dp->dp2.di_extb[j];
            ret = (*idesc->id_func)(idesc);
            if (ret & STOP)
                break;
        }
    }
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC)
        eascan(idesc, &dp->dp2);
    idesc->id_entryno *= bytes_to_sectors(check_sblk.b_un.b_fs->fs_fsize);
    if (DIP(dp, di_blocks) != idesc->id_entryno) {
        check_warn("INCORRECT BLOCK COUNT I=%lu (%ju should be %ju)",
            (u_long)inumber, (uintmax_t)DIP(dp, di_blocks),
            (uintmax_t)idesc->id_entryno);
        if (check_preen)
            printf(" (CORRECTED)\n");
        else if (check_reply("CORRECT") == 0)
            return (1);
        dp = check_ginode(inumber);
        DIP_SET(dp, di_blocks, idesc->id_entryno);
        check_inodirty();
    }
    return (1);
unknown:
    check_fatal("UNKNOWN FILE TYPE I=%lu\n", (u_long)inumber);
    inoinfo(inumber)->ino_state = FCLEAR;
    if (check_reply("CLEAR") == 1) {
        inoinfo(inumber)->ino_state = USTATE;
        dp = check_ginode(inumber);
        clearinode(dp);
        check_inodirty();
    }
    return (1);
}

void
check_pass1(void)
{
    struct inostat *info;
    struct inodesc idesc;
    struct bufarea *cgbp;
    struct cg *cgp;
    ufs_ino_t inumber, inosused, mininos;
    ufs2_daddr_t i, cgd;
    u_int8_t *cp;
    int c, rebuildcg;

    /*
     * Set file system reserved blocks in used block map.
     */
    for (c = 0; c < check_sblk.b_un.b_fs->fs_ncg; c++) {
        cgd = cgdmin(check_sblk.b_un.b_fs, c);
        if (c == 0) {
            i = cgbase(check_sblk.b_un.b_fs, c);
        } else
            i = cgsblock(check_sblk.b_un.b_fs, c);
        for (; i < cgd; i++)
            setbmap(i);
    }
    i = check_sblk.b_un.b_fs->fs_csaddr;
    cgd = i + howmany(check_sblk.b_un.b_fs->fs_cssize, check_sblk.b_un.b_fs->fs_fsize);
    for (; i < cgd; i++)
        setbmap(i);

    /*
     * Find all allocated blocks.
     */
    memset(&idesc, 0, sizeof(struct inodesc));
    idesc.id_func = pass1check;
    check_n_files = check_n_blks = 0;
    for (c = 0; c < check_sblk.b_un.b_fs->fs_ncg; c++) {
        inumber = c * check_sblk.b_un.b_fs->fs_ipg;
        setinodebuf(inumber);
        cgbp = check_cgget(c);
        cgp = cgbp->b_un.b_cg;
        rebuildcg = 0;
        if (!check_cgmagic(c, cgbp))
            rebuildcg = 1;
        if (!rebuildcg && check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC) {
            inosused = cgp->cg_initediblk;
            if (inosused > check_sblk.b_un.b_fs->fs_ipg) {
                check_fatal("Too many initialized inodes (%u > %d) in cylinder group %d\nReset to %d\n",
                    inosused, check_sblk.b_un.b_fs->fs_ipg,
                    c, check_sblk.b_un.b_fs->fs_ipg);
                inosused = check_sblk.b_un.b_fs->fs_ipg;
            }
        } else {
            inosused = check_sblk.b_un.b_fs->fs_ipg;
        }
#if 0
        if (got_siginfo) {
            printf("%s: phase 1: cyl group %d of %d (%d%%)\n",
                check_filename, c, check_sblk.b_un.b_fs->fs_ncg,
                c * 100 / check_sblk.b_un.b_fs->fs_ncg);
            got_siginfo = 0;
        }
#endif
        /*
         * If we are using soft updates, then we can trust the
         * cylinder group inode allocation maps to tell us which
         * inodes are allocated. We will scan the used inode map
         * to find the inodes that are really in use, and then
         * read only those inodes in from disk.
         */
        if ((check_preen || check_inoopt) && check_usedsoftdep && !rebuildcg) {
            cp = &cg_inosused(cgp)[(inosused - 1) / CHAR_BIT];
            for ( ; inosused > 0; inosused -= CHAR_BIT, cp--) {
                if (*cp == 0)
                    continue;
                for (i = 1 << (CHAR_BIT - 1); i > 0; i >>= 1) {
                    if (*cp & i)
                        break;
                    inosused--;
                }
                break;
            }
            if ((int)inosused < 0)
                inosused = 0;
        }
        /*
         * Allocate inoinfo structures for the allocated inodes.
         */
        check_inostathead[c].il_numalloced = inosused;
        if (inosused == 0) {
            check_inostathead[c].il_stat = 0;
            continue;
        }
        info = Calloc((unsigned)inosused, sizeof(struct inostat));
        if (info == NULL)
            errx(EEXIT, "cannot alloc %u bytes for inoinfo",
                (unsigned)(sizeof(struct inostat) * inosused));
        check_inostathead[c].il_stat = info;
        /*
         * Scan the allocated inodes.
         */
        for (i = 0; i < inosused; i++, inumber++) {
            if (inumber < ROOTINO) {
                (void)getnextinode(inumber, rebuildcg);
                continue;
            }
            /*
             * NULL return indicates probable end of allocated
             * inodes during cylinder group rebuild attempt.
             * We always keep trying until we get to the minimum
             * valid number for this cylinder group.
             */
            if (ckinode(inumber, &idesc, rebuildcg) == 0 &&
                i > cgp->cg_initediblk)
                break;
        }
        /*
         * This optimization speeds up future runs of fsck
         * by trimming down the number of inodes in cylinder
         * groups that formerly had many inodes but now have
         * fewer in use.
         */
        mininos = roundup(inosused + INOPB(check_sblk.b_un.b_fs), INOPB(check_sblk.b_un.b_fs));
        if (check_inoopt && !check_preen && !rebuildcg &&
            check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC &&
            cgp->cg_initediblk > 2 * INOPB(check_sblk.b_un.b_fs) &&
            mininos < cgp->cg_initediblk) {
            i = cgp->cg_initediblk;
            if (mininos < 2 * INOPB(check_sblk.b_un.b_fs))
                cgp->cg_initediblk = 2 * INOPB(check_sblk.b_un.b_fs);
            else
                cgp->cg_initediblk = mininos;
            check_warn("CYLINDER GROUP %d: RESET FROM %ju TO %d %s\n",
                c, (long)i, cgp->cg_initediblk, "VALID INODES");
            dirty(cgbp);
        }
        if (inosused < check_sblk.b_un.b_fs->fs_ipg)
            continue;
        lastino += 1;
        if (lastino < (c * check_sblk.b_un.b_fs->fs_ipg))
            inosused = 0;
        else
            inosused = lastino - (c * check_sblk.b_un.b_fs->fs_ipg);
        if (rebuildcg && inosused > cgp->cg_initediblk &&
            check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC) {
            cgp->cg_initediblk = roundup(inosused, INOPB(check_sblk.b_un.b_fs));
            check_warn("CYLINDER GROUP %d: FOUND %d VALID INODES\n", c,
                cgp->cg_initediblk);
        }
        /*
         * If we were not able to determine in advance which inodes
         * were in use, then reduce the size of the inoinfo structure
         * to the size necessary to describe the inodes that we
         * really found.
         */
        if (inumber == lastino)
            continue;
        check_inostathead[c].il_numalloced = inosused;
        if (inosused == 0) {
            free(check_inostathead[c].il_stat);
            check_inostathead[c].il_stat = 0;
            continue;
        }
        info = Calloc((unsigned)inosused, sizeof(struct inostat));
        if (info == NULL)
            errx(EEXIT, "cannot alloc %u bytes for inoinfo",
                (unsigned)(sizeof(struct inostat) * inosused));
        memmove(info, check_inostathead[c].il_stat, inosused * sizeof(*info));
        free(check_inostathead[c].il_stat);
        check_inostathead[c].il_stat = info;
    }
    freeinodebuf();
}

static int
pass1check(struct inodesc *idesc)
{
    int res = KEEPON;
    int anyout, nfrags;
    ufs2_daddr_t blkno = idesc->id_blkno;
    struct dups *dlp;
    struct dups *new;

    if (idesc->id_type == SNAP) {
        if (blkno == BLK_NOCOPY)
            return (KEEPON);
        if (blkno == BLK_SNAP)
            return (KEEPON);
    }
    if ((anyout = chkrange(blkno, idesc->id_numfrags)) != 0) {
        blkerror(idesc->id_number, "BAD", blkno);
        if (badblk++ >= MAXBAD) {
            check_warn("EXCESSIVE BAD BLKS I=%lu",
                (u_long)idesc->id_number);
            if (check_preen)
                printf(" (SKIPPING)\n");
            else if (check_reply("CONTINUE") == 0) {
                check_finish(0);
                exit(EEXIT);
            }
            return (STOP);
        }
    }
    for (nfrags = idesc->id_numfrags; nfrags > 0; blkno++, nfrags--) {
        if (anyout && chkrange(blkno, 1)) {
            res = SKIP;
        } else if (!testbmap(blkno)) {
            check_n_blks++;
            setbmap(blkno);
        } else {
            blkerror(idesc->id_number, "DUP", blkno);
            if (dupblk++ >= MAXDUP) {
                check_warn("EXCESSIVE DUP BLKS I=%lu",
                    (u_long)idesc->id_number);
                if (check_preen)
                    printf(" (SKIPPING)\n");
                else if (check_reply("CONTINUE") == 0) {
                    check_finish(0);
                    exit(EEXIT);
                }
                return (STOP);
            }
            new = (struct dups *)Malloc(sizeof(struct dups));
            if (new == NULL) {
                check_fatal("DUP TABLE OVERFLOW.\n");
                if (check_reply("CONTINUE") == 0) {
                    check_finish(0);
                    exit(EEXIT);
                }
                return (STOP);
            }
            new->dup = blkno;
            if (check_muldup == 0) {
                check_duplist = check_muldup = new;
                new->next = 0;
            } else {
                new->next = check_muldup->next;
                check_muldup->next = new;
            }
            for (dlp = check_duplist; dlp != check_muldup; dlp = dlp->next)
                if (dlp->dup == blkno)
                    break;
            if (dlp == check_muldup && dlp->dup != blkno)
                check_muldup = new;
        }
        /*
         * count the number of blocks found in id_entryno
         */
        idesc->id_entryno++;
    }
    return (res);
}

/*--------------------------------------------------------------
 * Pass 1b.
 */
static  struct dups *duphead;

static int
pass1bcheck(struct inodesc *idesc)
{
    struct dups *dlp;
    int nfrags, res = KEEPON;
    ufs2_daddr_t blkno = idesc->id_blkno;

    for (nfrags = idesc->id_numfrags; nfrags > 0; blkno++, nfrags--) {
        if (chkrange(blkno, 1))
            res = SKIP;
        for (dlp = duphead; dlp; dlp = dlp->next) {
            if (dlp->dup == blkno) {
                blkerror(idesc->id_number, "DUP", blkno);
                dlp->dup = duphead->dup;
                duphead->dup = blkno;
                duphead = duphead->next;
            }
            if (dlp == check_muldup)
                break;
        }
        if (check_muldup == 0 || duphead == check_muldup->next)
            return (STOP);
    }
    return (res);
}

void
check_pass1b(void)
{
    int c, i;
    union dinode *dp;
    struct inodesc idesc;
    ufs_ino_t inumber;

    memset(&idesc, 0, sizeof(struct inodesc));
    idesc.id_type = ADDR;
    idesc.id_func = pass1bcheck;
    duphead = check_duplist;
    inumber = 0;
    for (c = 0; c < check_sblk.b_un.b_fs->fs_ncg; c++) {
#if 0
        if (got_siginfo) {
            printf("%s: phase 1b: cyl group %d of %d (%d%%)\n",
                check_filename, c, check_sblk.b_un.b_fs->fs_ncg,
                c * 100 / check_sblk.b_un.b_fs->fs_ncg);
            got_siginfo = 0;
        }
#endif
        for (i = 0; i < check_sblk.b_un.b_fs->fs_ipg; i++, inumber++) {
            if (inumber < ROOTINO)
                continue;
            dp = check_ginode(inumber);
            if (dp == NULL)
                continue;
            idesc.id_number = inumber;
            if (inoinfo(inumber)->ino_state != USTATE &&
                (check_inode(dp, &idesc) & STOP))
                return;
        }
    }
}

/*--------------------------------------------------------------
 * Pass 2.
 */
#define MINDIRSIZE  (sizeof (struct dirtemplate))

static int
deleteentry(struct inodesc *idesc)
{
    struct direct *dirp = idesc->id_dirp;

    if (idesc->id_entryno++ < 2 || dirp->d_ino != idesc->id_parent)
        return (KEEPON);
    dirp->d_ino = 0;
    return (ALTERED|STOP|FOUND);
}

static int
fix_extraneous(struct inoinfo *inp, struct inodesc *idesc)
{
    struct inodesc dotdesc;
    char oldname[MAXPATHLEN + 1];
    char newname[MAXPATHLEN + 1];

    /*
     * If we have not yet found "..", look it up now so we know
     * which inode the directory itself believes is its parent.
     */
    if (inp->i_dotdot == 0) {
        memset(&dotdesc, 0, sizeof(struct inodesc));
        dotdesc.id_type = DATA;
        dotdesc.id_number = idesc->id_dirp->d_ino;
        dotdesc.id_func = check_findino;
        dotdesc.id_name = strdup("..");
        if ((check_inode(check_ginode(dotdesc.id_number), &dotdesc) & FOUND))
            inp->i_dotdot = dotdesc.id_parent;
    }
    /*
     * We have the previously found old name (inp->i_parent) and the
     * just found new name (idesc->id_number). We have five cases:
     * 1)  ".." is missing - can remove either name, choose to delete
     *     new one and let fsck create ".." pointing to old name.
     * 2) Both new and old are in same directory, choose to delete
     *    the new name and let fsck fix ".." if it is wrong.
     * 3) ".." does not point to the new name, so delete it and let
     *    fsck fix ".." to point to the old one if it is wrong.
     * 4) ".." points to the old name only, so delete the new one.
     * 5) ".." points to the new name only, so delete the old one.
     *
     * For cases 1-4 we eliminate the new name;
     * for case 5 we eliminate the old name.
     */
    if (inp->i_dotdot == 0 ||           /* Case 1 */
        idesc->id_number == inp->i_parent ||    /* Case 2 */
        inp->i_dotdot != idesc->id_number ||    /* Case 3 */
        inp->i_dotdot == inp->i_parent) {       /* Case 4 */
        getpathname(newname, idesc->id_number, idesc->id_number);
        if (strcmp(newname, "/") != 0)
            strcat (newname, "/");
        strcat(newname, idesc->id_dirp->d_name);
        getpathname(oldname, inp->i_number, inp->i_number);
        check_warn("%s IS AN EXTRANEOUS HARD LINK TO DIRECTORY %s",
            newname, oldname);
        if (check_preen) {
            printf(" (REMOVED)\n");
            return (1);
        }
        return (check_reply("REMOVE"));
    }
    /*
     * None of the first four cases above, so must be case (5).
     * Eliminate the old name and make the new the name the parent.
     */
    getpathname(oldname, inp->i_parent, inp->i_number);
    getpathname(newname, inp->i_number, inp->i_number);
    check_warn("%s IS AN EXTRANEOUS HARD LINK TO DIRECTORY %s", oldname,
        newname);
    if (!check_preen && !check_reply("REMOVE"))
        return (0);
    memset(&dotdesc, 0, sizeof(struct inodesc));
    dotdesc.id_type = DATA;
    dotdesc.id_number = inp->i_parent; /* directory in which name appears */
    dotdesc.id_parent = inp->i_number; /* inode number in entry to delete */
    dotdesc.id_func = deleteentry;
    if ((check_inode(check_ginode(dotdesc.id_number), &dotdesc) & FOUND) && check_preen)
        printf(" (REMOVED)\n");
    inp->i_parent = idesc->id_number;  /* reparent to correct directory */
    inoinfo(inp->i_number)->ino_linkcnt++; /* name gone, return reference */
    return (0);
}

static int
pass2check(struct inodesc *idesc)
{
    struct direct *dirp = idesc->id_dirp;
    struct inoinfo *inp;
    int n, entrysize, ret = 0;
    union dinode *dp;
    const char *errmsg;
    struct direct proto;

    /*
     * check for "."
     */
    if (dirp->d_ino > check_maxino)
        goto chk2;
    if (idesc->id_entryno != 0)
        goto chk1;
    if (dirp->d_ino != 0 && strcmp(dirp->d_name, ".") == 0) {
        if (dirp->d_ino != idesc->id_number) {
            direrror(idesc->id_number, "BAD INODE NUMBER FOR '.'");
            dirp->d_ino = idesc->id_number;
            if (check_reply("FIX") == 1)
                ret |= ALTERED;
        }
        if (dirp->d_type != DT_DIR) {
            direrror(idesc->id_number, "BAD TYPE VALUE FOR '.'");
            dirp->d_type = DT_DIR;
            if (check_reply("FIX") == 1)
                ret |= ALTERED;
        }
        goto chk1;
    }
    direrror(idesc->id_number, "MISSING '.'");
    proto.d_ino = idesc->id_number;
    proto.d_type = DT_DIR;
    proto.d_namlen = 1;
    (void)strcpy(proto.d_name, ".");
    entrysize = DIRSIZ(&proto);
    if (dirp->d_ino != 0 && strcmp(dirp->d_name, "..") != 0) {
        check_fatal("CANNOT FIX, FIRST ENTRY IN DIRECTORY CONTAINS %s\n",
            dirp->d_name);
    } else if (dirp->d_reclen < entrysize) {
        check_fatal("CANNOT FIX, INSUFFICIENT SPACE TO ADD '.'\n");
    } else if (dirp->d_reclen < 2 * entrysize) {
        proto.d_reclen = dirp->d_reclen;
        memmove(dirp, &proto, (size_t)entrysize);
        if (check_reply("FIX") == 1)
            ret |= ALTERED;
    } else {
        n = dirp->d_reclen - entrysize;
        proto.d_reclen = entrysize;
        memmove(dirp, &proto, (size_t)entrysize);
        idesc->id_entryno++;
        inoinfo(dirp->d_ino)->ino_linkcnt--;
        dirp = (struct direct *)((char *)(dirp) + entrysize);
        memset(dirp, 0, (size_t)n);
        dirp->d_reclen = n;
        if (check_reply("FIX") == 1)
            ret |= ALTERED;
    }
chk1:
    if (idesc->id_entryno > 1)
        goto chk2;
    inp = getinoinfo(idesc->id_number);
    proto.d_ino = inp->i_parent;
    proto.d_type = DT_DIR;
    proto.d_namlen = 2;
    (void)strcpy(proto.d_name, "..");
    entrysize = DIRSIZ(&proto);
    if (idesc->id_entryno == 0) {
        n = DIRSIZ(dirp);
        if (dirp->d_reclen < n + entrysize)
            goto chk2;
        proto.d_reclen = dirp->d_reclen - n;
        dirp->d_reclen = n;
        idesc->id_entryno++;
        inoinfo(dirp->d_ino)->ino_linkcnt--;
        dirp = (struct direct *)((char *)(dirp) + n);
        memset(dirp, 0, (size_t)proto.d_reclen);
        dirp->d_reclen = proto.d_reclen;
    }
    if (dirp->d_ino != 0 && strcmp(dirp->d_name, "..") == 0) {
        inp->i_dotdot = dirp->d_ino;
        if (dirp->d_type != DT_DIR) {
            direrror(idesc->id_number, "BAD TYPE VALUE FOR '..'");
            dirp->d_type = DT_DIR;
            if (check_reply("FIX") == 1)
                ret |= ALTERED;
        }
        goto chk2;
    }
    if (dirp->d_ino != 0 && strcmp(dirp->d_name, ".") != 0) {
        fileerror(inp->i_parent, idesc->id_number, "MISSING '..'");
        check_fatal("CANNOT FIX, SECOND ENTRY IN DIRECTORY CONTAINS %s\n",
            dirp->d_name);
        inp->i_dotdot = (ufs_ino_t)-1;
    } else if (dirp->d_reclen < entrysize) {
        fileerror(inp->i_parent, idesc->id_number, "MISSING '..'");
        check_fatal("CANNOT FIX, INSUFFICIENT SPACE TO ADD '..'\n");
        inp->i_dotdot = (ufs_ino_t)-1;
    } else if (inp->i_parent != 0) {
        /*
         * We know the parent, so fix now.
         */
        inp->i_dotdot = inp->i_parent;
        fileerror(inp->i_parent, idesc->id_number, "MISSING '..'");
        proto.d_reclen = dirp->d_reclen;
        memmove(dirp, &proto, (size_t)entrysize);
        if (check_reply("FIX") == 1)
            ret |= ALTERED;
    }
    idesc->id_entryno++;
    if (dirp->d_ino != 0)
        inoinfo(dirp->d_ino)->ino_linkcnt--;
    return (ret|KEEPON);
chk2:
    if (dirp->d_ino == 0)
        return (ret|KEEPON);
    if (dirp->d_namlen <= 2 &&
        dirp->d_name[0] == '.' &&
        idesc->id_entryno >= 2) {
        if (dirp->d_namlen == 1) {
            direrror(idesc->id_number, "EXTRA '.' ENTRY");
            dirp->d_ino = 0;
            if (check_reply("FIX") == 1)
                ret |= ALTERED;
            return (KEEPON | ret);
        }
        if (dirp->d_name[1] == '.') {
            direrror(idesc->id_number, "EXTRA '..' ENTRY");
            dirp->d_ino = 0;
            if (check_reply("FIX") == 1)
                ret |= ALTERED;
            return (KEEPON | ret);
        }
    }
    idesc->id_entryno++;
    n = 0;
    if (dirp->d_ino > check_maxino) {
        fileerror(idesc->id_number, dirp->d_ino, "I OUT OF RANGE");
        n = check_reply("REMOVE");
    } else if (((dirp->d_ino == WINO && dirp->d_type != DT_WHT) ||
            (dirp->d_ino != WINO && dirp->d_type == DT_WHT))) {
        fileerror(idesc->id_number, dirp->d_ino, "BAD WHITEOUT ENTRY");
        dirp->d_ino = WINO;
        dirp->d_type = DT_WHT;
        if (check_reply("FIX") == 1)
            ret |= ALTERED;
    } else {
again:
        switch (inoinfo(dirp->d_ino)->ino_state) {
        case USTATE:
            if (idesc->id_entryno <= 2)
                break;
            fileerror(idesc->id_number, dirp->d_ino, "UNALLOCATED");
            n = check_reply("REMOVE");
            break;

        case DCLEAR:
        case FCLEAR:
            if (idesc->id_entryno <= 2)
                break;
            if (inoinfo(dirp->d_ino)->ino_state == FCLEAR)
                errmsg = "DUP/BAD";
            else if (!check_preen && !check_usedsoftdep)
                errmsg = "ZERO LENGTH DIRECTORY";
            else {
                n = 1;
                break;
            }
            fileerror(idesc->id_number, dirp->d_ino, errmsg);
            if ((n = check_reply("REMOVE")) == 1)
                break;
            dp = check_ginode(dirp->d_ino);
            inoinfo(dirp->d_ino)->ino_state =
               (DIP(dp, di_mode) & IFMT) == IFDIR ? DSTATE : FSTATE;
            inoinfo(dirp->d_ino)->ino_linkcnt = DIP(dp, di_nlink);
            goto again;

        case DSTATE:
        case DZLINK:
            if (inoinfo(idesc->id_number)->ino_state == DFOUND)
                inoinfo(dirp->d_ino)->ino_state = DFOUND;
            /* FALLTHROUGH */

        case DFOUND:
            inp = getinoinfo(dirp->d_ino);
            if (idesc->id_entryno > 2) {
                if (inp->i_parent == 0)
                    inp->i_parent = idesc->id_number;
                else if ((n = fix_extraneous(inp, idesc)) == 1)
                    break;
            }
            /* FALLTHROUGH */

        case FSTATE:
        case FZLINK:
            if (dirp->d_type != inoinfo(dirp->d_ino)->ino_type) {
                fileerror(idesc->id_number, dirp->d_ino,
                    "BAD TYPE VALUE");
                dirp->d_type = inoinfo(dirp->d_ino)->ino_type;
                if (check_reply("FIX") == 1)
                    ret |= ALTERED;
            }
            inoinfo(dirp->d_ino)->ino_linkcnt--;
            break;

        default:
            errx(EEXIT, "BAD STATE %d FOR INODE I=%u",
                inoinfo(dirp->d_ino)->ino_state, dirp->d_ino);
        }
    }
    if (n == 0)
        return (ret|KEEPON);
    dirp->d_ino = 0;
    return (ret|KEEPON|ALTERED);
}

/*
 * Routine to sort disk blocks.
 */
static int
blksort(const void *arg1, const void *arg2)
{
    return ((*(struct inoinfo * const *)arg1)->i_blks[0] -
        (*(struct inoinfo * const *)arg2)->i_blks[0]);
}

void
check_pass2(void)
{
    union dinode *dp;
    struct inoinfo **inpp, *inp;
    struct inoinfo **inpend;
    struct inodesc curino;
    union dinode dino;
    int i;
    char pathbuf[MAXPATHLEN + 1];

    switch (inoinfo(ROOTINO)->ino_state) {

    case USTATE:
        check_fatal("ROOT INODE UNALLOCATED\n");
        if (check_reply("ALLOCATE") == 0) {
            check_finish(0);
            exit(EEXIT);
        }
        if (allocdir(ROOTINO, ROOTINO, 0755) != ROOTINO)
            errx(EEXIT, "CANNOT ALLOCATE ROOT INODE");
        break;

    case DCLEAR:
        check_fatal("DUPS/BAD IN ROOT INODE\n");
        if (check_reply("REALLOCATE")) {
            freeino(ROOTINO);
            if (allocdir(ROOTINO, ROOTINO, 0755) != ROOTINO)
                errx(EEXIT, "CANNOT ALLOCATE ROOT INODE");
            break;
        }
        if (check_reply("CONTINUE") == 0) {
            check_finish(0);
            exit(EEXIT);
        }
        break;

    case FSTATE:
    case FCLEAR:
    case FZLINK:
        check_fatal("ROOT INODE NOT DIRECTORY\n");
        if (check_reply("REALLOCATE")) {
            freeino(ROOTINO);
            if (allocdir(ROOTINO, ROOTINO, 0755) != ROOTINO)
                errx(EEXIT, "CANNOT ALLOCATE ROOT INODE");
            break;
        }
        if (check_reply("FIX") == 0) {
            check_finish(0);
            exit(EEXIT);
        }
        dp = check_ginode(ROOTINO);
        DIP_SET(dp, di_mode, DIP(dp, di_mode) & ~IFMT);
        DIP_SET(dp, di_mode, DIP(dp, di_mode) | IFDIR);
        check_inodirty();
        break;

    case DSTATE:
    case DZLINK:
        break;

    default:
        errx(EEXIT, "BAD STATE %d FOR ROOT INODE",
            inoinfo(ROOTINO)->ino_state);
    }
    inoinfo(ROOTINO)->ino_state = DFOUND;
    inoinfo(WINO)->ino_state = FSTATE;
    inoinfo(WINO)->ino_type = DT_WHT;

    /*
     * Sort the directory list into disk block order.
     */
    qsort((char *)inpsort, (size_t)inplast, sizeof *inpsort, blksort);

    /*
     * Check the integrity of each directory.
     */
    memset(&curino, 0, sizeof(struct inodesc));
    curino.id_type = DATA;
    curino.id_func = pass2check;
    inpend = &inpsort[inplast];
    for (inpp = inpsort; inpp < inpend; inpp++) {
#if 0
        if (got_siginfo) {
            printf("%s: phase 2: dir %td of %d (%d%%)\n", check_filename,
                inpp - inpsort, (int)inplast,
                (int)((inpp - inpsort) * 100 / inplast));
            got_siginfo = 0;
        }
#endif
        inp = *inpp;
        if (inp->i_isize == 0)
            continue;
        if (inp->i_isize < MINDIRSIZE) {
            direrror(inp->i_number, "DIRECTORY TOO SHORT");
            inp->i_isize = roundup(MINDIRSIZE, DIRBLKSIZ);
            if (check_reply("FIX") == 1) {
                dp = check_ginode(inp->i_number);
                DIP_SET(dp, di_size, inp->i_isize);
                check_inodirty();
            }
        } else if ((inp->i_isize & (DIRBLKSIZ - 1)) != 0) {
            getpathname(pathbuf, inp->i_number, inp->i_number);
            if (check_usedsoftdep)
                check_fatal("%s %s: LENGTH %jd NOT MULTIPLE OF %d\n",
                    "DIRECTORY", pathbuf,
                    (intmax_t)inp->i_isize, DIRBLKSIZ);
            else
                check_warn("%s %s: LENGTH %jd NOT MULTIPLE OF %d",
                    "DIRECTORY", pathbuf,
                    (intmax_t)inp->i_isize, DIRBLKSIZ);
            if (check_preen)
                printf(" (ADJUSTED)\n");
            inp->i_isize = roundup(inp->i_isize, DIRBLKSIZ);
            if (check_preen || check_reply("ADJUST") == 1) {
                dp = check_ginode(inp->i_number);
                DIP_SET(dp, di_size,
                    roundup(inp->i_isize, DIRBLKSIZ));
                check_inodirty();
            }
        }
        dp = &dino;
        memset(dp, 0, sizeof(struct ufs2_dinode));
        DIP_SET(dp, di_mode, IFDIR);
        DIP_SET(dp, di_size, inp->i_isize);
        for (i = 0;
             i < (inp->i_numblks<NDADDR ? inp->i_numblks : NDADDR);
             i++)
            DIP_SET(dp, di_db[i], inp->i_blks[i]);
        if (inp->i_numblks > NDADDR)
            for (i = 0; i < NIADDR; i++)
                DIP_SET(dp, di_ib[i], inp->i_blks[NDADDR + i]);
        curino.id_number = inp->i_number;
        curino.id_parent = inp->i_parent;
        (void)check_inode(dp, &curino);
    }
    /*
     * Now that the parents of all directories have been found,
     * make another pass to verify the value of `..'
     */
    for (inpp = inpsort; inpp < inpend; inpp++) {
        inp = *inpp;
        if (inp->i_parent == 0 || inp->i_isize == 0)
            continue;
        if (inoinfo(inp->i_parent)->ino_state == DFOUND &&
            INO_IS_DUNFOUND(inp->i_number))
            inoinfo(inp->i_number)->ino_state = DFOUND;
        if (inp->i_dotdot == inp->i_parent ||
            inp->i_dotdot == (ufs_ino_t)-1)
            continue;
        if (inp->i_dotdot == 0) {
            inp->i_dotdot = inp->i_parent;
            fileerror(inp->i_parent, inp->i_number, "MISSING '..'");
            if (check_reply("FIX") == 0)
                continue;
            (void)check_makeentry(inp->i_number, inp->i_parent, "..");
            inoinfo(inp->i_parent)->ino_linkcnt--;
            continue;
        }
        /*
         * Here we have:
         *    inp->i_number is directory with bad ".." in it.
         *    inp->i_dotdot is current value of "..".
         *    inp->i_parent is directory to which ".." should point.
         */
        getpathname(pathbuf, inp->i_parent, inp->i_number);
        printf("BAD INODE NUMBER FOR '..' in DIR I=%u (%s)\n",
            inp->i_number, pathbuf);
        getpathname(pathbuf, inp->i_dotdot, inp->i_dotdot);
        printf("CURRENTLY POINTS TO I=%u (%s), ",
            inp->i_dotdot, pathbuf);
        getpathname(pathbuf, inp->i_parent, inp->i_parent);
        printf("SHOULD POINT TO I=%u (%s)",
            inp->i_parent, pathbuf);
        if (check_preen)
            printf(" (FIXED)\n");
        else if (check_reply("FIX") == 0)
            continue;
        inoinfo(inp->i_dotdot)->ino_linkcnt++;
        inoinfo(inp->i_parent)->ino_linkcnt--;
        inp->i_dotdot = inp->i_parent;
        (void)check_changeino(inp->i_number, "..", inp->i_parent);
    }
    /*
     * Mark all the directories that can be found from the root.
     */
    propagate();
}

/*--------------------------------------------------------------
 * Pass 3.
 */
void
check_pass3(void)
{
    struct inoinfo *inp;
    int loopcnt, inpindex, state;
    ufs_ino_t orphan;
    struct inodesc idesc;
    char namebuf[MAXNAMLEN+1];

    for (inpindex = inplast - 1; inpindex >= 0; inpindex--) {
#if 0
        if (got_siginfo) {
            printf("%s: phase 3: dir %d of %d (%d%%)\n", check_filename,
                (int)(inplast - inpindex - 1), (int)inplast,
                (int)((inplast - inpindex - 1) * 100 / inplast));
            got_siginfo = 0;
        }
#endif
        inp = inpsort[inpindex];
        state = inoinfo(inp->i_number)->ino_state;
        if (inp->i_number == ROOTINO ||
            (inp->i_parent != 0 && !S_IS_DUNFOUND(state)))
            continue;
        if (state == DCLEAR)
            continue;
        /*
         * If we are running with soft updates and we come
         * across unreferenced directories, we just leave
         * them in DSTATE which will cause them to be pitched
         * in pass 4.
         */
        if (check_preen && check_resolved && check_usedsoftdep && S_IS_DUNFOUND(state)) {
            if (inp->i_dotdot >= ROOTINO)
                inoinfo(inp->i_dotdot)->ino_linkcnt++;
            continue;
        }
        for (loopcnt = 0; ; loopcnt++) {
            orphan = inp->i_number;
            if (inp->i_parent == 0 ||
                !INO_IS_DUNFOUND(inp->i_parent) ||
                loopcnt > countdirs)
                break;
            inp = getinoinfo(inp->i_parent);
        }
        if (loopcnt <= countdirs) {
            if (linkup(orphan, inp->i_dotdot, NULL)) {
                inp->i_parent = inp->i_dotdot = lfdir;
                inoinfo(lfdir)->ino_linkcnt--;
            }
            inoinfo(orphan)->ino_state = DFOUND;
            propagate();
            continue;
        }
        check_fatal("ORPHANED DIRECTORY LOOP DETECTED I=%lu\n",
            (u_long)orphan);
        if (check_reply("RECONNECT") == 0)
            continue;
        memset(&idesc, 0, sizeof(struct inodesc));
        idesc.id_type = DATA;
        idesc.id_number = inp->i_parent;
        idesc.id_parent = orphan;
        idesc.id_func = findname;
        idesc.id_name = namebuf;
        if ((check_inode(check_ginode(inp->i_parent), &idesc) & FOUND) == 0)
            check_fatal("COULD NOT FIND NAME IN PARENT DIRECTORY\n");
        if (linkup(orphan, inp->i_parent, namebuf)) {
            idesc.id_func = clearentry;
            if (check_inode(check_ginode(inp->i_parent), &idesc) & FOUND)
                inoinfo(orphan)->ino_linkcnt++;
            inp->i_parent = inp->i_dotdot = lfdir;
            inoinfo(lfdir)->ino_linkcnt--;
        }
        inoinfo(orphan)->ino_state = DFOUND;
        propagate();
    }
}

/*--------------------------------------------------------------
 * Pass 4.
 */
void
check_pass4(void)
{
    ufs_ino_t inumber;
    union dinode *dp;
    struct inodesc idesc;
    int i, n, cg;

    memset(&idesc, 0, sizeof(struct inodesc));
    idesc.id_type = ADDR;
    idesc.id_func = pass4check;
    for (cg = 0; cg < check_sblk.b_un.b_fs->fs_ncg; cg++) {
#if 0
        if (got_siginfo) {
            printf("%s: phase 4: cyl group %d of %d (%d%%)\n",
                check_filename, cg, check_sblk.b_un.b_fs->fs_ncg,
                cg * 100 / check_sblk.b_un.b_fs->fs_ncg);
            got_siginfo = 0;
        }
#endif
        inumber = cg * check_sblk.b_un.b_fs->fs_ipg;
        for (i = 0; i < check_inostathead[cg].il_numalloced; i++, inumber++) {
            if (inumber < ROOTINO)
                continue;
            idesc.id_number = inumber;
            switch (inoinfo(inumber)->ino_state) {

            case FZLINK:
            case DZLINK:
                if (inoinfo(inumber)->ino_linkcnt == 0) {
                    clri(&idesc, "UNREF", 1);
                    break;
                }
                /* fall through */

            case FSTATE:
            case DFOUND:
                n = inoinfo(inumber)->ino_linkcnt;
                if (n) {
                    adjust(&idesc, (short)n);
                    break;
                }
                break;

            case DSTATE:
                clri(&idesc, "UNREF", 1);
                break;

            case DCLEAR:
                dp = check_ginode(inumber);
                if (DIP(dp, di_size) == 0) {
                    clri(&idesc, "ZERO LENGTH", 1);
                    break;
                }
                /* fall through */
            case FCLEAR:
                clri(&idesc, "BAD/DUP", 1);
                break;

            case USTATE:
                break;

            default:
                errx(EEXIT, "BAD STATE %d FOR INODE I=%u",
                    inoinfo(inumber)->ino_state, inumber);
            }
        }
    }
}

static int
pass4check(struct inodesc *idesc)
{
    struct dups *dlp;
    int nfrags, res = KEEPON;
    ufs2_daddr_t blkno = idesc->id_blkno;

    for (nfrags = idesc->id_numfrags; nfrags > 0; blkno++, nfrags--) {
        if (chkrange(blkno, 1)) {
            res = SKIP;
        } else if (testbmap(blkno)) {
            for (dlp = check_duplist; dlp; dlp = dlp->next) {
                if (dlp->dup != blkno)
                    continue;
                dlp->dup = check_duplist->dup;
                dlp = check_duplist;
                check_duplist = check_duplist->next;
                free((char *)dlp);
                break;
            }
            if (dlp == 0) {
                clrbmap(blkno);
                check_n_blks--;
            }
        }
    }
    return (res);
}

/*--------------------------------------------------------------
 * Pass 5.
 */
#include <inttypes.h>

static void
clear_blocks(ufs2_daddr_t start, ufs2_daddr_t end)
{

    if (check_debug)
        printf("Zero frags %jd to %jd\n", (intmax_t)start, (intmax_t)end);
    if (check_Zflag)
        blzero(check_fswritefd, fsbtodb(check_sblk.b_un.b_fs, start),
            lfragtosize(check_sblk.b_un.b_fs, end - start + 1));
    if (check_Eflag)
        blerase(check_fswritefd, fsbtodb(check_sblk.b_un.b_fs, start),
            lfragtosize(check_sblk.b_un.b_fs, end - start + 1));
}

static void
check_maps(
    u_char *map1,               /* map of claimed allocations */
    u_char *map2,               /* map of determined allocations */
    int mapsize,                /* size of above two maps */
    ufs2_daddr_t startvalue,    /* resource value for first element in map */
    const char *name,           /* name of resource found in maps */
    int skip,                   /* number of entries to skip before starting to free */
    int limit)                  /* limit on number of entries to free */
{
#define BUFSIZE 16
    long i, j, k, l, m, size;
    ufs2_daddr_t n, astart, aend, ustart, uend;
    void (*msg)(const char *fmt, ...);

    msg = check_warn;
    astart = ustart = aend = uend = -1;
    for (i = 0; i < mapsize; i++) {
        j = *map1++;
        k = *map2++;
        if (j == k)
            continue;
        for (m = 0, l = 1; m < CHAR_BIT; m++, l <<= 1) {
            if ((j & l) == (k & l))
                continue;
            n = startvalue + i * CHAR_BIT + m;
            if ((j & l) != 0) {
                if (astart == -1) {
                    astart = aend = n;
                    continue;
                }
                if (aend + 1 == n) {
                    aend = n;
                    continue;
                }
                if (astart == aend)
                    (*msg)("ALLOCATED %s %" PRId64
                        " MARKED FREE\n",
                        name, astart);
                else
                    (*msg)("%s %sS %" PRId64 "-%" PRId64
                        " MARKED FREE\n",
                        "ALLOCATED", name, astart, aend);
                astart = aend = n;
            } else {
                if (ustart == -1) {
                    ustart = uend = n;
                    continue;
                }
                if (uend + 1 == n) {
                    uend = n;
                    continue;
                }
                size = uend - ustart + 1;
                if (size <= skip) {
                    skip -= size;
                    ustart = uend = n;
                    continue;
                }
                if (skip > 0) {
                    ustart += skip;
                    size -= skip;
                    skip = 0;
                }
                if (size > limit)
                    size = limit;
                if (check_debug && size == 1)
                    check_warn("%s %s %" PRId64
                        " MARKED USED\n",
                        "UNALLOCATED", name, ustart);
                else if (check_debug)
                    check_warn("%s %sS %" PRId64 "-%" PRId64
                        " MARKED USED\n",
                        "UNALLOCATED", name, ustart,
                        ustart + size - 1);
                limit -= size;
                if (limit <= 0)
                    return;
                ustart = uend = n;
            }
        }
    }
    if (astart != -1) {
        if (astart == aend)
            (*msg)("ALLOCATED %s %" PRId64
                " MARKED FREE\n", name, astart);
        else
            (*msg)("ALLOCATED %sS %" PRId64 "-%" PRId64
                " MARKED FREE\n",
                name, astart, aend);
    }
    if (ustart != -1) {
        size = uend - ustart + 1;
        if (size <= skip)
            return;
        if (skip > 0) {
            ustart += skip;
            size -= skip;
        }
        if (size > limit)
            size = limit;
        if (check_debug) {
            if (size == 1)
                check_warn("UNALLOCATED %s %" PRId64
                    " MARKED USED\n",
                    name, ustart);
            else
                check_warn("UNALLOCATED %sS %" PRId64 "-%" PRId64
                    " MARKED USED\n",
                    name, ustart, ustart + size - 1);
        }
    }
}

/*
 * Compare the original cylinder group inode and block bitmaps with the
 * updated cylinder group inode and block bitmaps. Free inodes and blocks
 * that have been added. Complain if any previously freed inodes blocks
 * are now allocated.
 */
static void
update_maps(
    struct cg *oldcg,   /* cylinder group of claimed allocations */
    struct cg *newcg)   /* cylinder group of determined allocations */
{
    int inomapsize, excessdirs;
    struct fs *fs = check_sblk.b_un.b_fs;

    inomapsize = howmany(fs->fs_ipg, CHAR_BIT);
    excessdirs = oldcg->cg_cs.cs_ndir - newcg->cg_cs.cs_ndir;
    if (excessdirs < 0) {
        check_fatal("LOST %d DIRECTORIES\n", -excessdirs);
        excessdirs = 0;
    }
    if (excessdirs > 0)
        check_maps(cg_inosused(newcg), cg_inosused(oldcg), inomapsize,
            oldcg->cg_cgx * (ufs2_daddr_t)fs->fs_ipg, "DIR",
            0, excessdirs);
    check_maps(cg_inosused(newcg), cg_inosused(oldcg), inomapsize,
        oldcg->cg_cgx * (ufs2_daddr_t)fs->fs_ipg, "FILE",
        excessdirs, fs->fs_ipg);
    check_maps(cg_blksfree(oldcg), cg_blksfree(newcg),
        howmany(fs->fs_fpg, CHAR_BIT),
        oldcg->cg_cgx * (ufs2_daddr_t)fs->fs_fpg, "FRAG",
        0, fs->fs_fpg);
}

void
check_pass5(void)
{
    int c, i, j, blk, frags, basesize, mapsize;
    int inomapsize, blkmapsize;
    struct fs *fs = check_sblk.b_un.b_fs;
    ufs2_daddr_t d, dbase, dmax, start;
    int rewritecg = 0;
    struct csum *cs;
    struct csum_total cstotal;
    struct inodesc idesc[3];
    char buf[MAXBSIZE];
    struct cg *cg, *newcg = (struct cg *)buf;
    struct bufarea *cgbp;

    inoinfo(WINO)->ino_state = USTATE;
    memset(newcg, 0, (size_t)fs->fs_cgsize);
    newcg->cg_niblk = fs->fs_ipg;
    if (check_cvtlevel >= 3) {
        if (fs->fs_maxcontig < 2 && fs->fs_contigsumsize > 0) {
            if (check_preen)
                check_warn("DELETING CLUSTERING MAPS\n");
            if (check_preen || check_reply("DELETE CLUSTERING MAPS")) {
                fs->fs_contigsumsize = 0;
                rewritecg = 1;
                dirty(&check_sblk);
            }
        }
        if (fs->fs_maxcontig > 1) {
            const char *doit = 0;

            if (fs->fs_contigsumsize < 1) {
                doit = "CREAT";
            } else if (fs->fs_contigsumsize < fs->fs_maxcontig &&
                   fs->fs_contigsumsize < FS_MAXCONTIG) {
                doit = "EXPAND";
            }
            if (doit) {
                i = fs->fs_contigsumsize;
                fs->fs_contigsumsize =
                    MIN(fs->fs_maxcontig, FS_MAXCONTIG);
                if (CGSIZE(fs) > (u_int)fs->fs_bsize) {
                    check_warn("CANNOT %s CLUSTER MAPS\n", doit);
                    fs->fs_contigsumsize = i;
                } else if (check_preen ||
                    check_reply("CREATE CLUSTER MAPS")) {
                    if (check_preen)
                        check_warn("%sING CLUSTER MAPS\n",
                            doit);
                    fs->fs_cgsize =
                        fragroundup(fs, CGSIZE(fs));
                    rewritecg = 1;
                    dirty(&check_sblk);
                }
            }
        }
    }
    basesize = &newcg->cg_space[0] - (u_char *)(&newcg->cg_firstfield);
    if (check_sblk.b_un.b_fs->fs_magic == FS_UFS2_MAGIC) {
        newcg->cg_iusedoff = basesize;
    } else {
        /*
         * We reserve the space for the old rotation summary
         * tables for the benefit of old kernels, but do not
         * maintain them in modern kernels. In time, they can
         * go away.
         */
        newcg->cg_old_btotoff = basesize;
        newcg->cg_old_boff = newcg->cg_old_btotoff +
            fs->fs_old_cpg * sizeof(int32_t);
        newcg->cg_iusedoff = newcg->cg_old_boff +
            fs->fs_old_cpg * fs->fs_old_nrpos * sizeof(u_int16_t);
        memset(&newcg->cg_space[0], 0, newcg->cg_iusedoff - basesize);
    }
    inomapsize = howmany(fs->fs_ipg, CHAR_BIT);
    newcg->cg_freeoff = newcg->cg_iusedoff + inomapsize;
    blkmapsize = howmany(fs->fs_fpg, CHAR_BIT);
    newcg->cg_nextfreeoff = newcg->cg_freeoff + blkmapsize;
    if (fs->fs_contigsumsize > 0) {
        newcg->cg_clustersumoff = newcg->cg_nextfreeoff -
            sizeof(u_int32_t);
        newcg->cg_clustersumoff =
            roundup(newcg->cg_clustersumoff, sizeof(u_int32_t));
        newcg->cg_clusteroff = newcg->cg_clustersumoff +
            (fs->fs_contigsumsize + 1) * sizeof(u_int32_t);
        newcg->cg_nextfreeoff = newcg->cg_clusteroff +
            howmany(fragstoblks(fs, fs->fs_fpg), CHAR_BIT);
    }
    newcg->cg_magic = CG_MAGIC;
    mapsize = newcg->cg_nextfreeoff - newcg->cg_iusedoff;
    memset(&idesc[0], 0, sizeof idesc);
    for (i = 0; i < 3; i++)
        idesc[i].id_type = ADDR;
    memset(&cstotal, 0, sizeof(struct csum_total));
    dmax = blknum(fs, fs->fs_size + fs->fs_frag - 1);
    for (d = fs->fs_size; d < dmax; d++)
        setbmap(d);
    for (c = 0; c < fs->fs_ncg; c++) {
#if 0
        if (got_siginfo) {
            printf("%s: phase 5: cyl group %d of %d (%d%%)\n",
                check_filename, c, check_sblk.b_un.b_fs->fs_ncg,
                c * 100 / check_sblk.b_un.b_fs->fs_ncg);
            got_siginfo = 0;
        }
#endif
        cgbp = check_cgget(c);
        cg = cgbp->b_un.b_cg;
        if (!cg_chkmagic(cg))
            check_fatal("CG %d: BAD MAGIC NUMBER\n", c);
        newcg->cg_time = cg->cg_time;
        newcg->cg_old_time = cg->cg_old_time;
        newcg->cg_unrefs = cg->cg_unrefs;
        newcg->cg_cgx = c;
        dbase = cgbase(fs, c);
        dmax = dbase + fs->fs_fpg;
        if (dmax > fs->fs_size)
            dmax = fs->fs_size;
        newcg->cg_ndblk = dmax - dbase;
        if (fs->fs_magic == FS_UFS1_MAGIC) {
            newcg->cg_old_ncyl = 0;
            newcg->cg_old_niblk = fs->fs_ipg;
            newcg->cg_niblk = 0;
        }
        if (fs->fs_contigsumsize > 0)
            newcg->cg_nclusterblks = newcg->cg_ndblk / fs->fs_frag;
        newcg->cg_cs.cs_ndir = 0;
        newcg->cg_cs.cs_nffree = 0;
        newcg->cg_cs.cs_nbfree = 0;
        newcg->cg_cs.cs_nifree = fs->fs_ipg;
        if (~cg->cg_rotor != 0 && cg->cg_rotor < newcg->cg_ndblk)
            newcg->cg_rotor = cg->cg_rotor;
        else
            newcg->cg_rotor = 0;
        if (~cg->cg_frotor != 0 && cg->cg_frotor < newcg->cg_ndblk)
            newcg->cg_frotor = cg->cg_frotor;
        else
            newcg->cg_frotor = 0;
        if (~cg->cg_irotor != 0 && cg->cg_irotor < fs->fs_ipg)
            newcg->cg_irotor = cg->cg_irotor;
        else
            newcg->cg_irotor = 0;
        if (fs->fs_magic == FS_UFS1_MAGIC) {
            newcg->cg_initediblk = 0;
        } else {
            if ((unsigned)cg->cg_initediblk > fs->fs_ipg)
                newcg->cg_initediblk = fs->fs_ipg;
            else
                newcg->cg_initediblk = cg->cg_initediblk;
        }
        memset(&newcg->cg_frsum[0], 0, sizeof newcg->cg_frsum);
        memset(cg_inosused(newcg), 0, (size_t)(mapsize));
        j = fs->fs_ipg * c;
        for (i = 0; i < check_inostathead[c].il_numalloced; j++, i++) {
            switch (inoinfo(j)->ino_state) {

            case USTATE:
                break;

            case DSTATE:
            case DCLEAR:
            case DFOUND:
            case DZLINK:
                newcg->cg_cs.cs_ndir++;
                /* FALLTHROUGH */

            case FSTATE:
            case FCLEAR:
            case FZLINK:
                newcg->cg_cs.cs_nifree--;
                setbit(cg_inosused(newcg), i);
                break;

            default:
                if (j < (int)ROOTINO)
                    break;
                errx(EEXIT, "BAD STATE %d FOR INODE I=%d",
                    inoinfo(j)->ino_state, j);
            }
        }
        if (c == 0)
            for (i = 0; i < (int)ROOTINO; i++) {
                setbit(cg_inosused(newcg), i);
                newcg->cg_cs.cs_nifree--;
            }
        start = -1;
        for (i = 0, d = dbase;
             d < dmax;
             d += fs->fs_frag, i += fs->fs_frag) {
            frags = 0;
            for (j = 0; j < fs->fs_frag; j++) {
                if (testbmap(d + j)) {
                    if ((check_Eflag || check_Zflag) && start != -1) {
                        clear_blocks(start, d + j - 1);
                        start = -1;
                    }
                    continue;
                }
                if (start == -1)
                    start = d + j;
                setbit(cg_blksfree(newcg), i + j);
                frags++;
            }
            if (frags == fs->fs_frag) {
                newcg->cg_cs.cs_nbfree++;
                if (fs->fs_contigsumsize > 0)
                    setbit(cg_clustersfree(newcg),
                        i / fs->fs_frag);
            } else if (frags > 0) {
                newcg->cg_cs.cs_nffree += frags;
                blk = blkmap(fs, cg_blksfree(newcg), i);
                ffs_fragacct(fs, blk, (int32_t*)newcg->cg_frsum, 1);
            }
        }
        if ((check_Eflag || check_Zflag) && start != -1)
            clear_blocks(start, d - 1);
        if (fs->fs_contigsumsize > 0) {
            int32_t *sump = cg_clustersum(newcg);
            u_char *mapp = cg_clustersfree(newcg);
            int map = *mapp++;
            int bit = 1;
            int run = 0;

            for (i = 0; i < newcg->cg_nclusterblks; i++) {
                if ((map & bit) != 0) {
                    run++;
                } else if (run != 0) {
                    if (run > fs->fs_contigsumsize)
                        run = fs->fs_contigsumsize;
                    sump[run]++;
                    run = 0;
                }
                if ((i & (CHAR_BIT - 1)) != (CHAR_BIT - 1)) {
                    bit <<= 1;
                } else {
                    map = *mapp++;
                    bit = 1;
                }
            }
            if (run != 0) {
                if (run > fs->fs_contigsumsize)
                    run = fs->fs_contigsumsize;
                sump[run]++;
            }
        }
        cstotal.cs_nffree += newcg->cg_cs.cs_nffree;
        cstotal.cs_nbfree += newcg->cg_cs.cs_nbfree;
        cstotal.cs_nifree += newcg->cg_cs.cs_nifree;
        cstotal.cs_ndir += newcg->cg_cs.cs_ndir;

        cs = &fs->fs_cs(fs, c);
        if (memcmp(&newcg->cg_cs, cs, sizeof *cs) != 0 &&
            dofix(&idesc[0], "FREE BLK COUNT(S) WRONG IN SUPERBLK")) {
            memmove(cs, &newcg->cg_cs, sizeof *cs);
            dirty(&check_sblk);
        }
        if (rewritecg) {
            memmove(cg, newcg, (size_t)fs->fs_cgsize);
            dirty(cgbp);
            continue;
        }
        if (memcmp(newcg, cg, basesize) != 0 &&
            dofix(&idesc[2], "SUMMARY INFORMATION BAD")) {
            memmove(cg, newcg, (size_t)basesize);
            dirty(cgbp);
        }
        if (check_usedsoftdep || check_debug)
            update_maps(cg, newcg);
        if (memcmp(cg_inosused(newcg), cg_inosused(cg), mapsize) != 0 &&
            dofix(&idesc[1], "BLK(S) MISSING IN BIT MAPS")) {
            memmove(cg_inosused(cg), cg_inosused(newcg),
                  (size_t)mapsize);
            dirty(cgbp);
        }
    }
    if (memcmp(&cstotal, &fs->fs_cstotal, sizeof cstotal) != 0
        && dofix(&idesc[0], "SUMMARY BLK COUNT(S) WRONG IN SUPERBLK")) {
        memmove(&fs->fs_cstotal, &cstotal, sizeof cstotal);
        fs->fs_ronly = 0;
        fs->fs_fmod = 0;
        dirty(&check_sblk);
    }
}

/*
 * Check the specified file system.
 */
void ufs_check(ufs_t *disk, const char *filesys, int verbose, int fix)
{
    ufs2_daddr_t n_ffree, n_bfree;
    struct dups *dp;
    int cylno;
    intmax_t blks, files;

    check_filename = filesys;
    check_part_offset = disk->d_part_offset;
    if (fix)
        check_yflag = 1;
    else
        check_nflag = 1;

    check_sblock_init();
    switch (check_setup(filesys, 0)) {
    case 0:
        check_fatal("CAN'T CHECK FILE SYSTEM.\n");
        return;
    case -1:
        check_warn("clean, %ld free ", (long)(check_sblk.b_un.b_fs->fs_cstotal.cs_nffree +
            check_sblk.b_un.b_fs->fs_frag * check_sblk.b_un.b_fs->fs_cstotal.cs_nbfree));
        printf("(%jd frags, %jd blocks, %.1f%% fragmentation)\n",
            (intmax_t)check_sblk.b_un.b_fs->fs_cstotal.cs_nffree,
            (intmax_t)check_sblk.b_un.b_fs->fs_cstotal.cs_nbfree,
            check_sblk.b_un.b_fs->fs_cstotal.cs_nffree * 100.0 / check_sblk.b_un.b_fs->fs_dsize);
        return;
    }

    /*
     * Determine if we can and should do journal recovery.
     */
    if ((check_sblk.b_un.b_fs->fs_flags & FS_SUJ) == FS_SUJ) {
        if ((check_sblk.b_un.b_fs->fs_flags & FS_NEEDSFSCK) != FS_NEEDSFSCK && check_skipclean) {
            if (check_suj(filesys, check_part_offset) == 0) {
                printf("\n***** FILE SYSTEM MARKED CLEAN *****\n");
                exit(0);
            }
            printf("** Skipping journal, falling through to full fsck\n\n");
        }
        /*
         * Write the superblock so we don't try to recover the
         * journal on another pass.
         */
        check_sblk.b_un.b_fs->fs_mtime = time(NULL);
        dirty(&check_sblk);
    }

    /*
     * Cleared if any questions answered no. Used to decide if
     * the superblock should be marked clean.
     */
    check_resolved = 1;
    /*
     * 1: scan inodes tallying blocks used
     */
    printf("** Last Mounted on %s\n", check_sblk.b_un.b_fs->fs_fsmnt);
    printf("** Phase 1 - Check Blocks and Sizes\n");
    check_pass1();

    /*
     * 1b: locate first references to duplicates, if any
     */
    if (check_duplist) {
        printf("** Phase 1b - Rescan For More DUPS\n");
        check_pass1b();
    }

    /*
     * 2: traverse directories from root to mark all connected directories
     */
    printf("** Phase 2 - Check Pathnames\n");
    check_pass2();

    /*
     * 3: scan inodes looking for disconnected directories
     */
    printf("** Phase 3 - Check Connectivity\n");
    check_pass3();

    /*
     * 4: scan inodes looking for disconnected files; check reference counts
     */
    printf("** Phase 4 - Check Reference Counts\n");
    check_pass4();

    /*
     * 5: check and repair resource counts in cylinder groups
     */
    printf("** Phase 5 - Check Cyl groups\n");
    check_pass5();

    /*
     * print out summary statistics
     */
    n_ffree = check_sblk.b_un.b_fs->fs_cstotal.cs_nffree;
    n_bfree = check_sblk.b_un.b_fs->fs_cstotal.cs_nbfree;
    files = check_maxino - ROOTINO - check_sblk.b_un.b_fs->fs_cstotal.cs_nifree - check_n_files;
    blks = check_n_blks +
        check_sblk.b_un.b_fs->fs_ncg * (cgdmin(check_sblk.b_un.b_fs, 0) - cgsblock(check_sblk.b_un.b_fs, 0));
    blks += cgsblock(check_sblk.b_un.b_fs, 0) - cgbase(check_sblk.b_un.b_fs, 0);
    blks += howmany(check_sblk.b_un.b_fs->fs_cssize, check_sblk.b_un.b_fs->fs_fsize);
    blks = check_maxfsblock - (n_ffree + check_sblk.b_un.b_fs->fs_frag * n_bfree) - blks;
    check_warn("%ld files, %jd used, %ju free ",
        (long)check_n_files, (intmax_t)check_n_blks,
        (uintmax_t)(n_ffree + check_sblk.b_un.b_fs->fs_frag * n_bfree));
    printf("(%ju frags, %ju blocks, %.1f%% fragmentation)\n",
        (uintmax_t)n_ffree, (uintmax_t)n_bfree,
        n_ffree * 100.0 / check_sblk.b_un.b_fs->fs_dsize);
    if (verbose) {
        if (files < 0)
            printf("%jd inodes missing\n", -files);
        if (blks < 0)
            printf("%jd blocks missing\n", -blks);
        if (check_duplist != NULL) {
            printf("The following duplicate blocks remain:");
            for (dp = check_duplist; dp; dp = dp->next)
                printf(" %jd,", (intmax_t)dp->dup);
            printf("\n");
        }
    }
    check_duplist = (struct dups *)0;
    check_muldup = (struct dups *)0;
    check_inocleanup();
    if (check_fsmodified) {
        check_sblk.b_un.b_fs->fs_time = time(NULL);
        dirty(&check_sblk);
    }
    if (check_cvtlevel && check_sblk.b_dirty) {
        /*
         * Write out the duplicate super blocks
         */
        for (cylno = 0; cylno < check_sblk.b_un.b_fs->fs_ncg; cylno++)
            check_blwrite(check_fswritefd, (char *)check_sblk.b_un.b_fs,
                fsbtodb(check_sblk.b_un.b_fs, cgsblock(check_sblk.b_un.b_fs, cylno)),
                SBLOCKSIZE);
    }
    if (check_rerun)
        check_resolved = 0;

    /*
     * Check to see if the file system is mounted read-write.
     */
    check_finish(check_resolved);

    for (cylno = 0; cylno < check_sblk.b_un.b_fs->fs_ncg; cylno++)
        if (check_inostathead[cylno].il_stat != NULL)
            free((char *)check_inostathead[cylno].il_stat);
    free((char *)check_inostathead);
    check_inostathead = NULL;
    if (check_fsmodified)
        printf("\n***** FILE SYSTEM WAS MODIFIED *****\n");
    if (check_rerun)
        printf("\n***** PLEASE RERUN FSCK *****\n");
}
