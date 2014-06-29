/*
 * SD flash card disk driver.
 */
#include "sd.h"
#if NSD > 0

#include <sys/param.h>
#include <sys/buf.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>

#include <mips/dev/device.h>
#include <machine/pic32mz.h>

#define sdunit(dev)     ((minor(dev) & 8) >> 3)
#define sdpart(dev)     ((minor(dev) & 7))
#define RAWPART         0               /* 'x' partition */

#define NPARTITIONS     4
#define SECTSIZE        512
#define MBR_MAGIC       0xaa55

#ifndef SD_KHZ
#define SD_KHZ          13333           /* speed 13.33 MHz */
#endif

#if DEV_BSIZE != 512
#error Only 512-byte block size supported.
#endif

struct part {
    u_int8_t status;        /* active (bootable) flag */
#define P_ACTIVE 0x80
    u_int8_t start[3];      /* ignored */
    u_int8_t type;          /* type of partition */
    u_int8_t end[3];        /* ignored */
    u_int32_t lba_start;    /* offset in 512-byte sectors */
    u_int32_t lba_length;   /* length in 512-byte sectors */
};

/*
 * The structure of a disk drive.
 */
struct disk {
    /* the partition table */
    struct part dk_part [NPARTITIONS+1];

    int     dk_bc;          /* byte count left */
    int     dk_unit;        /* physical unit number */
    u_int   dk_status;      /* copy of status reg. */
    u_int   dk_error;       /* copy of error reg. */
    int     dk_open;        /* open/closed refcnt */
    u_int   dk_copenpart;   /* character units open on this drive */
    u_int   dk_bopenpart;   /* block units open on this drive */
    u_int   dk_openpart;    /* all units open on this drive */
    int     dk_wlabel;      /* label writable? */
};

static struct disk sddrives[NSD];       /* Table of units */
static int sd_type[NSD];                /* Card type */
static int sd_fd[NSD];                  /* SPI descriptor */

#define TIMO_WAIT_WDONE 400000
#define TIMO_WAIT_WIDLE 200000
#define TIMO_WAIT_CMD   100000
#define TIMO_WAIT_WDATA 30000
#define TIMO_READ       90000
#define TIMO_SEND_OP    8000
#define TIMO_CMD        7000
#define TIMO_SEND_CSD   6000
#define TIMO_WAIT_WSTOP 5000

int sd_timo_cmd;                /* Max timeouts, for sysctl */
int sd_timo_send_op;
int sd_timo_send_csd;
int sd_timo_read;
int sd_timo_wait_cmd;
int sd_timo_wait_wdata;
int sd_timo_wait_wdone;
int sd_timo_wait_wstop;
int sd_timo_wait_widle;

/*
 * Definitions for MMC/SDC commands.
 */
#define CMD_GO_IDLE             0       /* CMD0 */
#define CMD_SEND_OP_MMC         1       /* CMD1 (MMC) */
#define CMD_SEND_IF_COND        8
#define CMD_SEND_CSD            9
#define CMD_SEND_CID            10
#define CMD_STOP                12
#define CMD_SEND_STATUS         13      /* CMD13 */
#define CMD_SET_BLEN            16
#define CMD_READ_SINGLE         17
#define CMD_READ_MULTIPLE       18
#define CMD_SET_BCOUNT          23      /* (MMC) */
#define CMD_SET_WBECNT          23      /* ACMD23 (SDC) */
#define CMD_WRITE_SINGLE        24
#define CMD_WRITE_MULTIPLE      25
#define CMD_SEND_OP_SDC         41      /* ACMD41 (SDC) */
#define CMD_APP                 55      /* CMD55 */
#define CMD_READ_OCR            58

#define DATA_START_BLOCK        0xFE    /* start data for single block */
#define STOP_TRAN_TOKEN         0xFD    /* stop token for write multiple */
#define WRITE_MULTIPLE_TOKEN    0xFC    /* start data for write multiple */

/*
 * Wait while busy, up to 300 msec.
 */
static void spi_wait_ready (int unit, int limit, int *maxcount)
{
    int i;

    spi_transfer(sd_fd[unit], 0xFF);
    for (i=0; i<limit; i++)
    {
        if (spi_transfer(sd_fd[unit], 0xFF) == 0xFF)
        {
            if (*maxcount < i)
                *maxcount = i;
            return;
        }
    }
    printf ("sd%d: wait_ready(%d) failed\n",unit, limit);
}

/*
 * Send a command and address to SD media.
 * Return response:
 *   FF - timeout
 *   00 - command accepted
 *   01 - command received, card in idle state
 *
 * Other codes:
 *   bit 0 = Idle state
 *   bit 1 = Erase Reset
 *   bit 2 = Illegal command
 *   bit 3 = Communication CRC error
 *   bit 4 = Erase sequence error
 *   bit 5 = Address error
 *   bit 6 = Parameter error
 *   bit 7 = Always 0
 */
static int card_cmd(unsigned int unit, unsigned int cmd, unsigned int addr)
{
    int i, reply;

    /* Wait for not busy, up to 300 msec. */
    if (cmd != CMD_GO_IDLE)
        spi_wait_ready(unit, TIMO_WAIT_CMD, &sd_timo_wait_cmd);

    /* Send a comand packet (6 bytes). */
    spi_transfer(sd_fd[unit], cmd | 0x40);
    spi_transfer(sd_fd[unit], addr >> 24);
    spi_transfer(sd_fd[unit], addr >> 16);
    spi_transfer(sd_fd[unit], addr >> 8);
    spi_transfer(sd_fd[unit], addr);

    /* Send cmd checksum for CMD_GO_IDLE.
     * For all other commands, CRC is ignored. */
    if (cmd == CMD_GO_IDLE)
        spi_transfer(sd_fd[unit], 0x95);
    else if (cmd == CMD_SEND_IF_COND)
        spi_transfer(sd_fd[unit], 0x87);
    else
        spi_transfer(sd_fd[unit], 0xFF);

    /* Wait for a response. */
    for (i=0; i<TIMO_CMD; i++)
    {
        reply = spi_transfer(sd_fd[unit], 0xFF);
        if (! (reply & 0x80))
        {
            if (sd_timo_cmd < i)
                sd_timo_cmd = i;
            return reply;
        }
    }
    if (cmd != CMD_GO_IDLE)
    {
        printf ("sd%d: card_cmd timeout, cmd=%02x, addr=%08x, reply=%02x\n",
            unit,cmd, addr, reply);
    }
    return reply;
}

/*
 * Add extra clocks after a deselect
 */
static inline void
sd_deselect(unsigned int fd)
{
    spi_deselect(fd);
    spi_transfer(fd, 0xFF);
}

/*
 * Initialize a card.
 * Return nonzero if successful.
 */
static int card_init(int unit)
{
    int i, reply;
    unsigned char response[4];
    int timeout = 4;

    /* Slow speed: 250 kHz */
    spi_brg(sd_fd[unit], 250);

    sd_type[unit] = 0;

    do {
        /* Unselect the card. */
        sd_deselect(sd_fd[unit]);

        /* Send 80 clock cycles for start up. */
        for (i=0; i<10; i++)
            spi_transfer(sd_fd[unit], 0xFF);

        /* Select the card and send a single GO_IDLE command. */
        spi_select(sd_fd[unit]);
        timeout--;
        reply = card_cmd(unit, CMD_GO_IDLE, 0);

    } while ((reply != 0x01) && (timeout != 0));

    sd_deselect(sd_fd[unit]);
    if (reply != 1)
    {
        /* It must return Idle. */
        return 0;
    }

    /* Check SD version. */
    spi_select(sd_fd[unit]);
    reply = card_cmd(unit, CMD_SEND_IF_COND, 0x1AA);
    if (reply & 4)
    {
        /* Illegal command: card type 1. */
        sd_deselect(sd_fd[unit]);
        sd_type[unit] = 1;
    } else {
        response[0] = spi_transfer(sd_fd[unit], 0xFF);
        response[1] = spi_transfer(sd_fd[unit], 0xFF);
        response[2] = spi_transfer(sd_fd[unit], 0xFF);
        response[3] = spi_transfer(sd_fd[unit], 0xFF);
        sd_deselect(sd_fd[unit]);
        if (response[3] != 0xAA)
        {
            printf ("sd%d: cannot detect card type, response=%02x-%02x-%02x-%02x\n",
                unit, response[0], response[1], response[2], response[3]);
            return 0;
        }
        sd_type[unit] = 2;
    }


    /* Send repeatedly SEND_OP until Idle terminates. */
    for (i=0; ; i++)
    {
        spi_select(sd_fd[unit]);
        card_cmd(unit,CMD_APP, 0);
        reply = card_cmd(unit,CMD_SEND_OP_SDC,
                        (sd_type[unit] == 2) ? 0x40000000 : 0);
        spi_select(sd_fd[unit]);
        if (reply == 0)
            break;
        if (i >= TIMO_SEND_OP)
        {
            /* Init timed out. */
            printf ("card_init: SEND_OP timed out, reply = %d\n", reply);
            return 0;
        }
    }
    if (sd_timo_send_op < i)
        sd_timo_send_op = i;

    /* If SD2 read OCR register to check for SDHC card. */
    if (sd_type[unit] == 2)
    {
        spi_select(sd_fd[unit]);
        reply = card_cmd(unit, CMD_READ_OCR, 0);
        if (reply != 0)
        {
            sd_deselect(sd_fd[unit]);
            printf ("sd%d: READ_OCR failed, reply=%02x\n", unit, reply);
            return 0;
        }
        response[0] = spi_transfer(sd_fd[unit], 0xFF);
        response[1] = spi_transfer(sd_fd[unit], 0xFF);
        response[2] = spi_transfer(sd_fd[unit], 0xFF);
        response[3] = spi_transfer(sd_fd[unit], 0xFF);
        sd_deselect(sd_fd[unit]);
        if ((response[0] & 0xC0) == 0xC0)
        {
            sd_type[unit] = 3;
        }
    }

    /* Fast speed. */
    spi_brg(sd_fd[unit], SD_KHZ);
    return 1;
}

/*
 * Get number of sectors on the disk.
 * Return nonzero if successful.
 */
static int card_size(int unit)
{
    unsigned char csd [16];
    unsigned csize, n;
    int reply, i;
    int nsectors;

    spi_select(sd_fd[unit]);
    reply = card_cmd(unit,CMD_SEND_CSD, 0);
    if (reply != 0)
    {
        /* Command rejected. */
        sd_deselect(sd_fd[unit]);
        return 0;
    }
    /* Wait for a response. */
    for (i=0; ; i++)
    {
        reply = spi_transfer(sd_fd[unit], 0xFF);
        if (reply == DATA_START_BLOCK)
            break;
        if (i >= TIMO_SEND_CSD)
        {
            /* Command timed out. */
            sd_deselect(sd_fd[unit]);
            printf ("sd%d: card_size: SEND_CSD timed out, reply = %d\n",
                unit, reply);
            return 0;
        }
    }
    if (sd_timo_send_csd < i)
        sd_timo_send_csd = i;

    /* Read data. */
    for (i=0; i<sizeof(csd); i++)
    {
        csd [i] = spi_transfer(sd_fd[unit], 0xFF);
    }
    /* Ignore CRC. */
    spi_transfer(sd_fd[unit], 0xFF);
    spi_transfer(sd_fd[unit], 0xFF);

    /* Disable the card. */
    sd_deselect(sd_fd[unit]);

    /* CSD register has different structure
     * depending upon protocol version. */
    switch (csd[0] >> 6)
    {
        case 1:                 /* SDC ver 2.00 */
            csize = csd[9] + (csd[8] << 8) + 1;
            nsectors = csize << 10;
            break;
        case 0:                 /* SDC ver 1.XX or MMC. */
            n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            csize = (csd[8] >> 6) + (csd[7] << 2) + ((csd[6] & 3) << 10) + 1;
            nsectors = csize << (n - 9);
            break;
        default:                /* Unknown version. */
            return 0;
    }
    return nsectors>>1;
}

/*
 * Read a block of data.
 * Return nonzero if successful.
 */
static int
card_read(int unit, unsigned int offset, char *data, unsigned int bcount)
{
    int reply, i;

    /* Send read-multiple command. */
    spi_select(sd_fd[unit]);
    if (sd_type[unit] != 3) offset <<= 9;
    reply = card_cmd(unit, CMD_READ_MULTIPLE, offset<<1);
    if (reply != 0)
    {
        /* Command rejected. */
        printf ("sd%d: card_read: bad READ_MULTIPLE reply = %d, offset = %08x\n",
            unit, reply, offset<<1);
        sd_deselect(sd_fd[unit]);
        return 0;
    }

again:
    /* Wait for a response. */
    for (i=0; ; i++)
    {
        int x = spl0();
        reply = spi_transfer(sd_fd[unit], 0xFF);
        splx(x);
        if (reply == DATA_START_BLOCK)
            break;
        if (i >= TIMO_READ)
        {
            /* Command timed out. */
            printf ("sd%d: card_read: READ_MULTIPLE timed out, reply = %d\n",
                unit, reply);
            sd_deselect(sd_fd[unit]);
            return 0;
        }
    }
    if (sd_timo_read < i)
        sd_timo_read = i;

    /* Read data. */
    if (bcount >= SECTSIZE)
    {
        spi_bulk_read_32_be(sd_fd[unit], SECTSIZE, data);
        data += SECTSIZE;
    } else {
        spi_bulk_read(sd_fd[unit], bcount, (unsigned char *)data);
        data += bcount;
        for (i=bcount; i<SECTSIZE; i++)
            spi_transfer(sd_fd[unit], 0xFF);
    }
    /* Ignore CRC. */
    spi_transfer(sd_fd[unit], 0xFF);
    spi_transfer(sd_fd[unit], 0xFF);

    if (bcount > SECTSIZE)
    {
        /* Next sector. */
        bcount -= SECTSIZE;
        goto again;
    }

    /* Stop a read-multiple sequence. */
    card_cmd(unit, CMD_STOP, 0);
    sd_deselect(sd_fd[unit]);
    return 1;
}

/*
 * Write a block of data.
 * Return nonzero if successful.
 */
static int
card_write (int unit, unsigned offset, char *data, unsigned bcount)
{
    unsigned reply, i;

    /* Send pre-erase count. */
    spi_select(sd_fd[unit]);
    card_cmd(unit, CMD_APP, 0);
    reply = card_cmd(unit, CMD_SET_WBECNT, (bcount + SECTSIZE - 1) / SECTSIZE);
    if (reply != 0)
    {
        /* Command rejected. */
        sd_deselect(sd_fd[unit]);
        printf("sd%d: card_write: bad SET_WBECNT reply = %02x, count = %u\n",
            unit, reply, (bcount + SECTSIZE - 1) / SECTSIZE);
        return 0;
    }

    /* Send write-multiple command. */
    if (sd_type[unit] != 3) offset <<= 9;
    reply = card_cmd(unit, CMD_WRITE_MULTIPLE, offset<<1);
    if (reply != 0)
    {
        /* Command rejected. */
        sd_deselect(sd_fd[unit]);
        printf("sd%d: card_write: bad WRITE_MULTIPLE reply = %02x\n", unit, reply);
        return 0;
    }
    sd_deselect(sd_fd[unit]);
again:
    /* Select, wait while busy. */
    spi_select(sd_fd[unit]);
    spi_wait_ready(unit, TIMO_WAIT_WDATA, &sd_timo_wait_wdata);

    /* Send data. */
    spi_transfer(sd_fd[unit], WRITE_MULTIPLE_TOKEN);
    if (bcount >= SECTSIZE)
    {
        spi_bulk_write_32_be(sd_fd[unit], SECTSIZE, data);
        data += SECTSIZE;
    } else {
        spi_bulk_write(sd_fd[unit], bcount, (unsigned char *)data);
        data += bcount;
        for (i=bcount; i<SECTSIZE; i++)
            spi_transfer(sd_fd[unit], 0xFF);
    }
    /* Send dummy CRC. */
    spi_transfer(sd_fd[unit], 0xFF);
    spi_transfer(sd_fd[unit], 0xFF);

    /* Check if data accepted. */
    reply = spi_transfer(sd_fd[unit], 0xFF);
    if ((reply & 0x1f) != 0x05)
    {
        /* Data rejected. */
        sd_deselect(sd_fd[unit]);
        printf("sd%d: card_write: data rejected, reply = %02x\n", unit,reply);
        return 0;
    }

    /* Wait for write completion. */
    int x = spl0();
    spi_wait_ready(unit, TIMO_WAIT_WDONE, &sd_timo_wait_wdone);
    splx(x);
    sd_deselect(sd_fd[unit]);

    if (bcount > SECTSIZE)
    {
        /* Next sector. */
        bcount -= SECTSIZE;
        goto again;
    }

    /* Stop a write-multiple sequence. */
    spi_select(sd_fd[unit]);
    spi_wait_ready(unit, TIMO_WAIT_WSTOP, &sd_timo_wait_wstop);
    spi_transfer(sd_fd[unit], STOP_TRAN_TOKEN);
    spi_wait_ready(unit, TIMO_WAIT_WIDLE, &sd_timo_wait_widle);
    sd_deselect(sd_fd[unit]);
    return 1;
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
    struct disk *u;
    int unit = sdunit(dev);
    unsigned part = sdpart(dev);
    unsigned mask, i;

    if (unit >= NSD || part > NPARTITIONS)
        return ENXIO;
    u = &sddrives[unit];
    u->dk_unit = unit;

    /*
     * Initialize the SD card interface.
     * Get the card type and size.
     */
    if (u->dk_open == 0) {
        if (! card_init(unit)) {
            printf ("sd%d: no SD card detected\n", unit);
            return ENODEV;
        }
        /* Get the size of raw partition. */
        bzero (u->dk_part, sizeof(u->dk_part));
        u->dk_part[RAWPART].lba_start = 0;
        u->dk_part[RAWPART].lba_length = card_size(unit);
        if (u->dk_part[RAWPART].lba_length == 0) {
            printf ("sd%d: cannot get card size\n", unit);
            return ENODEV;
        }
        printf ("sd%d: type %s, size %u kbytes, speed %u Mbit/sec\n", unit,
            sd_type[unit]==3 ? "SDHC" : sd_type[unit]==2 ? "II" : "I",
            u->dk_part[RAWPART].lba_length, spi_get_brg(sd_fd[unit]) / 1000);

        /* Read partition table. */
        u_int16_t buf[256];
        int s = splbio();
        if (! card_read(unit, 0, (char*)buf, sizeof(buf))) {
            splx(s);
            printf ("sd%d: cannot read partition table\n", unit);
            return ENODEV;
        }
        splx(s);
        if (buf[255] == MBR_MAGIC) {
            bcopy (&buf[223], &u->dk_part[1], 64);
#if 1
            for (i=1; i<=NPARTITIONS; i++) {
                if (u->dk_part[i].type != 0)
                    printf ("rd%d%c: type %02X, start %u, length %u\n",
                        unit, i+'a'-1, u->dk_part[i].type,
                        u->dk_part[i].lba_start,
                        u->dk_part[i].lba_length);
            }
#endif
        }
    }
    u->dk_open++;

    /*
     * Warn if a partion is opened
     * that overlaps another partition which is open
     * unless one is the "raw" partition (whole disk).
     */
    mask = 1 << part;
    if (part != RAWPART && ! (u->dk_openpart & mask)) {
        unsigned start = u->dk_part[part].lba_start;
        unsigned end = start + u->dk_part[part].lba_length;

        /* Check for overlapped partitions. */
        for (i=0; i<=NPARTITIONS; i++) {
            struct part *pp = &u->dk_part[i];

            if (i == part || i == RAWPART)
                continue;

            if (pp->lba_start + pp->lba_length <= start ||
                pp->lba_start >= end)
                continue;

            if (u->dk_openpart & (1 << i))
                log(LOG_WARNING, "sd%d%c: overlaps open partition (sd%d%c)\n",
                    unit, part + 'a' - 1,
                    unit, pp - u->dk_part + 'a' - 1);
        }
    }

    u->dk_openpart |= mask;
    switch (mode) {
    case S_IFCHR:
        u->dk_copenpart |= mask;
        break;
    case S_IFBLK:
        u->dk_bopenpart |= mask;
        break;
    }
    return 0;
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
    struct disk *u;    /* Disk unit to do the IO.  */
    int unit = sdunit(bp->b_dev);
    int s;
    unsigned offset = 0;

    if (unit >= NSD || bp->b_blkno < 0) {
        printf("sdstrategy: unit = %d, blkno = %d, bcount = %d\n",
            unit, bp->b_blkno, bp->b_bcount);
        bp->b_flags |= B_ERROR;
        goto bad;
    }
    u = &sddrives[unit];
    if (u->dk_open) {
        /*
         * Determine the size of the transfer, and make sure it is
         * within the boundaries of the partition.
         */
        struct part *p = &u->dk_part[sdpart(bp->b_dev)];
        long maxsz = p->lba_length;
        long sz = (bp->b_bcount + DEV_BSIZE - 1) >> DEV_BSHIFT;

        offset = p->lba_start;
        if (bp->b_blkno + offset == 0 &&
            ! (bp->b_flags & B_READ) && ! u->dk_wlabel) {
                /* Write to partition table not allowed. */
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

    s = splbio();
    if (bp->b_flags & B_READ) {
        card_read(unit, offset, bp->b_un.b_addr, bp->b_bcount);
    } else {
        card_write(unit, offset, bp->b_un.b_addr, bp->b_bcount);
    }
    biodone(bp);
    splx(s);
    return;

bad:
    bp->b_error = EINVAL;
    biodone(bp);
}

int
sdsize(dev)
    dev_t dev;
{
    int unit = sdunit(dev);

    return card_size(unit);
}

int
sdioctl(dev, cmd, data, flag, p)
    dev_t dev;
    u_long cmd;
    caddr_t data;
    int flag;
    struct proc *p;
{
    int unit = sdunit(dev);
    struct disk *du = &sddrives[unit];
    int error = 0;

    switch (cmd) {

    case DIOCWLABEL:
        if (! (flag & FWRITE)) {
            error = EBADF;
            break;
        }
        /* Enable/disable modification of partition table. */
        du->dk_wlabel = *(int *)data;
        break;

    default:
        error = ENOTTY;
        break;
    }
    return (error);
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
    return physio(sdstrategy, 0, dev, B_READ, minphys, uio);
}

int
sdwrite(dev, uio)
    dev_t dev;
    struct uio *uio;
{
    return physio(sdstrategy, 0, dev, B_WRITE, minphys, uio);
}

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 */
static int
sdprobe(config)
    struct scsi_device *config;
{
    int unit = config->sd_unit;
    int fd;

    if (unit < 0 || unit >= NSD)
        return 0;

    fd = spi_open(config->sd_ctlr, config->sd_flags);
    if (fd < 0) {
        printf("sd: Cannot open SPI port\n");
        return 0;
    }

    sd_fd[unit] = fd;

    spi_brg(fd, SD_KHZ);
    spi_set(fd, PIC32_SPICON_CKE);

    printf ("sd: port %s, select pin %c%d\n",
        spi_name(fd), spi_csname(fd), spi_cspin(fd));
    return 1;
}

/*
 * Check for interrupts from all devices.
 */
static void
sdintr(unit)
    int unit;
{
    // TODO
}

struct driver sddriver = {
    "sd", sdprobe, 0, 0, sdintr,
};
#endif
