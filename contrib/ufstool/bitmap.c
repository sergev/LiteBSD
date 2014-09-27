/*-
 * Copyright (c) 1982, 1986, 1989, 1993
 *  The Regents of the University of California.  All rights reserved.
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
 *
 *  @(#)ffs_subr.c  8.5 (Berkeley) 3/21/95
 */

#include <sys/param.h>
#include <stdlib.h>

#include "libufs.h"

/*
 * Bit patterns for identifying fragments in the block map
 * used as ((map & around) == inside)
 */
static const int around[9] = {
    0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff
};
static const int inside[9] = {
    0x0, 0x2, 0x6, 0xe, 0x1e, 0x3e, 0x7e, 0xfe, 0x1fe
};

/*
 * Given a block map bit pattern, the frag tables tell whether a
 * particular size fragment is available.
 *
 * used as:
 * if ((1 << (size - 1)) & fragtbl[fs->fs_frag][map] {
 *  at least one fragment of the indicated size is available
 * }
 *
 * These tables are used by the scanc instruction on the VAX to
 * quickly find an appropriate fragment.
 */
static const u_char fragtbl124[256] = {
    0x00, 0x16, 0x16, 0x2a, 0x16, 0x16, 0x26, 0x4e,
    0x16, 0x16, 0x16, 0x3e, 0x2a, 0x3e, 0x4e, 0x8a,
    0x16, 0x16, 0x16, 0x3e, 0x16, 0x16, 0x36, 0x5e,
    0x16, 0x16, 0x16, 0x3e, 0x3e, 0x3e, 0x5e, 0x9e,
    0x16, 0x16, 0x16, 0x3e, 0x16, 0x16, 0x36, 0x5e,
    0x16, 0x16, 0x16, 0x3e, 0x3e, 0x3e, 0x5e, 0x9e,
    0x2a, 0x3e, 0x3e, 0x2a, 0x3e, 0x3e, 0x2e, 0x6e,
    0x3e, 0x3e, 0x3e, 0x3e, 0x2a, 0x3e, 0x6e, 0xaa,
    0x16, 0x16, 0x16, 0x3e, 0x16, 0x16, 0x36, 0x5e,
    0x16, 0x16, 0x16, 0x3e, 0x3e, 0x3e, 0x5e, 0x9e,
    0x16, 0x16, 0x16, 0x3e, 0x16, 0x16, 0x36, 0x5e,
    0x16, 0x16, 0x16, 0x3e, 0x3e, 0x3e, 0x5e, 0x9e,
    0x26, 0x36, 0x36, 0x2e, 0x36, 0x36, 0x26, 0x6e,
    0x36, 0x36, 0x36, 0x3e, 0x2e, 0x3e, 0x6e, 0xae,
    0x4e, 0x5e, 0x5e, 0x6e, 0x5e, 0x5e, 0x6e, 0x4e,
    0x5e, 0x5e, 0x5e, 0x7e, 0x6e, 0x7e, 0x4e, 0xce,
    0x16, 0x16, 0x16, 0x3e, 0x16, 0x16, 0x36, 0x5e,
    0x16, 0x16, 0x16, 0x3e, 0x3e, 0x3e, 0x5e, 0x9e,
    0x16, 0x16, 0x16, 0x3e, 0x16, 0x16, 0x36, 0x5e,
    0x16, 0x16, 0x16, 0x3e, 0x3e, 0x3e, 0x5e, 0x9e,
    0x16, 0x16, 0x16, 0x3e, 0x16, 0x16, 0x36, 0x5e,
    0x16, 0x16, 0x16, 0x3e, 0x3e, 0x3e, 0x5e, 0x9e,
    0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x7e,
    0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x7e, 0xbe,
    0x2a, 0x3e, 0x3e, 0x2a, 0x3e, 0x3e, 0x2e, 0x6e,
    0x3e, 0x3e, 0x3e, 0x3e, 0x2a, 0x3e, 0x6e, 0xaa,
    0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x7e,
    0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x7e, 0xbe,
    0x4e, 0x5e, 0x5e, 0x6e, 0x5e, 0x5e, 0x6e, 0x4e,
    0x5e, 0x5e, 0x5e, 0x7e, 0x6e, 0x7e, 0x4e, 0xce,
    0x8a, 0x9e, 0x9e, 0xaa, 0x9e, 0x9e, 0xae, 0xce,
    0x9e, 0x9e, 0x9e, 0xbe, 0xaa, 0xbe, 0xce, 0x8a,
};

static const u_char fragtbl8[256] = {
    0x00, 0x01, 0x01, 0x02, 0x01, 0x01, 0x02, 0x04,
    0x01, 0x01, 0x01, 0x03, 0x02, 0x03, 0x04, 0x08,
    0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x05,
    0x02, 0x03, 0x03, 0x02, 0x04, 0x05, 0x08, 0x10,
    0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x05,
    0x01, 0x01, 0x01, 0x03, 0x03, 0x03, 0x05, 0x09,
    0x02, 0x03, 0x03, 0x02, 0x03, 0x03, 0x02, 0x06,
    0x04, 0x05, 0x05, 0x06, 0x08, 0x09, 0x10, 0x20,
    0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x05,
    0x01, 0x01, 0x01, 0x03, 0x03, 0x03, 0x05, 0x09,
    0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x05,
    0x03, 0x03, 0x03, 0x03, 0x05, 0x05, 0x09, 0x11,
    0x02, 0x03, 0x03, 0x02, 0x03, 0x03, 0x02, 0x06,
    0x03, 0x03, 0x03, 0x03, 0x02, 0x03, 0x06, 0x0a,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x05, 0x06, 0x04,
    0x08, 0x09, 0x09, 0x0a, 0x10, 0x11, 0x20, 0x40,
    0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x05,
    0x01, 0x01, 0x01, 0x03, 0x03, 0x03, 0x05, 0x09,
    0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x05,
    0x03, 0x03, 0x03, 0x03, 0x05, 0x05, 0x09, 0x11,
    0x01, 0x01, 0x01, 0x03, 0x01, 0x01, 0x03, 0x05,
    0x01, 0x01, 0x01, 0x03, 0x03, 0x03, 0x05, 0x09,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07,
    0x05, 0x05, 0x05, 0x07, 0x09, 0x09, 0x11, 0x21,
    0x02, 0x03, 0x03, 0x02, 0x03, 0x03, 0x02, 0x06,
    0x03, 0x03, 0x03, 0x03, 0x02, 0x03, 0x06, 0x0a,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07,
    0x02, 0x03, 0x03, 0x02, 0x06, 0x07, 0x0a, 0x12,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x05, 0x06, 0x04,
    0x05, 0x05, 0x05, 0x07, 0x06, 0x07, 0x04, 0x0c,
    0x08, 0x09, 0x09, 0x0a, 0x09, 0x09, 0x0a, 0x0c,
    0x10, 0x11, 0x11, 0x12, 0x20, 0x21, 0x40, 0x80,
};

/*
 * The actual fragtbl array.
 */
static const u_char *fragtbl[MAXFRAG + 1] = {
    0, fragtbl124, fragtbl124, 0, fragtbl124, 0, 0, 0, fragtbl8,
};

/*
 * Update the frsum fields to reflect addition or deletion
 * of some frags.
 */
void
ffs_fragacct(fs, fragmap, fraglist, cnt)
    struct fs *fs;
    int fragmap;
    int32_t fraglist[];
    int cnt;
{
    int inblk;
    int field, subfield;
    int siz, pos;

    inblk = (int)fragtbl[fs->fs_frag][fragmap] << 1;
    fragmap <<= 1;
    for (siz = 1; siz < fs->fs_frag; siz++) {
        if ((inblk & (1 << (siz + (fs->fs_frag % NBBY)))) == 0)
            continue;
        field = around[siz];
        subfield = inside[siz];
        for (pos = siz; pos <= fs->fs_frag; pos++) {
            if ((fragmap & field) == subfield) {
                fraglist[siz] += cnt;
                pos += siz;
                field <<= siz;
                subfield <<= siz;
            }
            field <<= 1;
            subfield <<= 1;
        }
    }
}

/*
 * block operations
 *
 * check if a block is available
 */
int
ffs_isblock(fs, cp, h)
    struct fs *fs;
    unsigned char *cp;
    ufs1_daddr_t h;
{
    unsigned char mask;

    switch ((int)fs->fs_frag) {
    case 8:
        return (cp[h] == 0xff);
    case 4:
        mask = 0x0f << ((h & 0x1) << 2);
        return ((cp[h >> 1] & mask) == mask);
    case 2:
        mask = 0x03 << ((h & 0x3) << 1);
        return ((cp[h >> 2] & mask) == mask);
    case 1:
        mask = 0x01 << (h & 0x7);
        return ((cp[h >> 3] & mask) == mask);
    default:
        break;
    }
    return (0);
}

/*
 * check if a block is free
 */
int
ffs_isfreeblock(fs, cp, h)
    struct fs *fs;
    u_char *cp;
    ufs1_daddr_t h;
{

    switch ((int)fs->fs_frag) {
    case 8:
        return (cp[h] == 0);
    case 4:
        return ((cp[h >> 1] & (0x0f << ((h & 0x1) << 2))) == 0);
    case 2:
        return ((cp[h >> 2] & (0x03 << ((h & 0x3) << 1))) == 0);
    case 1:
        return ((cp[h >> 3] & (0x01 << (h & 0x7))) == 0);
    default:
        break;
    }
    return (0);
}

/*
 * take a block out of the map
 */
void
ffs_clrblock(fs, cp, h)
    struct fs *fs;
    u_char *cp;
    ufs1_daddr_t h;
{

    switch ((int)fs->fs_frag) {
    case 8:
        cp[h] = 0;
        return;
    case 4:
        cp[h >> 1] &= ~(0x0f << ((h & 0x1) << 2));
        return;
    case 2:
        cp[h >> 2] &= ~(0x03 << ((h & 0x3) << 1));
        return;
    case 1:
        cp[h >> 3] &= ~(0x01 << (h & 0x7));
        return;
    default:
        break;
    }
}

/*
 * put a block into the map
 */
void
ffs_setblock(fs, cp, h)
    struct fs *fs;
    unsigned char *cp;
    ufs1_daddr_t h;
{

    switch ((int)fs->fs_frag) {

    case 8:
        cp[h] = 0xff;
        return;
    case 4:
        cp[h >> 1] |= (0x0f << ((h & 0x1) << 2));
        return;
    case 2:
        cp[h >> 2] |= (0x03 << ((h & 0x3) << 1));
        return;
    case 1:
        cp[h >> 3] |= (0x01 << (h & 0x7));
        return;
    default:
        break;
    }
}

/*
 * Update the cluster map because of an allocation or free.
 *
 * Cnt == 1 means free; cnt == -1 means allocating.
 */
void
ffs_clusteracct(fs, cgp, blkno, cnt)
    struct fs *fs;
    struct cg *cgp;
    ufs1_daddr_t blkno;
    int cnt;
{
    int32_t *sump;
    int32_t *lp;
    u_char *freemapp, *mapp;
    int i, start, end, forw, back, map, bit;

    if (fs->fs_contigsumsize <= 0)
        return;
    freemapp = cg_clustersfree(cgp);
    sump = cg_clustersum(cgp);
    /*
     * Allocate or clear the actual block.
     */
    if (cnt > 0)
        setbit(freemapp, blkno);
    else
        clrbit(freemapp, blkno);
    /*
     * Find the size of the cluster going forward.
     */
    start = blkno + 1;
    end = start + fs->fs_contigsumsize;
    if (end >= cgp->cg_nclusterblks)
        end = cgp->cg_nclusterblks;
    mapp = &freemapp[start / NBBY];
    map = *mapp++;
    bit = 1 << (start % NBBY);
    for (i = start; i < end; i++) {
        if ((map & bit) == 0)
            break;
        if ((i & (NBBY - 1)) != (NBBY - 1)) {
            bit <<= 1;
        } else {
            map = *mapp++;
            bit = 1;
        }
    }
    forw = i - start;
    /*
     * Find the size of the cluster going backward.
     */
    start = blkno - 1;
    end = start - fs->fs_contigsumsize;
    if (end < 0)
        end = -1;
    mapp = &freemapp[start / NBBY];
    map = *mapp--;
    bit = 1 << (start % NBBY);
    for (i = start; i > end; i--) {
        if ((map & bit) == 0)
            break;
        if ((i & (NBBY - 1)) != 0) {
            bit >>= 1;
        } else {
            map = *mapp--;
            bit = 1 << (NBBY - 1);
        }
    }
    back = start - i;
    /*
     * Account for old cluster and the possibly new forward and
     * back clusters.
     */
    i = back + forw + 1;
    if (i > fs->fs_contigsumsize)
        i = fs->fs_contigsumsize;
    sump[i] += cnt;
    if (back > 0)
        sump[back] -= cnt;
    if (forw > 0)
        sump[forw] -= cnt;
    /*
     * Update cluster summary information.
     */
    lp = &sump[fs->fs_contigsumsize];
    for (i = fs->fs_contigsumsize; i > 0; i--)
        if (*lp-- > 0)
            break;
    fs->fs_maxcluster[cgp->cg_cgx] = i;
}

static int
scanc(u_int size, u_char *cp, const u_char table[], int mask)
{
    u_char *end = &cp[size];

    while (cp < end && (table[*cp] & mask) == 0)
        ++cp;
    return end - cp;
}

/*
 * Find a block of the specified size in the specified cylinder group.
 *
 * It is a panic if a request is made to find a block if none are
 * available.
 */
ufs1_daddr_t
ffs_mapsearch(struct fs *fs, struct cg *cgp, ufs1_daddr_t bpref, int allocsiz)
{
    ufs1_daddr_t bno;
    int start, len, loc, i;
    int blk, field, subfield, pos;

    /*
     * find the fragment by searching through the free block
     * map for an appropriate bit pattern
     */
    if (bpref)
        start = dtogd(fs, bpref) / NBBY;
    else
        start = cgp->cg_frotor / NBBY;
    len = howmany(fs->fs_fpg, NBBY) - start;
    loc = scanc((u_int)len, (u_char *)&cg_blksfree(cgp)[start],
        fragtbl[fs->fs_frag],
        (u_char)(1 << (allocsiz - 1 + (fs->fs_frag % NBBY))));
    if (loc == 0) {
        len = start + 1;
        start = 0;
        loc = scanc((u_int)len, (u_char *)&cg_blksfree(cgp)[0],
            fragtbl[fs->fs_frag],
            (u_char)(1 << (allocsiz - 1 + (fs->fs_frag % NBBY))));
        if (loc == 0) {
            fprintf(stderr, "%s: map corrupted: start = %d, len = %d, fs = %s\n",
                __func__, start, len, fs->fs_fsmnt);
            exit(-1);
        }
    }
    bno = (start + len - loc) * NBBY;
    cgp->cg_frotor = bno;
    /*
     * found the byte in the map
     * sift through the bits to find the selected frag
     */
    for (i = bno + NBBY; bno < i; bno += fs->fs_frag) {
        blk = blkmap(fs, cg_blksfree(cgp), bno);
        blk <<= 1;
        field = around[allocsiz];
        subfield = inside[allocsiz];
        for (pos = 0; pos <= fs->fs_frag - allocsiz; pos++) {
            if ((blk & field) == subfield)
                return (bno + pos);
            field <<= 1;
            subfield <<= 1;
        }
    }
    fprintf(stderr, "%s: block not in map: bno = %d, fs = %s\n",
        __func__, bno, fs->fs_fsmnt);
    exit(-1);
    return (-1);
}
