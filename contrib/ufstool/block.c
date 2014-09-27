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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <inttypes.h>

#include "libufs.h"
#include "internal.h"

int verbose;

ssize_t
ufs_sector_read(ufs_t *disk, ufs1_daddr_t sectno, void *data, size_t size)
{
    ssize_t cnt;
    int64_t offset = (int64_t)sectno * disk->d_secsize;

    offset += disk->d_part_offset;
    cnt = pread(disk->d_fd, data, size, offset);
    if (cnt == -1) {
        printf ("%s(sectno=%u, size=%zu) read error at offset=%jd \n", __func__, sectno, size, (intmax_t)offset);
        goto fail;
    }
    if (cnt == 0) {
        printf ("%s(sectno=%u, size=%zu) end of file at offset=%jd \n", __func__, sectno, size, (intmax_t)offset);
        goto fail;
    }
    if ((size_t)cnt != size) {
        printf ("%s(sectno=%u, size=%zu) short read at offset=%jd \n", __func__, sectno, size, (intmax_t)offset);
        goto fail;
    }
//printf ("--- %s(sectno=%u, size=%zu) offset=%jd - returned %zd bytes \n", __func__, sectno, size, (intmax_t)offset, cnt);
    return (cnt);
fail:
    memset(data, 0, size);
    return (-1);
}

ssize_t
ufs_sector_write(ufs_t *disk, ufs1_daddr_t sectno, const void *data, size_t size)
{
    ssize_t cnt;
    int64_t offset = (int64_t)sectno * disk->d_secsize;
    int rv;

    rv = ufs_disk_reopen_writable(disk);
    if (rv == -1) {
        fprintf(stderr, "%s: failed to open disk for writing\n", __func__);
        return (-1);
    }

    offset += disk->d_part_offset;
    cnt = pwrite(disk->d_fd, data, size, offset);
    if (cnt == -1) {
        fprintf(stderr, "%s: write error to block device\n", __func__);
        return (-1);
    }
    if ((size_t)cnt != size) {
        fprintf(stderr, "%s: short write to block device\n", __func__);
        return (-1);
    }
//printf ("--- %s(sectno=%u, size=%zu) offset=%jd - returned %zd bytes \n", __func__, sectno, size, (intmax_t)offset, cnt);
    return (cnt);
}

int
ufs_sector_erase(ufs_t *disk, ufs1_daddr_t sectno, ufs1_daddr_t size)
{
    char *zero_chunk;
    int64_t offset, zero_chunk_size, pwrite_size;
    int rv;

    rv = ufs_disk_reopen_writable(disk);
    if (rv == -1) {
        fprintf(stderr, "%s: failed to open disk for writing\n", __func__);
        return(rv);
    }

    offset = sectno * disk->d_secsize;
    offset += disk->d_part_offset;
    zero_chunk_size = 65536 * disk->d_secsize;
    zero_chunk = calloc(1, zero_chunk_size);
    if (zero_chunk == NULL) {
        fprintf(stderr, "%s: failed to allocate memory\n", __func__);
        return (-1);
    }
    while (size > 0) {
        pwrite_size = size;
        if (pwrite_size > zero_chunk_size)
            pwrite_size = zero_chunk_size;
        rv = pwrite(disk->d_fd, zero_chunk, pwrite_size, offset);
        if (rv == -1) {
            fprintf(stderr, "%s: failed writing to disk\n", __func__);
            break;
        }
        size -= rv;
        offset += rv;
        rv = 0;
    }
    free(zero_chunk);
    return (rv);
}

/*
 * Allocate a block in a cylinder group.
 *
 * This algorithm implements the following policy:
 *   1) allocate the requested block.
 *   2) allocate a rotationally optimal block in the same cylinder.
 *   3) allocate the next available block on the block rotor for the
 *      specified cylinder group.
 * Note that this routine only allocates fs_bsize blocks; these
 * blocks may be fragmented by the routine that allocates them.
 */
static ufs1_daddr_t
cg_alloc_block(ufs_t *disk, int cg, ufs1_daddr_t bpref, int size)
{
    struct fs *fs = &disk->d_fs;
    struct cg *cgp = &disk->d_cg;
    ufs1_daddr_t bno, blkno;

    if (size != fs->fs_bsize) {
        fprintf (stderr, "%s: fragments not supported\n", __func__);
        exit(-1);
    }
    if (fs->fs_cs(fs, cg).cs_nbfree == 0)
        return 0;
    if (ufs_cgroup_read(disk, cg) < 0) {
        return 0;
    }
    if (!cg_chkmagic(cgp) || cgp->cg_cs.cs_nbfree == 0) {
        return 0;
    }
    cgp->cg_time = time(NULL);

    if (bpref == 0 || dtog(fs, bpref) != cgp->cg_cgx) {
        bpref = cgp->cg_rotor;
        goto norot;
    }
    bpref = blknum(fs, bpref);
    bpref = dtogd(fs, bpref);

    if (ffs_isblock(fs, cg_blksfree(cgp), fragstoblks(fs, bpref))) {
        /* The requested block is available, use it. */
        bno = bpref;
    } else {
        /* Take next available block in this cylinder group. */
norot:  bno = ffs_mapsearch(fs, cgp, bpref, (int)fs->fs_frag);
        if (bno < 0)
            return 0;
        cgp->cg_rotor = bno;
    }

    blkno = fragstoblks(fs, bno);
    ffs_clrblock(fs, cg_blksfree(cgp), (long)blkno);
    ffs_clusteracct(fs, cgp, blkno, -1);
    cgp->cg_cs.cs_nbfree--;
    fs->fs_cstotal.cs_nbfree--;
    fs->fs_cs(fs, cgp->cg_cgx).cs_nbfree--;
    fs->fs_fmod = 1;
    blkno = cgp->cg_cgx * fs->fs_fpg + bno;

    ufs_cgroup_write_last(disk);
    return blkno;
}

/*
 * Allocate a block near the preferenced address.
 */
int
ufs_block_alloc (ufs_t *disk, ufs1_daddr_t bpref, ufs1_daddr_t *bno)
{
    struct fs *fs = &disk->d_fs;
    int cg;

    *bno = 0;
    if (fs->fs_cstotal.cs_nbfree != 0) {
        if (bpref >= fs->fs_size)
            bpref = 0;
        cg = dtog(fs, bpref);
        *bno = ufs_cgroup_hashalloc(disk, cg, bpref, fs->fs_bsize, cg_alloc_block);
        if (*bno > 0)
            return 0;
    }
    return -ENOSPC;
}

/*
 * Free a block or fragment.
 * The specified block is placed back in the free map.
 */
void
ufs_block_free (ufs_t *disk, ufs1_daddr_t bno)
{
    struct fs *fs = &disk->d_fs;
    struct cg *cgp = &disk->d_cg;
    ufs1_daddr_t blkno;
    int cg;

    if (verbose > 1)
        printf ("free block %d, total %lld\n",
            bno, (unsigned long long)fs->fs_cstotal.cs_nbfree);
    if (bno >= fs->fs_size) {
        fprintf (stderr, "%s: bad block %d\n", __func__, bno);
        return;
    }
    cg = dtog(fs, bno);
    if (ufs_cgroup_read(disk, cg) < 0) {
        return;
    }
    if (!cg_chkmagic(cgp)) {
        return;
    }
    cgp->cg_time = time(NULL);
    bno = dtogd(fs, bno);
    blkno = fragstoblks(fs, bno);
    if (!ffs_isfreeblock(fs, cg_blksfree(cgp), blkno)) {
        fprintf (stderr, "%s: freeing free block: block = %d, fs = %s\n",
            __func__, bno, fs->fs_fsmnt);
        exit(-1);
    }
    ffs_setblock(fs, cg_blksfree(cgp), blkno);
    ffs_clusteracct(fs, cgp, blkno, 1);
    cgp->cg_cs.cs_nbfree++;
    fs->fs_cstotal.cs_nbfree++;
    fs->fs_cs(fs, cg).cs_nbfree++;
    fs->fs_fmod = 1;
    ufs_cgroup_write_last(disk);
}
