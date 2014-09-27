/*
 * Copyright (c) 2003 Juli Mallett.  All rights reserved.
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
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "libufs.h"
#include "internal.h"

int
ufs_cgroup_read_next(ufs_t *disk)
{
    return ufs_cgroup_read(disk, disk->d_ccg++);
}

int
ufs_cgroup_read(ufs_t *disk, int c)
{
    struct fs *fs = &disk->d_fs;

    if ((unsigned)c >= fs->fs_ncg) {
        return (0);
    }
    if (ufs_sector_read(disk, fsbtodb(fs, cgtod(fs, c)), disk->d_cgunion.d_buf,
        fs->fs_bsize) < 0) {
        fprintf (stderr, "%s: unable to read cylinder group %u\n", __func__, c);
        return (-1);
    }
    disk->d_lcg = c;
    return (1);
}

int
ufs_cgroup_write_last(ufs_t *disk)
{
    return (ufs_cgroup_write(disk, disk->d_lcg));
}

int
ufs_cgroup_write(ufs_t *disk, int c)
{
    struct fs *fs;

    fs = &disk->d_fs;
    if (ufs_sector_write(disk, fsbtodb(fs, cgtod(fs, c)),
        disk->d_cgunion.d_buf, fs->fs_bsize) == -1) {
        fprintf(stderr, "%s: unable to write cylinder group\n", __func__);
        return (-1);
    }
    return (0);
}

/*
 * Dump a cylinder group.
 */
void
ufs_print_cg(struct cg *cgr, FILE *out)
{
    int j;

    fprintf(out, "                     Magic number: %#x\n", cgr->cg_magic);
    fprintf(out, "                Last time written: %s", ctime((const time_t*)&cgr->cg_old_time));
    fprintf(out, "             Cylinder group index: %d\n", cgr->cg_cgx);
    fprintf(out, "              Number of cylinders: %d\n", cgr->cg_old_ncyl);
    fprintf(out, "          Number of inode sectors: %d\n", cgr->cg_old_niblk);
    fprintf(out, "           Number of data sectors: %d\n", cgr->cg_ndblk);
    fprintf(out, "                 Number of blocks: %d\n", cgr->cg_nclusterblks);

    /* Cylinder summary information */
    fprintf(out, "            Number of directories: %d\n", cgr->cg_cs.cs_ndir);
    fprintf(out, "            Number of free blocks: %d\n", cgr->cg_cs.cs_nbfree);
    fprintf(out, "            Number of free inodes: %d\n", cgr->cg_cs.cs_nifree);
    fprintf(out, "             Number of free frags: %d\n", cgr->cg_cs.cs_nffree);

    fprintf(out, "Rotational pos of last used block: %d\n", cgr->cg_rotor);
    fprintf(out, " Rotational pos of last used frag: %d\n", cgr->cg_frotor);
    fprintf(out, "Rotational pos of last used inode: %d\n", cgr->cg_irotor);

    /* Counts of available frags */
    fprintf(out, "        Counts of available frags:");
    for (j = 0; j < MAXFRAG; j++) {
        if (j)
            fprintf(out, ",");
        fprintf(out, " %d", cgr->cg_frsum[j]);
    }
    fprintf(out, "\n");
    fprintf(out, "   Offset of block totals per cyl: %d bytes\n", cgr->cg_old_btotoff);
    fprintf(out, "   Offset of free block positions: %d bytes\n", cgr->cg_old_boff);
    fprintf(out, "         Offset of used inode map: %d bytes\n", cgr->cg_iusedoff);
    fprintf(out, "         Offset of free block map: %d bytes\n", cgr->cg_freeoff);
    fprintf(out, "   Offset of next available space: %d bytes\n", cgr->cg_nextfreeoff);
    fprintf(out, " Offset of counts of avail blocks: %d bytes\n", cgr->cg_clustersumoff);
    fprintf(out, "         Offset of free block map: %d bytes\n", cgr->cg_clusteroff);
}

/*
 * Implement the cylinder overflow algorithm.
 *
 * The policy implemented by this algorithm is:
 *   1) allocate the block in its requested cylinder group.
 *   2) quadradically rehash on the cylinder group number.
 *   3) brute force search for a free block.
 */
ufs1_daddr_t
ufs_cgroup_hashalloc(ufs_t *disk, int cg, ufs1_daddr_t pref, int param,
    ufs1_daddr_t (*allocator)())
{
    struct fs *fs = &disk->d_fs;
    ufs1_daddr_t result;
    int i, icg = cg;

    /*
     * 1: preferred cylinder group
     */
    result = (*allocator)(disk, cg, pref, param);
    if (result)
        return (result);
    /*
     * 2: quadratic rehash
     */
    for (i = 1; i < fs->fs_ncg; i *= 2) {
        cg += i;
        if (cg >= fs->fs_ncg)
            cg -= fs->fs_ncg;
        result = (*allocator)(disk, cg, 0, param);
        if (result)
            return (result);
    }
    /*
     * 3: brute force search
     * Note that we start at i == 2, since 0 was checked initially,
     * and 1 is always checked in the quadratic rehash.
     */
    cg = (icg + 2) % fs->fs_ncg;
    for (i = 2; i < fs->fs_ncg; i++) {
        result = (*allocator)(disk, cg, 0, param);
        if (result)
            return (result);
        cg++;
        if (cg == fs->fs_ncg)
            cg = 0;
    }
    return 0;
}
