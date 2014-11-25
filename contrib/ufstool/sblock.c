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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include "libufs.h"
#include "internal.h"

static const int superblocks[] = SBLOCKSEARCH;

int
ufs_superblock_read(ufs_t *disk)
{
    u_int8_t block[MAXBSIZE];
    struct fs *fs;
    int sb, superblock;
    int i, size, blks;
    u_int8_t *space;

    fs = &disk->d_fs;
    superblock = superblocks[0];

    for (sb = 0; (superblock = superblocks[sb]) != -1; sb++) {
//printf ("--- %s() superblock offset = %d\n", __func__, superblock);
        if (ufs_sector_read(disk, superblock / disk->d_secsize, disk->d_sb, SBLOCKSIZE) == -1) {
            fprintf (stderr, "%s: non-existent or truncated superblock at offset %d\n", __func__, superblock);
            return (-1);
        }
        if (fs->fs_magic == FS_UFS1_MAGIC)
            disk->d_ufs = 1;
        if (fs->fs_magic == FS_UFS2_MAGIC &&
            fs->fs_sblockloc == superblock)
            disk->d_ufs = 2;
        if (fs->fs_bsize <= MAXBSIZE &&
            (size_t)fs->fs_bsize >= sizeof(*fs)) {
            if (disk->d_ufs)
                break;
        }
        disk->d_ufs = 0;
    }
    if (superblock == -1 || disk->d_ufs == 0) {
        /*
         * Other error cases will result in errno being set, here we
         * must set it to indicate no superblock could be found with
         * which to associate this disk/filesystem.
         */
        fprintf (stderr, "%s: no usable known superblock found\n", __func__);
        errno = ENOENT;
        return (-1);
    }
    disk->d_secsize = fs->fs_fsize / fsbtodb(fs, 1);
    disk->d_sblock = superblock / disk->d_secsize;
    /*
     * Read in the superblock summary information.
     */
    size = fs->fs_cssize;
    blks = howmany(size, fs->fs_fsize);
    size += fs->fs_ncg * sizeof(int32_t);
    space = malloc(size);
    if (space == NULL) {
        fprintf (stderr, "%s: failed to allocate space for summary information\n", __func__);
        return (-1);
    }
    fs->fs_csp = (struct csum *)space;
    for (i = 0; i < blks; i += fs->fs_frag) {
        size = fs->fs_bsize;
        if (i + fs->fs_frag > blks)
            size = (blks - i) * fs->fs_fsize;
        if (ufs_sector_read(disk, fsbtodb(fs, fs->fs_csaddr + i), block, size)
            == -1) {
            fprintf (stderr, "%s: failed to read sb summary information\n", __func__);
            free(fs->fs_csp);
            return (-1);
        }
        bcopy(block, space, size);
        space += size;
    }
    fs->fs_maxcluster = (int32_t *)space;
    disk->d_sbcsum = fs->fs_csp;

    return (0);
}

int
ufs_superblock_write(ufs_t *disk, int all)
{
    struct fs *fs = &disk->d_fs;
    int blks, size;
    u_int8_t *space;
    unsigned i;

    if (!disk->d_sblock) {
        disk->d_sblock = fs->fs_sblockloc / disk->d_secsize;
    }

    if (fs->fs_magic == FS_UFS1_MAGIC) {
        fs->fs_old_time = fs->fs_time;
        fs->fs_old_size = fs->fs_size;
        fs->fs_old_dsize = fs->fs_dsize;
        fs->fs_old_csaddr = fs->fs_csaddr;
        fs->fs_old_cstotal.cs_ndir = fs->fs_cstotal.cs_ndir;
        fs->fs_old_cstotal.cs_nbfree = fs->fs_cstotal.cs_nbfree;
        fs->fs_old_cstotal.cs_nifree = fs->fs_cstotal.cs_nifree;
        fs->fs_old_cstotal.cs_nffree = fs->fs_cstotal.cs_nffree;
    }

    if (ufs_sector_write(disk, disk->d_sblock, fs, SBLOCKSIZE) == -1) {
        fprintf (stderr, "%s: failed to write superblock\n", __func__);
        return (-1);
    }

    /*
     * Write superblock summary information.
     */
    blks = howmany(fs->fs_cssize, fs->fs_fsize);
    space = (u_int8_t *)disk->d_sbcsum;
    for (i = 0; i < blks; i += fs->fs_frag) {
        size = fs->fs_bsize;
        if (i + fs->fs_frag > blks)
            size = (blks - i) * fs->fs_fsize;
        if (ufs_sector_write(disk, fsbtodb(fs, fs->fs_csaddr + i), space, size)
            == -1) {
            fprintf (stderr, "%s: failed to write sb summary information\n", __func__);
            return (-1);
        }
        space += size;
    }
    if (all) {
        for (i = 0; i < fs->fs_ncg; i++) {
            if (ufs_sector_write(disk, fsbtodb(fs, cgsblock(fs, i)),
                fs, SBLOCKSIZE) == -1) {
                fprintf (stderr, "%s: failed to update a superblock\n", __func__);
                return (-1);
            }
        }
    }
    return (0);
}

/*
 * Dump the superblock.
 */
void ufs_print(ufs_t *disk, FILE *out)
{
    struct fs *sb = &disk->d_fs;
    struct cg *cg = &disk->d_cg;
    int cylno;
    time_t t;

    fprintf(out, "           Address of super-block: %d\n", sb->fs_sblkno);
    fprintf(out, "              Offset of cyl-block: %d\n", sb->fs_cblkno);
    fprintf(out, "           Offset of inode-blocks: %d\n", sb->fs_iblkno);
    fprintf(out, "    Offset of first data after cg: %d\n", sb->fs_dblkno);

    fprintf(out, "Cylinder group offset in cylinder: %d\n", sb->fs_old_cgoffset);
    fprintf(out, "              Cylinder group mask: %#x\n", sb->fs_old_cgmask);
    t = sb->fs_old_time;
    fprintf(out, "                Last time written: %s", ctime(&t));
    fprintf(out, "           Total number of blocks: %d\n", sb->fs_old_size);
    fprintf(out, "            Number of data blocks: %d\n", sb->fs_old_dsize);
    fprintf(out, "        Number of cylinder groups: %d\n", sb->fs_ncg);
    fprintf(out, "             Size of basic blocks: %d bytes\n", sb->fs_bsize);
    fprintf(out, "              Size of frag blocks: %d bytes\n", sb->fs_fsize);
    fprintf(out, "       Number of frags in a block: %d\n", sb->fs_frag);

    fprintf(out, "         Minimum %% of free blocks: %d%%\n", sb->fs_minfree);
    fprintf(out, "     Optimal delay for next block: %d msec\n", sb->fs_old_rotdelay);
    fprintf(out, "            Disk revolution speed: %d rotations per second\n", sb->fs_old_rps);

    fprintf(out, "           Mask for block offsets: %#x\n", sb->fs_bmask);     /* ``blkoff'' calc */
    fprintf(out, "            Mask for frag offsets: %#x\n", sb->fs_fmask);     /* ``fragoff'' calc */
    fprintf(out, "   Shift for logical block number: %d\n", sb->fs_bshift);     /* ``lblkno'' calc */
    fprintf(out, "        Shift for number of frags: %d\n", sb->fs_fshift);     /* ``numfrags'' calc  */

    fprintf(out, "  Max number of contiguous blocks: %d\n", sb->fs_maxcontig);
    fprintf(out, "    Max blocks per cylinder group: %d\n", sb->fs_maxbpg);

    fprintf(out, "              Block to frag shift: %d\n", sb->fs_fragshift);
    fprintf(out, "Filesys frag to disk sector shift: %d\n", sb->fs_fsbtodb);
    fprintf(out, "       Actual size of super block: %d bytes\n", sb->fs_sbsize);
    fprintf(out, "    Number of indirects per block: %d\n", sb->fs_nindir);
    fprintf(out, "       Number of inodes per block: %d\n", sb->fs_inopb);
    fprintf(out, "   Number of sectors per fragment: %d\n", sb->fs_old_nspf);

    fprintf(out, "          Optimization preference: %d (%s)\n", sb->fs_optim,
    sb->fs_optim == FS_OPTTIME ? "time" : "space");

    fprintf(out, "      Number of sectors per track: %d\n", sb->fs_old_npsect); /* including spares */
    fprintf(out, "       Hardware sector interleave: %d\n", sb->fs_old_interleave);
    fprintf(out, "        Sector #0 skew, per track: %d\n", sb->fs_old_trackskew);

    fprintf(out, "  Block addr of cyl group summary: %d\n", sb->fs_old_csaddr);
    fprintf(out, "   Size of cyl group summary area: %d bytes\n", sb->fs_cssize);
    fprintf(out, "              Cylinder group size: %d\n", sb->fs_cgsize);

    fprintf(out, "                Sectors per track: %d\n", sb->fs_old_nsect);
    fprintf(out, "             Sectors per cylinder: %d\n", sb->fs_old_spc);
    fprintf(out, "        Total number of cylinders: %d\n", sb->fs_old_ncyl);
    fprintf(out, "              Cylinders per group: %d\n", sb->fs_old_cpg);
    fprintf(out, "                 Inodes per group: %d\n", sb->fs_ipg);
    fprintf(out, "                 Blocks per group: %d frags\n", sb->fs_fpg);

    fprintf(out, "      Total number of directories: %d\n", sb->fs_old_cstotal.cs_ndir);
    fprintf(out, "      Total number of free blocks: %d\n", sb->fs_old_cstotal.cs_nbfree);
    fprintf(out, "      Total number of free inodes: %d\n", sb->fs_old_cstotal.cs_nifree);
    fprintf(out, "       Total number of free frags: %d\n", sb->fs_old_cstotal.cs_nffree);

    fprintf(out, "                  Name mounted on: '%s'\n", sb->fs_fsmnt);

    fprintf(out, "    Seek speed, cyls per rotation: %d\n", sb->fs_old_cpc);

    fprintf(out, "      Size of block summary array: %d\n", sb->fs_contigsumsize);
    fprintf(out, "      Max length of short symlink: %d\n", sb->fs_maxsymlinklen);
    fprintf(out, "         Format of on-disk inodes: %d (%s)\n", sb->fs_old_inodefmt,
    sb->fs_old_inodefmt == FS_42INODEFMT ? "4.2bsd" :
    sb->fs_old_inodefmt == FS_44INODEFMT ? "4.4bsd" : "unknown");

    fprintf(out, "                Maximum file size: %ju bytes\n", (uintmax_t) sb->fs_maxfilesize);
    fprintf(out, "                Block offset mask: %#018jx\n", (uintmax_t) sb->fs_qbmask);
    fprintf(out, "                 Frag offset mask: %#018jx\n", (uintmax_t) sb->fs_qfmask);

    fprintf(out, "   Number of rotational positions: %d\n", sb->fs_old_nrpos);
    fprintf(out, "                     Magic number: %#x\n", sb->fs_magic);

    /* Read and print all the cylinder groups. */
    for (cylno = 0; cylno < sb->fs_ncg; cylno++) {
        if (ufs_sector_read(disk, fsbtodb(sb, cgtod(sb, cylno)), (void*)cg, (size_t)sb->fs_cgsize) == -1) {
            fprintf (stderr, "Cannot read cylinder group %d\n", cylno);
            exit(-1);
        }
        fprintf(out, "-------- Cylinder group #%d --------\n", cylno);
        ufs_print_cg(cg, out);
    }
}
