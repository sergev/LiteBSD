/*
 * SD flash card disk driver.
 */
#include "sd.h"
#if NSD > 0

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/stat.h>
#include <sys/syslog.h>

#include <mips/dev/device.h>

#define sdunit(dev) ((minor(dev) & 8) >> 3)
#define sdpart(dev) ((minor(dev) & 7))
#define RAWPART     0                   /* 'x' partition */

#define NPARTITIONS 4

struct partition {          /* the partition table */
    u_int p_size;           /* number of sectors in partition */
    u_int p_offset;         /* starting sector */
    u_int p_fsize;          /* filesystem basic fragment size */
    u_int p_fstype;         /* filesystem type, see below */
    u_int p_frag;           /* filesystem fragments per block */
};

/*
 * The structure of a disk drive.
 */
struct disk {
    /* the partition table */
    struct partition dk_partitions [NPARTITIONS];

    int     dk_bc;          /* byte count left */
    int     dk_skip;        /* blocks already transferred */
    int     dk_unit;        /* physical unit number */
    int     dk_state;       /* control state */
#define CLOSED      0       /* disk is closed. */
#define WANTOPEN    1       /* open requested, not started */
#define RECAL       2       /* doing restore */
#define RDLABEL     3       /* reading pack label */
#define OPEN        4       /* done with open */

    u_int   dk_status;      /* copy of status reg. */
    u_int   dk_error;       /* copy of error reg. */
    int     dk_open;        /* open/closed refcnt */
    u_int   dk_copenpart;   /* character units open on this drive */
    u_int   dk_bopenpart;   /* block units open on this drive */
    u_int   dk_openpart;    /* all units open on this drive */
    int     dk_wlabel;      /* label writable? */
};

struct disk sddrives[NSD];              /* table of units */
struct buf sdtab;                       /* head of queue per controller */
struct buf sdutab[NSD];                 /* head of queue per drive */

/*
 * Routine to queue a read or write command to the controller.  The request is
 * linked into the active list for the controller.  If the controller is idle,
 * the transfer is started.
 */
void
sdustart(du)
    struct disk *du;
{
    struct buf *bp, *dp;

    dp = &sdutab[du->dk_unit];
    if (dp->b_active)
        return;

    bp = dp->b_actf;
    if (bp) {
        /* Append the block to sdtab queue. */
        bp->b_actb = sdtab.b_actb;
        *sdtab.b_actb = bp;
        sdtab.b_actb = &bp->b_actf;

        dp->b_active = 1;               /* mark the drive as busy */
    }
}

/*
 * Controller startup routine.  This does the calculation, and starts
 * a single-sector read or write operation.  Called to start a transfer,
 * or from the interrupt routine to continue a multi-sector transfer.
 * RESTRICTIONS:
 * 1.   The transfer length must be an exact multiple of the sector size.
 */
void
sdstart()
{
    struct disk *du;   /* disk unit for IO */
    struct buf *bp;
    struct buf *dp;
    long blknum,  sector;
    long addr, i;
    int unit, s;

loop:
    dp = sdtab.b_actf;
    if (! dp)
        return;

    bp = dp->b_actf;
    if (! bp) {
        sdtab.b_actf = dp->b_forw;
        goto loop;
    }
    unit = sdunit(bp->b_dev);
    du = &sddrives[unit];
    if (du->dk_state <= RDLABEL) {
        if (sdcontrol(bp)) {
            dp->b_actf = bp->av_forw;
            goto loop;  /* done */
        }
        return;
    }

    /*
     * Convert DEV_BSIZE "blocks" to sectors.
     */
    blknum = (unsigned long) bp->b_blkno * DEV_BSIZE / 512 + du->dk_skip;
#ifdef SDDEBUG
    if (du->dk_skip == 0) {
        dprintf(DDSK,"\nsdstart %d: %s %d@%d; map ", unit,
            (bp->b_flags & B_READ) ? "read" : "write",
            bp->b_bcount, blknum);
    } else {
        dprintf(DDSK," %d)%x", du->dk_skip, inb(wdc+wd_altsts));
    }
#endif

    addr = (int) bp->b_un.b_addr;
    if (du->dk_skip == 0)
        du->dk_bc = bp->b_bcount;
    if (du->dk_state == OPEN)
        blknum += du->dk_partitions[sdpart(bp->b_dev)].p_offset;

    sdtab.b_active = 1;     /* mark controller active */

    if(du->dk_skip==0) {
        if(sdtab.b_errcnt && (bp->b_flags & B_READ) == 0) du->dk_bc += 512;
        //while ((inb(wdc+wd_status) & WDCS_BUSY) != 0)
        //    ;
        //outb(wdc+wd_precomp, 0xff);
        //outb(wdc+wd_seccnt, ((du->dk_bc +511) / 512));
        //outb(wdc+wd_sector, blknum);

        //outb(wdc+wd_cyl_lo, cylin);
        //outb(wdc+wd_cyl_hi, cylin >> 8);

        /* Set up the SDH register (select drive).     */
        //outb(wdc+wd_sdh, WDSD_IBM | (unit<<4) | (head & 0xf));
        //while ((inb(wdc+wd_status) & WDCS_READY) == 0)

        //outb(wdc+wd_command, (bp->b_flags & B_READ)? WDCC_READ : WDCC_WRITE);
    }

    /* If this is a read operation, just go away until it's done.   */
    if (bp->b_flags & B_READ)
        return;

    /* Ready to send data?  */
    //while ((inb(wdc+wd_status) & WDCS_DRQ) == 0)
    //    ;

    /* ASSUMES CONTIGUOUS MEMORY */
    //outsw (wdc+wd_data, addr+du->dk_skip*512, 256);
    du->dk_bc -= 512;
}

/*
 * Read/write routine for a buffer.  Finds the proper unit, range checks
 * arguments, and schedules the transfer.  Does not wait for the transfer
 * to complete.  Multi-page transfers are supported.  All I/O requests must
 * be a multiple of a sector in length.
 */
void
sdstrategy(bp)
    struct buf *bp;
{
    struct disk *du;    /* Disk unit to do the IO.  */
    struct buf *dp;
    int unit = sdunit(bp->b_dev);
    int s;

    if (unit >= NSD || bp->b_blkno < 0) {
        printf("sdstrategy: unit = %d, blkno = %d, bcount = %d\n",
            unit, bp->b_blkno, bp->b_bcount);
        pg("sd: error in sdstrategy");
        bp->b_flags |= B_ERROR;
        goto bad;
    }
    du = &sddrives[unit];
    if (du->dk_state == OPEN) {
        /*
         * Determine the size of the transfer, and make sure it is
         * within the boundaries of the partition.
         */
        struct partition *p = &du->dk_partitions[sdpart(bp->b_dev)];
        long maxsz = p->p_size;
        long sz = (bp->b_bcount + DEV_BSIZE - 1) >> DEV_BSHIFT;

        if (bp->b_blkno + p->p_offset <= 0 &&
            ! (bp->b_flags & B_READ) && ! du->dk_wlabel) {
                bp->b_error = EROFS;
                goto bad;
        }
        if (bp->b_blkno < 0 || bp->b_blkno + sz > maxsz) {
                /* if exactly at end of disk, return an EOF */
                if (bp->b_blkno == maxsz) {
                        bp->b_resid = bp->b_bcount;
                        biodone(bp);
                        return;
                }
                /* or truncate if part of it fits */
                sz = maxsz - bp->b_blkno;
                if (sz <= 0)
                        goto bad;
                bp->b_bcount = sz << DEV_BSHIFT;
        }
    } else {
        /* Reading the partition table. */
    }

    dp = &sdutab[unit];
    s = splhigh();
    bp->b_actb = dp->b_actb;    /* append the block to unit queue */
    *dp->b_actb = bp;
    dp->b_actb = &bp->b_actf;
    if (! dp->b_active)
        sdustart(du);           /* start drive if idle */
    if (! sdtab.b_active)
        sdstart(s);             /* start IO if controller idle */
    splx(s);
    return;

bad:
    bp->b_error = EINVAL;
    biodone(bp);
}

/*
 * Initialize a drive.
 */
int
sdopen(dev, flags, mode, p)
    dev_t dev;
    int flags, mode;
    struct proc *p;
{
    struct disk *du;
    struct buf *bp;
    struct partition *pp;
    unsigned int unit = sdunit(dev);
    int part = sdpart(dev), mask, i;

    if (unit >= NSD || part > NPARTITIONS)
        return ENXIO;
    du = &sddrives[unit];
    du->dk_unit = unit;
    sdutab[unit].b_actf = 0;
    du->dk_state = WANTOPEN;

    /*
     * Recal, read of disk label will be done in sdcontrol
     * during first read operation.
     */
    bp = geteblk(512);
    bp->b_dev = dev & 0xff00;
    bp->b_bcount = 0;
    bp->b_blkno = 0;            // Read sector #0
    bp->b_flags = B_READ;
    sdstrategy(bp);
    biowait(bp);
    if (bp->b_flags & B_ERROR) {
        du->dk_state = CLOSED;
    } else {
        du->dk_state = OPEN;
        // TODO: copy partition table to dk_partitions[]
    }

    bp->b_flags = B_INVAL | B_AGE;
    brelse(bp);
    if (du->dk_state == CLOSED)
        return ENXIO;

    du->dk_open = 1;

    /*
     * Warn if a partion is opened
     * that overlaps another partition which is open
     * unless one is the "raw" partition (whole disk).
     */
    mask = 1 << part;
    if (! (du->dk_openpart & mask) && part != RAWPART) {
        int start, end;

        pp = &du->dk_partitions[part];
        start = pp->p_offset;
        end = pp->p_offset + pp->p_size;
        for (pp = du->dk_partitions;
             pp < &du->dk_partitions[NPARTITIONS]; pp++)
        {
            if (pp->p_offset + pp->p_size <= start ||
                pp->p_offset >= end)
                continue;

            if (pp - du->dk_partitions == RAWPART)
                continue;

            if (du->dk_openpart & (1 << (pp - du->dk_partitions)))
                log(LOG_WARNING, "sd%d%c: overlaps open partition (%c)\n",
                    unit, part + 'a' - 1, pp - du->dk_partitions + 'a' - 1);
        }
    }

    du->dk_openpart |= mask;
    switch (mode) {
    case S_IFCHR:
        du->dk_copenpart |= mask;
        break;
    case S_IFBLK:
        du->dk_bopenpart |= mask;
        break;
    }
    return 0;
}

int
sdioctl(dev, cmd, data, flag, p)
    dev_t dev;
    u_long cmd;
    caddr_t data;
    int flag;
    struct proc *p;
{
    // TODO
    return EINVAL;
}

int
sdsize(dev)
    dev_t dev;
{
    // TODO
    return -1;
}

/*
* Non-interrupt driven, non-dma dump routine.
*/
int
sddump(dev)
    dev_t dev;
{
    // TODO
    return ENXIO;
}

int
sdread(dev, uio)
    dev_t dev;
    struct uio *uio;
{
    // TODO
    return EPERM;
}

int
sdwrite(dev, uio)
    dev_t dev;
    struct uio *uio;
{
    // TODO
    return EPERM;
}

/*
* Test to see if device is present.
* Return true if found and initialized ok.
*/
sdprobe(cp)
    struct mips_ctlr *cp;
{
    // TODO
    return 0;
}

/*
* Check for interrupts from all devices.
*/
void
sdintr(unit)
    int unit;
{
    // TODO
}

struct	driver sddriver = {
    "sd", sdprobe, 0, 0, sdintr,
};
#endif
