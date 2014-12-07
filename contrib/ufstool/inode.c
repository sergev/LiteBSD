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
 */

#include <sys/param.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <inttypes.h>

#include "libufs.h"
#include "dir.h"
#include "internal.h"

extern int verbose;

int
ufs_inode_get (ufs_t *disk, ufs_inode_t *inode, unsigned inum)
{
    struct fs *fs = &disk->d_fs;
    unsigned bno;
    int64_t offset;
    struct ufs1_dinode buf;

    if (disk->d_ufs != 1) {
        fprintf(stderr, "%s: only UFS1 format supported\n", __func__);
        exit(-1);
    }
    memset (inode, 0, sizeof (*inode));
    inode->disk = disk;
    inode->number = inum;
    if (inum == 0) {
        fprintf(stderr, "%s: bad inode number = %u\n", __func__, inum);
        return -1;
    }

    /* Inodes are numbered starting from 1. */
    bno = ino_to_fsba(fs, inum);
    offset = lfragtosize(fs, bno) +
        (ino_to_fsbo(fs, inum) * sizeof(struct ufs1_dinode));
    offset += disk->d_part_offset;
//printf("--- %s(inum = %u) bno=%u, offset=%ju, d_secsize=%lu \n", __func__, inum, bno, (uintmax_t) offset, disk->d_secsize);
    if (bno >= fs->fs_old_size) {
        fprintf(stderr, "%s: bad block number %u\n", __func__, bno);
        return -1;
    }

    if (pread (disk->d_fd, &buf, sizeof(buf), offset) != sizeof(buf)) {
        fprintf(stderr, "%s: read error at offset %jd, inode %u\n",
            __func__, (intmax_t)offset, inum);
        return -1;
    }
    inode->mode   = buf.di_mode;
    inode->nlink  = buf.di_nlink;
    inode->size   = buf.di_size;
    inode->atime  = buf.di_atime;
    inode->mtime  = buf.di_mtime;
    inode->ctime  = buf.di_ctime;
    inode->flags  = buf.di_flags;
    inode->blocks = buf.di_blocks;
    inode->uid    = buf.di_uid;
    inode->gid    = buf.di_gid;
    memcpy (inode->daddr, buf.di_db, sizeof(buf.di_db));
    memcpy (inode->iaddr, buf.di_ib, sizeof(buf.di_ib));
/*if (inode->mode) { ufs_inode_print (inode, stdout); printf ("---\n"); }*/
    if (verbose > 3)
        printf ("get inode %u\n", inode->number);
    return 0;
}

int
ufs_inode_save (ufs_inode_t *inode, int force)
{
    ufs_t *disk = inode->disk;
    struct fs *fs = &disk->d_fs;
    int64_t offset;
    unsigned bno;
    struct ufs1_dinode buf;

    if (disk->d_ufs != 1) {
        fprintf(stderr, "%s: Only UFS1 format supported\n", __func__);
        exit(-1);
    }
    if (! disk->d_writable)
        return -1;
    if (! force && ! inode->dirty)
        return 0;
    if (inode->number == 0) {
        fprintf(stderr, "%s: bad inode number = %u\n", __func__, inode->number);
        return -1;
    }

    /* Inodes are numbered starting from 1. */
    bno = ino_to_fsba(fs, inode->number);
    offset = lfragtosize(fs, bno) +
        (ino_to_fsbo(fs, inode->number) * sizeof(struct ufs1_dinode));
    offset += disk->d_part_offset;
//printf("--- %s(inum = %u) bno=%u, offset=%ju, d_secsize=%lu \n", __func__, inum, bno, (uintmax_t) offset, disk->d_secsize);
    if (bno >= fs->fs_old_size) {
        fprintf(stderr, "%s: bad block number %u\n", __func__, bno);
        return -1;
    }

    memset (&buf, 0, sizeof(buf));
    buf.di_mode   = inode->mode;
    buf.di_nlink  = inode->nlink;
    buf.di_size   = inode->size;
    buf.di_atime  = inode->atime = time(NULL);
    buf.di_mtime  = inode->mtime;
    buf.di_ctime  = inode->ctime;
    buf.di_flags  = inode->flags;
    buf.di_blocks = inode->blocks;
    buf.di_uid    = inode->uid;
    buf.di_gid    = inode->gid;
    memcpy (buf.di_db, inode->daddr, sizeof(buf.di_db));
    memcpy (buf.di_ib, inode->iaddr, sizeof(buf.di_ib));

    if (pwrite (disk->d_fd, &buf, sizeof(buf), offset) != sizeof(buf)) {
        fprintf(stderr, "%s: write error at offset %jd, inode %u\n",
            __func__, (intmax_t)offset, inode->number);
        return -1;
    }
    inode->dirty = 0;
    if (verbose > 3)
        printf ("save inode %u\n", inode->number);
    return 0;
}

/*
 * Free an indirect block.
 */
static int
free_indirect_block (ufs_inode_t *inode, unsigned int bno, int nblk)
{
    struct fs *fs = &inode->disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned nb;
    unsigned char data [MAXBSIZE];
    int i;

    if (ufs_sector_read (inode->disk, fsbtodb(fs, bno), data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return 0;
    }
    for (i=bsize-4; i>=0; i-=4) {
        if (i/4 < nblk) {
            /* Truncate up to required size. */
            return 0;
        }
        nb = data [i+3] << 24 | data [i+2] << 16 |
             data [i+1] << 8  | data [i];
        if (nb) {
            ufs_block_free (inode->disk, nb);
            inode->blocks -= bsize / 512;
        }
    }
    ufs_block_free (inode->disk, bno);
    return 1;
}

/*
 * Free a double indirect block.
 */
static int
free_double_indirect_block (ufs_inode_t *inode, unsigned int bno, int nblk)
{
    struct fs *fs = &inode->disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned nb;
    unsigned char data [MAXBSIZE];
    int i;

    if (ufs_sector_read (inode->disk, fsbtodb(fs, bno), data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return 0;
    }
    for (i=bsize-4; i>=0; i-=4) {
        if (i/4 * bsize/4 < nblk) {
            /* Truncate up to required size. */
            return 0;
        }
        nb = data [i+3] << 24 | data [i+2] << 16 |
             data [i+1] << 8  | data [i];
        if (nb) {
            free_indirect_block (inode, nb, nblk - i/4 * bsize/4);
            inode->blocks -= bsize / 512;
        }
    }
    ufs_block_free (inode->disk, bno);
    return 1;
}

/*
 * Free a triple indirect block.
 */
static int
free_triple_indirect_block (ufs_inode_t *inode, unsigned int bno, int nblk)
{
    struct fs *fs = &inode->disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned nb;
    unsigned char data [MAXBSIZE];
    int i;

    if (ufs_sector_read (inode->disk, fsbtodb(fs, bno), data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return 0;
    }
    for (i=bsize-4; i>=0; i-=4) {
        if (i/4 * bsize/4 * bsize/4 < nblk) {
            /* Truncate up to required size. */
            return 0;
        }
        nb = data [i+3] << 24 | data [i+2] << 16 |
             data [i+1] << 8  | data [i];
        if (nb) {
            free_double_indirect_block (inode, nb,
                nblk - i/4 * bsize/4 * bsize/4);
            inode->blocks -= bsize / 512;
        }
    }
    ufs_block_free (inode->disk, bno);
    return 1;
}

/*
 * Free all the disk blocks associated
 * with the specified inode structure.
 */
void ufs_inode_truncate (ufs_inode_t *inode, unsigned long size)
{
    unsigned bsize = inode->disk->d_fs.fs_bsize;
    int i, nblk;
    ufs1_daddr_t *blk;

    if ((inode->mode & IFMT) == IFCHR ||
        (inode->mode & IFMT) == IFBLK)
        return;

    nblk = (size + bsize - 1) / bsize;
    for (i=NDADDR-1; i>=0; --i) {
        blk = &inode->daddr[i];
        if (*blk != 0) {
            if (i * bsize < size)
                break;
            ufs_block_free (inode->disk, *blk);
            *blk = 0;
        }
    }

#define SINGLE  0       /* index of single indirect block */
#define DOUBLE  1       /* index of double indirect block */
#define TRIPLE  2       /* index of triple indirect block */

    for (i=NIADDR-1; i>=0; --i) {
        blk = &inode->iaddr[i];
        if (*blk != 0) {
            if (i == TRIPLE) {
                if (free_triple_indirect_block (inode, *blk,
                    nblk - (NDADDR + bsize/4 + bsize/4*bsize/4)) < 0)
                    break;
            } else if (i == DOUBLE) {
                if (free_double_indirect_block (inode, *blk,
                    nblk - (NDADDR + bsize/4)) < 0)
                    break;
            } else /*if (i == SINGLE)*/ {
                if (free_indirect_block (inode, *blk, nblk - NDADDR) < 0)
                    break;
            }
            *blk = 0;
            inode->blocks -= bsize / 512;
        }
    }

    inode->size = size;
    inode->dirty = 1;
}

void ufs_inode_clear (ufs_inode_t *inode)
{
    inode->dirty = 1;
    inode->mode = 0;
    inode->nlink = 0;
    inode->uid = 0;
    inode->gid = 0;
    inode->size = 0;
    inode->blocks = 0;
    memset (inode->daddr, 0, sizeof(inode->daddr));
    memset (inode->iaddr, 0, sizeof(inode->iaddr));
    inode->flags = 0;
    inode->atime = 0;
    inode->mtime = 0;
    inode->ctime = 0;
}

void
ufs_inode_print (ufs_inode_t *inode, FILE *out)
{
    int i;

    fprintf (out, "     I-node: %u\n", inode->number);
    fprintf (out, "       Type: %s\n",
        (inode->mode & IFMT) == IFDIR ? "Directory" :
        (inode->mode & IFMT) == IFCHR ? "Character device" :
        (inode->mode & IFMT) == IFBLK ? "Block device" :
        (inode->mode & IFMT) == IFREG ? "File" :
        (inode->mode & IFMT) == IFLNK ? "Symbolic link" :
        (inode->mode & IFMT) == IFSOCK? "Socket" :
        (inode->mode & IFMT) == IFWHT ? "Whiteout" :
        "Unknown");
    fprintf (out, "       Size: %ju bytes\n", (uintmax_t)inode->size);
    fprintf (out, "       Mode: %#o\n", inode->mode);

    fprintf (out, "            ");
    if (inode->mode & ISUID)  fprintf (out, " SUID");
    if (inode->mode & ISGID)  fprintf (out, " SGID");
    if (inode->mode & ISVTX)  fprintf (out, " SVTX");
    if (inode->mode & IREAD)  fprintf (out, " READ");
    if (inode->mode & IWRITE) fprintf (out, " WRITE");
    if (inode->mode & IEXEC)  fprintf (out, " EXEC");
    fprintf (out, "\n");

    fprintf (out, "      Links: %u\n", inode->nlink);
    fprintf (out, "Owner:Group: %u:%u\n", inode->uid, inode->gid);

    fprintf (out, "     Blocks:");
    for (i=0; i<NDADDR; ++i) {
        fprintf (out, " %u", inode->daddr[i]);
    }
    for (i=0; i<NIADDR; ++i) {
        fprintf (out, " %u", inode->iaddr[i]);
    }
    fprintf (out, "\n");

    time_t t = inode->ctime;
    fprintf (out, "    Created: %s", ctime(&t));
    t = inode->mtime;
    fprintf (out, "   Modified: %s", ctime(&t));
    t = inode->atime;
    fprintf (out, "Last access: %s", ctime(&t));
}

void
ufs_inode_print_path (ufs_inode_t *inode,
    const char *dirname, const char *filename, FILE *out)
{
    fprintf (out, "%s/%s", dirname, filename);
    switch (inode->mode & IFMT) {
    case IFDIR:
        if (filename[0] != 0)
            fprintf (out, "/");
        break;
    case IFCHR:
        fprintf (out, " - char %d %d",
            inode->daddr[0] >> 8, inode->daddr[0] & 0xff);
        break;
    case IFBLK:
        fprintf (out, " - block %d %d",
            inode->daddr[0] >> 8, inode->daddr[0] & 0xff);
        break;
    default:
        fprintf (out, " - %ju bytes", (uintmax_t)inode->size);
        break;
    }
    fprintf (out, "\n");
}

static void
print_indirect_block (ufs_t *disk, unsigned int bno, FILE *out)
{
    struct fs *fs = &disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned inodes_per_block = INOPB(fs);
    unsigned nb;
    int32_t data [MAXBSIZE/4];
    int i;

    fprintf (out, " [%d]", bno);
    if (ufs_sector_read (disk, fsbtodb(fs, bno), data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return;
    }
    for (i=0; i<inodes_per_block; i++) {
        nb = data [i];
        if (nb)
            fprintf (out, " %d", nb);
    }
}

static void
print_double_indirect_block (ufs_t *disk, unsigned int bno, FILE *out)
{
    struct fs *fs = &disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned inodes_per_block = INOPB(fs);
    unsigned nb;
    int32_t data [MAXBSIZE/4];
    int i;

    fprintf (out, " [%d]", bno);
    if (ufs_sector_read (disk, fsbtodb(fs, bno), data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return;
    }
    for (i=0; i<inodes_per_block; i++) {
        nb = data [i];
        if (nb)
            print_indirect_block (disk, nb, out);
    }
}

static void
print_triple_indirect_block (ufs_t *disk, unsigned int bno, FILE *out)
{
    struct fs *fs = &disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned inodes_per_block = INOPB(fs);
    unsigned nb;
    int32_t data [MAXBSIZE/4];
    int i;

    fprintf (out, " [%d]", bno);
    if (ufs_sector_read (disk, fsbtodb(fs, bno), data, bsize) < 0) {
        fprintf (stderr, "%s: read error at block %d\n", __func__, bno);
        return;
    }
    for (i=0; i<inodes_per_block; i++) {
        nb = data [i];
        if (nb)
            print_indirect_block (disk, nb, out);
    }
}

void
ufs_inode_print_blocks (ufs_inode_t *inode, FILE *out)
{
    int i;

    if ((inode->mode & IFMT) == IFCHR ||
        (inode->mode & IFMT) == IFBLK)
        return;

    fprintf (out, "    ");
    for (i=0; i<NDADDR; ++i) {
        if (inode->daddr[i] == 0)
            continue;
        fprintf (out, " %d", inode->daddr[i]);
    }
    if (inode->iaddr[0] != 0)
        print_indirect_block (inode->disk, inode->iaddr[0], out);
    if (inode->iaddr[1] != 0)
        print_double_indirect_block (inode->disk, inode->iaddr[1], out);
    if (inode->iaddr[2] != 0)
        print_triple_indirect_block (inode->disk, inode->iaddr[2], out);
    fprintf (out, "\n");
}

void ufs_directory_scan (ufs_inode_t *dir, const char *dirname,
    ufs_directory_scanner_t scanner, void *arg)
{
    ufs_inode_t file;
    unsigned long offset;
    unsigned char name [MAXBSIZE - 12];
    struct direct dirent;

    /* Variable record per file */
    for (offset = 0; offset < dir->size; offset += dirent.d_reclen) {
        if (ufs_inode_read (dir, offset, (unsigned char*) &dirent, 8) < 0) {
            fprintf (stderr, "%s: read error at offset %ld\n",
                dirname[0] ? dirname : "/", offset);
            return;
        }
        if (dirent.d_ino == 0)
            continue;
//printf ("--- scan offset %lu: ino=%u, reclen=%u, type=%u, namlen=%u\n", offset, dirent.d_ino, dirent.d_reclen, dirent.d_type, dirent.d_namlen);
        if (ufs_inode_read (dir, offset+8, name, (dirent.d_namlen + 4) / 4 * 4) < 0) {
            fprintf (stderr, "%s: name read error at offset %ld\n",
                dirname[0] ? dirname : "/", offset);
            return;
        }
//printf ("--- scan offset %lu: name='%s'\n", offset, name);

        if ((name[0]=='.' && name[1]==0) ||
            (name[0]=='.' && name[1]=='.' && name[2]==0))
            continue;

        if (ufs_inode_get (dir->disk, &file, dirent.d_ino) < 0) {
            fprintf (stderr, "cannot scan inode %d\n", dirent.d_ino);
            continue;
        }
        scanner (dir, &file, dirname, (char*) name, arg);
    }
}

/*
 * Return the physical block number on a device given the
 * inode and the logical block number in a file.
 */
static unsigned
map_block (ufs_inode_t *inode, unsigned lbn)
{
    struct fs *fs = &inode->disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned block [MAXBSIZE / 4];
    unsigned int nb, i, j, sh;
    unsigned nshift = ffs(NINDIR(fs)) - 1;
    unsigned nmask = NINDIR(fs) - 1;

    /*
     * Blocks 0..NDADDR-1 are direct blocks.
     */
    if (lbn < NDADDR) {
        /* small file algorithm */
        return inode->daddr [lbn];
    }

    /*
     * Addresses NIADDR..NIADDR+2
     * have single, double, triple indirect blocks.
     * The first step is to determine
     * how many levels of indirection.
     */
    sh = 0;
    nb = 1;
    lbn -= NDADDR;
    for (j=NIADDR; ; j--) {
        if (j == 0)
            return 0;
        sh += nshift;
        nb <<= nshift;
        if (lbn < nb)
            break;
        lbn -= nb;
    }

    /*
     * Fetch the first indirect block.
     */
    nb = inode->iaddr [NIADDR-j];
    if (nb == 0)
        return 0;

    /*
     * Fetch through the indirect blocks.
     */
    for(; j <= 3; j++) {
        if (ufs_sector_read (inode->disk, fsbtodb(fs, nb),
            (unsigned char*) block, bsize) < 0)
            return 0;

        sh -= nshift;
        i = (lbn >> sh) & nmask;
        nb = block [i];
        if (nb == 0)
            return 0;
    }
    return nb;
}

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 */
static unsigned
map_block_write (ufs_inode_t *inode, unsigned lbn)
{
    struct fs *fs = &inode->disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned block [MAXBSIZE / 4];
    unsigned int sh, i, j;
    unsigned nshift = ffs(NINDIR(fs)) - 1;
    unsigned nmask = NINDIR(fs) - 1;
    ufs1_daddr_t nb, newb, bpref;

    /*
     * Blocks 0..NDADDR-1 are direct blocks.
     */
    if (lbn < NDADDR) {
        /* small file algorithm */
        nb = inode->daddr [lbn];
        if (nb != 0) {
            if (verbose)
                printf ("map logical block %d to physical %d\n", lbn, nb);
            return nb;
        }

        /* allocate new block */
        bpref = (lbn > 0) ? inode->daddr [lbn-1] : ino_to_fsba(fs, inode->number);
        if (ufs_block_alloc (inode->disk, bpref, &nb) < 0)
            return 0;
        inode->daddr[lbn] = nb;
        inode->blocks += bsize / 512;
        inode->dirty = 1;
        return nb;
    }

    /*
     * Addresses NIADDR..NIADDR+2
     * have single, double, triple indirect blocks.
     * The first step is to determine
     * how many levels of indirection.
     */
    sh = 0;
    nb = 1;
    lbn -= NDADDR;
    for (j=NIADDR; ; j--) {
        if (j == 0)
            return 0;
        sh += nshift;
        nb <<= nshift;
        if (lbn < nb)
            break;
        lbn -= nb;
    }

    /*
     * Fetch the first indirect block.
     */
    nb = inode->iaddr [NIADDR-j];
    if (nb == 0) {
        bpref = inode->daddr[NDADDR-1];
        if (ufs_block_alloc (inode->disk, bpref, &nb) < 0)
            return 0;
        if (verbose)
            printf ("inode %d: allocate new block %d\n", inode->number, nb);
        memset (block, 0, bsize);
        if (ufs_sector_write (inode->disk, fsbtodb(fs, nb),
            (unsigned char*) block, bsize) < 0)
            return 0;
        inode->iaddr [NIADDR-j] = nb;
        inode->blocks += bsize / 512;
        inode->dirty = 1;
    }

    /*
     * Fetch through the indirect blocks
     */
    for(; j <= 3; j++) {
        if (ufs_sector_read (inode->disk, fsbtodb(fs, nb),
            (unsigned char*) block, bsize) < 0)
            return 0;

        sh -= nshift;
        i = (lbn >> sh) & nmask;
        if (block [i] != 0)
            nb = block [i];
        else {
            /* Allocate new block. */
            if (ufs_block_alloc (inode->disk, nb, &newb) < 0)
                return 0;
            if (verbose)
                printf ("inode %d: allocate new block %d\n", inode->number, newb);
            block[i] = newb;
            if (ufs_sector_write (inode->disk, fsbtodb(fs, nb),
                (unsigned char*) block, bsize) < 0)
                return 0;
            memset (block, 0, bsize);
            if (ufs_sector_write (inode->disk, fsbtodb(fs, newb),
                (unsigned char*) block, bsize) < 0)
                return 0;
            nb = newb;
            inode->blocks += bsize / 512;
            inode->dirty = 1;
        }
    }
    return nb;
}

int
ufs_inode_read (ufs_inode_t *inode, unsigned long offset,
    unsigned char *data, unsigned long bytes)
{
    struct fs *fs = &inode->disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned char block [MAXBSIZE];
    unsigned long n;
    unsigned int bn, inblock_offset;

    if (bytes + offset > inode->size)
        return -1;
    while (bytes != 0) {
        inblock_offset = offset % bsize;
        n = bsize - inblock_offset;
        if (n > bytes)
            n = bytes;

//printf ("--- %s: offset %lu -> block %lu, inblock_offset=%u, bytes=%lu \n", __func__, offset, offset / bsize, inblock_offset, n);
        bn = map_block (inode, offset / bsize);
//printf ("---     map_block returned bn=%u \n", bn);
        if (bn == 0)
            return -1;

        if (ufs_sector_read (inode->disk, fsbtodb(fs, bn), block, bsize) < 0)
            return -1;
        memcpy (data, block + inblock_offset, n);
        data += n;
        offset += n;
        bytes -= n;
    }
    return 0;
}

int
ufs_inode_write (ufs_inode_t *inode, unsigned long offset,
    unsigned char *data, unsigned long bytes)
{
    struct fs *fs = &inode->disk->d_fs;
    unsigned bsize = fs->fs_bsize;
    unsigned char block [MAXBSIZE];
    unsigned long n;
    unsigned int bn, inblock_offset;

    inode->mtime = time(NULL);
    while (bytes != 0) {
        inblock_offset = offset % bsize;
        n = bsize - inblock_offset;
        if (n > bytes)
            n = bytes;

//printf ("--- %s: offset %lu -> block %lu, inblock_offset=%u, bytes=%lu \n", __func__, offset, offset / bsize, inblock_offset, n);
        bn = map_block_write (inode, offset / bsize);
//printf ("---     map_block returned bn=%u \n", bn);
        if (bn == 0)
            return -1;
        if (inode->size < offset + n) {
            /* Increase file size. */
            inode->size = offset + n;
            inode->dirty = 1;
        }
        if (verbose)
            printf ("inode %d offset %ld: write %ld bytes to block %d\n",
                inode->number, offset, n, bn);

        if (n == bsize) {
            if (ufs_sector_write (inode->disk, fsbtodb(fs, bn), data, bsize) < 0)
                return -1;
        } else {
            if (ufs_sector_read (inode->disk, fsbtodb(fs, bn), block, bsize) < 0)
                return -1;
            memcpy (block + inblock_offset, data, n);
            if (ufs_sector_write (inode->disk, fsbtodb(fs, bn), block, bsize) < 0)
                return -1;
        }
        data += n;
        offset += n;
        bytes -= n;
    }
    return 0;
}

/*
 * Put the specified inode back in the free map.
 */
static int
ffs_inode_free (ufs_t *disk, ufs_ino_t ino, mode_t mode)
{
    struct fs *fs = &disk->d_fs;
    struct cg *cgp = &disk->d_cg;
    int error, cg;

    if ((u_int)ino >= fs->fs_ipg * fs->fs_ncg) {
        fprintf (stderr, "%s: inode index out of range: ino = %u, fs = %s\n",
            __func__, ino, fs->fs_fsmnt);
        exit(-1);
    }
    cg = ino_to_cg(fs, ino);
    error = ufs_cgroup_read(disk, cg);
    if (error < 0) {
        return error;
    }
    if (!cg_chkmagic(cgp)) {
        fprintf (stderr, "%s: bad magic of cgroup %d\n", __func__, cg);
        return (0);
    }
    cgp->cg_time = time(NULL);
    ino %= fs->fs_ipg;
    if (isclr(cg_inosused(cgp), ino)) {
        fprintf (stderr, "%s: freeing free inode: ino = %u, fs = %s\n",
            __func__, ino, fs->fs_fsmnt);
        exit(-1);
    }
    clrbit(cg_inosused(cgp), ino);
    if (ino < cgp->cg_irotor)
        cgp->cg_irotor = ino;
    cgp->cg_cs.cs_nifree++;
    fs->fs_cstotal.cs_nifree++;
    fs->fs_cs(fs, cg).cs_nifree++;
    if ((mode & IFMT) == IFDIR) {
        cgp->cg_cs.cs_ndir--;
        fs->fs_cstotal.cs_ndir--;
        fs->fs_cs(fs, cg).cs_ndir--;
    }
    fs->fs_fmod = 1;
    ufs_cgroup_write_last(disk);
    return (0);
}

/*
 * Write zeroes to inode.
 */
static int
inode_write_zeros (ufs_inode_t *inode, unsigned offset, int nbytes)
{
    static unsigned char zeroes[DIRBLKSIZ];

    if (nbytes < 0) {
        fprintf (stderr, "%s: bad length %d bytes\n", __func__, nbytes);
        return -1;
    }
    if (nbytes > 0) {
        if (ufs_inode_write (inode, offset, zeroes, nbytes) < 0) {
            fprintf (stderr, "%s: inode %d: write error at offset %d\n",
                __func__, inode->number, offset);
            return -1;
        }
    }
    return 0;
}

/*
 * Convert a pathname into a pointer to an inode.
 * op =
 *  INODE_OP_LOOKUP if name is saught
 *  INODE_OP_CREATE if name is to be created, mode is given
 *  INODE_OP_DELETE if name is to be deleted
 *  INODE_OP_LINK   if name is to be linked, mode contains inode number
 *
 * Return -1 on any error.
 * Return 0 when the inode was found.
 * Return 1 when the inode was created/deleted/linked.
 */
typedef enum {
    INODE_OP_LOOKUP,                    /* lookup inode by name */
    INODE_OP_CREATE,                    /* create new file */
    INODE_OP_DELETE,                    /* delete file */
    INODE_OP_LINK,                      /* make a link to a file */
} ufs_op_t;

static int
inode_by_name (ufs_t *disk, ufs_inode_t *inode, const char *name,
    ufs_op_t op, int mode, int link_inum)
{
    ufs_inode_t dir;
    int c, namlen, reclen;
    const char *namptr;
    unsigned long offset, last_offset;
    struct direct dirent;

    /* Start from root. */
    if (ufs_inode_get (disk, &dir, ROOTINO) < 0) {
        fprintf (stderr, "%s: cannot get root\n", __func__);
        return -1;
    }
    c = *name++;
    while (c == '/')
        c = *name++;
    if (! c && op != INODE_OP_LOOKUP) {
        /* Cannot write or delete root directory. */
        return -1;
    }
cloop:
    /* Here inode contains pointer
     * to last component matched. */
    if (! c) {
        *inode = dir;
        return 0;
    }

    /* If there is another component,
     * inode must be a directory. */
    if ((dir.mode & IFMT) != IFDIR) {
        return -1;
    }

    /* Gather up dir name into buffer. */
    namptr = name - 1;
    while (c && c != '/') {
        c = *name++;
    }
    namlen = name - namptr - 1;
    while (c == '/')
        c = *name++;

    /* Search a directory, variable record per file */
    if (verbose > 2)
        printf ("scan for '%.*s', %d bytes\n", namlen, namptr, namlen);
    last_offset = 0;
    for (offset = 0; offset < dir.size; last_offset = offset, offset += dirent.d_reclen) {
        unsigned char fname [DIRBLKSIZ];

        if (ufs_inode_read (&dir, offset, (unsigned char*) &dirent, 8) < 0) {
            fprintf (stderr, "inode %d: read error at offset %ld\n",
                dir.number, offset);
            return -1;
        }
        if (verbose > 2)
            printf ("scan offset %lu: inum=%u, reclen=%u, namlen=%u\n", offset, dirent.d_ino, dirent.d_reclen, dirent.d_namlen);
        if (dirent.d_reclen == 0) {
            fprintf (stderr, "inode %d: zero length record detected\n", dir.number);
            return -1;
        }
        if (dirent.d_ino == 0)
            continue;
        if (ufs_inode_read (&dir, offset+8, fname, (dirent.d_namlen + 4) / 4 * 4) < 0) {
            fprintf (stderr, "inode %d: name read error at offset %ld\n",
                dir.number, offset);
            return -1;
        }
        if (verbose > 2)
            printf ("scan offset %lu: name='%.*s'\n", offset, dirent.d_namlen, fname);
        if (dirent.d_namlen == namlen &&
            memcmp (namptr, (char*) fname, namlen) == 0) {
            /* Here a component matched in a directory.
             * If there is more pathname, go back to
             * cloop, otherwise return. */
            if (op == INODE_OP_DELETE && ! c) {
                goto delete_file;
            }
            if (ufs_inode_get (disk, &dir, dirent.d_ino) < 0) {
                fprintf (stderr, "inode_open(): cannot get inode %d\n", dirent.d_ino);
                return -1;
            }
            goto cloop;
        }
    }
    /* If at the end of the directory, the search failed.
     * Report what is appropriate as per flag. */
    if (op == INODE_OP_CREATE && ! c)
        goto create_file;
    if (op == INODE_OP_LINK && ! c)
        goto create_link;
    return -1;

    /*
     * Make a new file, and return it's inode.
     */
create_file:
    if (ufs_inode_alloc (&dir, mode, inode) < 0) {
        fprintf (stderr, "%s: cannot allocate inode\n", namptr);
        return -1;
    }
    inode->dirty = 1;
    inode->mode = mode & (07777 | IFMT);
    if ((inode->mode & IFMT) == 0)
        inode->mode |= IFREG;
    inode->nlink = 1;
    inode->uid = 0;
    inode->flags = 0;
    inode->ctime = time(0);
    if ((inode->mode & IFMT) == IFDIR) {
        /* Make link '.' */
        struct direct dotent;
        dotent.d_ino = inode->number;
        dotent.d_reclen = DIRBLKSIZ;
        dotent.d_namlen = 1;
        dotent.d_type = IFDIR >> 12;
        memcpy (dotent.d_name, ".\0\0\0", 4);
        if (ufs_inode_write (inode, 0, (unsigned char*) &dotent, sizeof(dotent)) < 0) {
            fprintf (stderr, "inode %d: write error at offset %ld\n",
                inode->number, 0L);
            return -1;
        }
        /* Increase file size. */
        inode->size = DIRBLKSIZ;
        ++inode->nlink;
    }
    if (ufs_inode_save (inode, 0) < 0) {
        fprintf (stderr, "%s: cannot save file inode\n", namptr);
        return -1;
    }

    /* Write a directory entry. */
    if (verbose > 2)
        printf ("*** create file '%.*s', inode %d\n", namlen, namptr, inode->number);
    reclen = dirent.d_reclen - 8 - (dirent.d_namlen + 4) / 4 * 4;
    c = 8 + (namlen + 4) / 4 * 4;
    if (reclen >= c) {
        /* Enough space */
        dirent.d_reclen -= reclen;
        if (verbose > 2)
            printf ("*** previous entry %u-%u-%u at offset %lu\n",
                dirent.d_ino, dirent.d_reclen, dirent.d_namlen, last_offset);
        if (ufs_inode_write (&dir, last_offset, (unsigned char*) &dirent, 8) < 0) {
            fprintf (stderr, "inode %d: write error at offset %ld\n",
                dir.number, last_offset);
            return -1;
        }
    } else {
        /* No space, extend directory. */
        if (verbose > 2)
            printf ("*** extend dir, previous entry %u-%u-%u at offset %lu\n",
                dirent.d_ino, dirent.d_reclen, dirent.d_namlen, last_offset);
        reclen = DIRBLKSIZ;
    }
    offset = last_offset + dirent.d_reclen;
    dirent.d_ino = inode->number;
    dirent.d_reclen = reclen;
    dirent.d_namlen = namlen;
    dirent.d_type = (mode & IFMT) >> 12;
    if (verbose > 2)
        printf ("*** new entry %u-%u-%u at offset %lu\n", dirent.d_ino, dirent.d_reclen, dirent.d_namlen, offset);
    if (ufs_inode_write (&dir, offset, (unsigned char*) &dirent, 8) < 0) {
        fprintf (stderr, "inode %d: write error at offset %ld\n",
            dir.number, offset);
        return -1;
    }
    if (verbose > 2)
        printf ("*** name '%.*s' at offset %lu\n", namlen, namptr, offset+8);
    if (ufs_inode_write (&dir, offset+8, (unsigned char*) namptr, namlen) < 0) {
        fprintf (stderr, "inode %d: write error at offset %ld\n",
            dir.number, offset+8);
        return -1;
    }
    if (inode_write_zeros (&dir, offset+8+namlen, reclen-8-namlen) < 0) {
        fprintf (stderr, "inode %d: write error at offset %ld\n",
            dir.number, offset+8+namlen);
        return -1;
    }
    /* Align directory size. */
    dir.size = (dir.size + DIRBLKSIZ - 1) / DIRBLKSIZ * DIRBLKSIZ;
    if (ufs_inode_save (&dir, 0) < 0) {
        fprintf (stderr, "%s: cannot save directory inode\n", namptr);
        return -1;
    }
    return 1;

    /*
     * Delete file. Return inode of deleted file.
     */
delete_file:
    if (verbose > 2)
        printf ("*** delete inode %d\n", dirent.d_ino);
    if (ufs_inode_get (disk, inode, dirent.d_ino) < 0) {
        fprintf (stderr, "%s: cannot get inode %d\n", namptr, dirent.d_ino);
        return -1;
    }
    inode->dirty = 1;
    inode->nlink--;
    if (inode->nlink <= 0) {
        ufs_inode_truncate (inode, 0);
        ffs_inode_free (disk, dirent.d_ino, inode->mode);
        ufs_inode_clear (inode);
    }
    /* Extend previous entry to cover the empty space. */
    reclen = dirent.d_reclen;
    if (ufs_inode_read (&dir, last_offset, (unsigned char*) &dirent, 8) < 0) {
        fprintf (stderr, "inode %d: read error at offset %ld\n",
            dir.number, last_offset);
        return -1;
    }
    dirent.d_reclen += reclen;
    if (ufs_inode_write (&dir, last_offset, (unsigned char*) &dirent, 8) < 0) {
        fprintf (stderr, "inode %d: write error at offset %ld\n",
            dir.number, last_offset);
        return -1;
    }
    if (ufs_inode_save (&dir, 0) < 0) {
        fprintf (stderr, "%s: cannot save directory inode\n", namptr);
        return -1;
    }
    return 1;

    /*
     * Make a link. Return a directory inode.
     */
create_link:
    if (verbose > 2)
        printf ("*** link inode %d to '%.*s', directory %d\n", link_inum, namlen, namptr, dir.number);
    reclen = dirent.d_reclen - 8 - (dirent.d_namlen + 4) / 4 * 4;
    c = 8 + (namlen + 4) / 4 * 4;
    if (reclen >= c) {
        /* Enough space */
        dirent.d_reclen -= reclen;
        if (verbose > 2)
            printf ("*** previous entry %u-%u-%u at offset %lu\n",
                dirent.d_ino, dirent.d_reclen, dirent.d_namlen, last_offset);
        if (ufs_inode_write (&dir, last_offset, (unsigned char*) &dirent, 8) < 0) {
            fprintf (stderr, "inode %d: write error at offset %ld\n",
                dir.number, last_offset);
            return -1;
        }
    } else {
        /* No space, extend directory. */
        if (verbose > 2)
            printf ("*** extend dir, previous entry %u-%u-%u at offset %lu\n",
                dirent.d_ino, dirent.d_reclen, dirent.d_namlen, last_offset);
        reclen = DIRBLKSIZ;
    }
    offset = last_offset + dirent.d_reclen;
    dirent.d_ino = link_inum;
    dirent.d_reclen = reclen;
    dirent.d_namlen = namlen;
    dirent.d_type = (mode & IFMT) >> 12;
    if (verbose > 2)
        printf ("*** new entry %u-%u-%u at offset %lu\n", dirent.d_ino, dirent.d_reclen, dirent.d_namlen, offset);
    if (ufs_inode_write (&dir, offset, (unsigned char*) &dirent, 8) < 0) {
        fprintf (stderr, "inode %d: write error at offset %ld\n",
            dir.number, offset);
        return -1;
    }
    if (verbose > 2)
        printf ("*** name '%.*s' at offset %lu\n", namlen, namptr, offset+8);
    if (ufs_inode_write (&dir, offset+8, (unsigned char*) namptr, namlen) < 0) {
        fprintf (stderr, "inode %d: write error at offset %ld\n",
            dir.number, offset+8);
        return -1;
    }
    if (inode_write_zeros (&dir, offset+8+namlen, reclen-8-namlen) < 0) {
        fprintf (stderr, "inode %d: write error at offset %ld\n",
            dir.number, offset+8+namlen);
        return -1;
    }
    if (ufs_inode_save (&dir, 0) < 0) {
        fprintf (stderr, "%s: cannot save directory inode\n", namptr);
        return -1;
    }
    *inode = dir;
    return 1;
}

/*
 * Find inode by name.
 * Return 0 on any error.
 * Return 1 when the inode was found.
 */
int
ufs_inode_lookup (ufs_t *disk, ufs_inode_t *inode, const char *name)
{
    return inode_by_name (disk, inode, name, INODE_OP_LOOKUP, 0, 0);
}

/*
 * Create inode by name.
 * Return 0 on any error.
 * Return 1 when the inode was found.
 * Return 2 when the inode was created.
 */
int
ufs_inode_create (ufs_t *disk, ufs_inode_t *inode, const char *name, int mode)
{
    return inode_by_name (disk, inode, name, INODE_OP_CREATE, mode, 0);
}

/*
 * Delete inode by name.
 * Return 0 on any error.
 * Return 2 when the inode was created.
 */
int
ufs_inode_delete (ufs_t *disk, ufs_inode_t *inode, const char *name)
{
    return inode_by_name (disk, inode, name, INODE_OP_DELETE, 0, 0);
}

/*
 * Create a new link for the inode.
 * Return 0 on any error.
 * Return 2 when the inode was linked.
 */
int
ufs_inode_link (ufs_t *disk, ufs_inode_t *inode, const char *name, int inum, int mode)
{
    return inode_by_name (disk, inode, name, INODE_OP_LINK, mode, inum);
}

/*
 * Count a number of 0xff bytes in the array.
 */
static int
count_ff(int size, unsigned char *cp)
{
    unsigned char *end = &cp[size];

    while (cp < end && *cp == 0xff)
        ++cp;
    return end - cp;
}

/*
 * Determine whether an inode can be allocated.
 *
 * Check to see if an inode is available, and if it is,
 * allocate it using the following policy:
 *   1) allocate the requested inode.
 *   2) allocate the next available inode after the requested
 *      inode in the specified cylinder group.
 */
static ufs1_daddr_t
cg_alloc_inode(ufs_t *disk, int cg, ufs1_daddr_t ipref, int mode)
{
    struct fs *fs = &disk->d_fs;
    struct cg *cgp = &disk->d_cg;
    int start, len, loc, map, i;

    if (fs->fs_cs(fs, cg).cs_nifree == 0)
        return 0;
    if (ufs_cgroup_read(disk, cg) < 0) {
        return 0;
    }
    if (!cg_chkmagic(cgp) || cgp->cg_cs.cs_nifree == 0) {
        return 0;
    }
    cgp->cg_time = time(NULL);
    if (ipref) {
        ipref %= fs->fs_ipg;
        if (isclr(cg_inosused(cgp), ipref))
            goto gotit;
    }
    start = cgp->cg_irotor / NBBY;
    len = howmany(fs->fs_ipg - cgp->cg_irotor, NBBY);
    loc = count_ff(len, &cg_inosused(cgp)[start]);
    if (loc == 0) {
        len = start + 1;
        start = 0;
        loc = count_ff(len, &cg_inosused(cgp)[0]);
        if (loc == 0) {
            fprintf (stderr, "%s: map corrupted, cg = %d, irotor = %d, fs = %s\n",
                __func__, cg, cgp->cg_irotor, fs->fs_fsmnt);
            exit(-1);
        }
    }
    i = start + len - loc;
    map = cg_inosused(cgp)[i];
    ipref = i * NBBY;
    for (i = 1; i < (1 << NBBY); i <<= 1, ipref++) {
        if ((map & i) == 0) {
            cgp->cg_irotor = ipref;
            goto gotit;
        }
    }
    fprintf (stderr, "%s: block not in map, fs = %s\n",
        __func__, fs->fs_fsmnt);
    exit(-1);
gotit:
    setbit(cg_inosused(cgp), ipref);
    cgp->cg_cs.cs_nifree--;
    fs->fs_cstotal.cs_nifree--;
    fs->fs_cs(fs, cg).cs_nifree--;
    fs->fs_fmod = 1;
    if ((mode & IFMT) == IFDIR) {
            cgp->cg_cs.cs_ndir++;
            fs->fs_cstotal.cs_ndir++;
            fs->fs_cs(fs, cg).cs_ndir++;
    }
    ufs_cgroup_write_last(disk);
    return (cg * fs->fs_ipg + ipref);
}

/*
 * Find a cylinder to place a directory.
 *
 * The policy implemented by this algorithm is to select from
 * among those cylinder groups with above the average number of
 * free inodes, the one with the smallest number of directories.
 */
static ufs_ino_t
ffs_dirpref(fs)
    struct fs *fs;
{
    int cg, minndir, mincg, avgifree;

    avgifree = fs->fs_cstotal.cs_nifree / fs->fs_ncg;
    minndir = fs->fs_ipg;
    mincg = 0;
    for (cg = 0; cg < fs->fs_ncg; cg++) {
        if (fs->fs_cs(fs, cg).cs_ndir < minndir &&
            fs->fs_cs(fs, cg).cs_nifree >= avgifree)
        {
            mincg = cg;
            minndir = fs->fs_cs(fs, cg).cs_ndir;
        }
    }
    return (ufs_ino_t)(fs->fs_ipg * mincg);
}

/*
 * Allocate an inode in the file system.
 *
 * If allocating a directory, use ffs_dirpref to select the inode:
 *   1) allocate the preferred inode.
 *   2) allocate an inode in the same cylinder group.
 *   3) quadratically rehash into other cylinder groups, until an
 *      available inode is located.
 * If no inode preference is given the following hierarchy is used:
 *   1) allocate an inode in cylinder group 0.
 *   2) quadratically rehash into other cylinder groups, until an
 *      available inode is located.
 */
int
ufs_inode_alloc (ufs_inode_t *dir, int mode, ufs_inode_t *inode)
{
    struct fs *fs = &dir->disk->d_fs;
    ufs_ino_t ino, ipref;
    int cg, error;

    if (fs->fs_cstotal.cs_nifree == 0) {
noinodes:
        fprintf (stderr, "%s: create/symlink failed, no inodes free\n", __func__);
        return -ENOSPC;
    }
    if ((mode & IFMT) == IFDIR)
        ipref = ffs_dirpref(fs);
    else
        ipref = dir->number;

    if (ipref >= fs->fs_ncg * fs->fs_ipg)
        ipref = 0;
    cg = ino_to_cg(fs, ipref);

    ino = ufs_cgroup_hashalloc(dir->disk, cg, ipref, mode, cg_alloc_inode);
    if (ino == 0)
        goto noinodes;

    error = ufs_inode_get (dir->disk, inode, ino);
    if (error) {
        ffs_inode_free(dir->disk, ino, mode);
        return (error);
    }
    if (inode->mode) {
        fprintf (stderr, "%s: dup alloc: mode = 0%o, inum = %d, fs = %s\n",
            __func__, inode->mode, inode->number, fs->fs_fsmnt);
        exit(-1);
    }
    if (inode->blocks) {				/* XXX */
        printf("free inode %s/%u had %d blocks\n",
            fs->fs_fsmnt, ino, inode->blocks);
        inode->blocks = 0;
    }
    inode->flags = 0;
    return (0);
}
