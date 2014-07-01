/*
 * SPI driver for pic32.
 */
#include <spi.h>
#if NSPI > 0

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/spi.h>

#include <mips/dev/device.h>
#include <mips/dev/spi.h>
#include <machine/pic32mz.h>

/*
 * To enable debug output, uncomment the first line.
 */
//#define PRINTDBG printf
#ifndef PRINTDBG
#   define PRINTDBG(...) /*empty*/
#endif

/*
 * PIC32 port i/o registers.
 */
struct gpioreg {
    volatile unsigned ansel;            /* Analog select */
    volatile unsigned anselclr;
    volatile unsigned anselset;
    volatile unsigned anselinv;
    volatile unsigned tris;             /* Mask of inputs */
    volatile unsigned trisclr;
    volatile unsigned trisset;
    volatile unsigned trisinv;
    volatile unsigned port;             /* Read inputs, write outputs */
    volatile unsigned portclr;
    volatile unsigned portset;
    volatile unsigned portinv;
    volatile unsigned lat;              /* Read/write outputs */
    volatile unsigned latclr;
    volatile unsigned latset;
    volatile unsigned latinv;
    volatile unsigned odc;              /* Open drain configuration */
    volatile unsigned odcclr;
    volatile unsigned odcset;
    volatile unsigned odcinv;
    volatile unsigned cnpu;             /* Input pin pull-up enable */
    volatile unsigned cnpuclr;
    volatile unsigned cnpuset;
    volatile unsigned cnpuinv;
    volatile unsigned cnpd;             /* Input pin pull-down enable */
    volatile unsigned cnpdclr;
    volatile unsigned cnpdset;
    volatile unsigned cnpdinv;
    volatile unsigned cncon;            /* Interrupt-on-change control */
    volatile unsigned cnconclr;
    volatile unsigned cnconset;
    volatile unsigned cnconinv;
    volatile unsigned cnen;             /* Input change interrupt enable */
    volatile unsigned cnenclr;
    volatile unsigned cnenset;
    volatile unsigned cneninv;
    volatile unsigned cnstat;           /* Change notification status */
    volatile unsigned cnstatclr;
    volatile unsigned cnstatset;
    volatile unsigned cnstatinv;
    volatile unsigned unused[6*4];
};

#if NSPI > 6
#error Max 6 SPI ports supported.
#endif
static struct spireg *const spi_base[6] = {
    (struct spireg*) &SPI1CON,
    (struct spireg*) &SPI2CON,
    (struct spireg*) &SPI3CON,
    (struct spireg*) &SPI4CON,
    (struct spireg*) &SPI5CON,
    (struct spireg*) &SPI6CON,
};

struct spictl {
    struct spiio io;
} spitab[NSPI];

/*
 * Default SPI bus speed in kHz.
 */
#ifndef SPI_KHZ
#define SPI_KHZ 10000
#endif

/*
 * Setup SPI connection in default mode.
 * Use further function calls to set baud rate, clock phase, etc.
 */
void spi_setup(struct spiio *io, int port, int pin)
{
    // Set up the device
    io->reg     = spi_base[port];
    io->mode    = PIC32_SPICON_MSTEN | PIC32_SPICON_ON;
    io->cs_port = 0;
    spi_set_speed(io, SPI_KHZ);
    spi_set_cspin(io, pin);
}

/*
 * Setup the chip select pin for the SPI device.
 * Chip select pin is encoded as 0bPPPPNNNN, or 0xPN, where:
 * N is the pin number 0..F,
 * P is the port index 1..A:
 *   1 - port A
 *   2 - port B
 *   3 - port C
 *   4 - port D
 *   5 - port E
 *   6 - port F
 *   7 - port G
 *   8 - port H
 *   9 - port J
 *   A - port K
 */
void spi_set_cspin(struct spiio *io, int pin)
{
    /* Release the old CS pin. */
    if (io->cs_port) {
        /* Configure the chip select pin as input. */
        io->cs_port->trisset = io->cs_mask;
    }
    io->cs_port = 0;
    io->cs_pin  = 0;
    io->cs_mask = 0;

    if (pin >= 0x10) {
        struct gpioreg *cs_port = (struct gpioreg*) &ANSELA;

        cs_port += (pin >> 4 & 15) - 1;
        io->cs_port = cs_port;
        io->cs_pin  = pin & 15;
        io->cs_mask = 1 << io->cs_pin;

        /* Configure the chip select pin as output high. */
        cs_port->latset  = io->cs_mask;
        cs_port->trisclr = io->cs_mask;
    }
}

/*
 * Set the SPI bit rate for a device (in kHz).
 */
void spi_set_speed(struct spiio *io, unsigned int khz)
{
    io->divisor = (CPU_KHZ / khz + 1) / 2 - 1;
}

/*
 * Assert the CS pin of a device.
 * Not only do we set the CS pin, but before we do so we also reconfigure
 * the SPI bus to the required settings for this device.
 */
void spi_select(struct spiio *io)
{
    if (io->cs_port) {
        io->reg->brg = io->divisor;
        io->reg->con = io->mode;

        io->cs_port->latclr = io->cs_mask;
    }
}

/*
 * Deassert the CS pin of a device.
 */
void spi_deselect(struct spiio *io)
{
    if (io->cs_port) {
        io->cs_port->latset = io->cs_mask;
    }
}

/*
 * Set a mode setting or two - just updates the internal records,
 * the actual mode is changed next time the CS is asserted.
 */
void spi_set(struct spiio *io, unsigned int set)
{
    io->mode |= set;
}

/*
 * Clear a mode setting or two - just updates the internal records,
 * the actual mode is changed next time the CS is asserted.
 */
void spi_clr(struct spiio *io, unsigned int set)
{
    io->mode &= ~set;
}

/*
 * Return the current status of the SPI bus for the device in question
 * Just returns the ->stat entry in the register set.
 */
unsigned int spi_status(struct spiio *io)
{
    if (! io->reg)
        return 0;
    return io->reg->stat;
}

/*
 * Transfer one word of data, and return the read word of data.
 * The actual number of bits sent depends on the mode of the transfer.
 * This is blocking, and waits for the transfer to complete
 * before returning.  Times out after a certain period.
 */
unsigned spi_transfer(struct spiio *io, unsigned data)
{
    struct spireg *reg = io->reg;
    unsigned int cnt = 100000;

    reg->con = io->mode;
    reg->brg = io->divisor;
    reg->buf = data;
    while (! (reg->stat & PIC32_SPISTAT_SPIRBF) && --cnt > 0)
        asm volatile ("nop");
    if (cnt == 0)
        return -1;

    return reg->buf;
}

/*
 * Send a chunk of 8-bit data.
 */
void spi_bulk_write(struct spiio *io, unsigned int nbytes, unsigned char *data)
{
    unsigned i;

    for (i=0; i<nbytes; i++) {
        spi_transfer(io, *data++);
    }
}

/*
 * Receive a chunk of 8-bit data.
 */
void spi_bulk_read(struct spiio *io, unsigned int nbytes, unsigned char *data)
{
    unsigned i;

    for(i=0; i<nbytes; i++) {
        *data++ = spi_transfer(io, 0xFF);
    }
}

/*
 * Send and receive a chunk of 8-bit data.
 */
void spi_bulk_rw(struct spiio *io, unsigned int nbytes, unsigned char *data)
{
    unsigned int i;

    for(i=0; i<nbytes; i++) {
        *data++ = spi_transfer(io, *data);
    }
}

void spi_bulk_write16(struct spiio *io, unsigned int nbytes, short *data)
{
    struct spireg *reg = io->reg;
    unsigned int words = nbytes >> 1;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = *data++;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            (void) reg->buf;
            nread++;
        }
    }
    reg->con = io->mode;
}

/*
 * Write a chunk of 32-bit data as fast as possible.
 * Switches in to 32-bit mode regardless, and uses the enhanced buffer mode.
 * Data should be a multiple of 32 bits.
 */
void spi_bulk_write32(struct spiio *io, unsigned int nbytes, int *data)
{
    struct spireg *reg = io->reg;
    unsigned int words = nbytes >> 2;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = *data++;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            (void) reg->buf;
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_write32_be(struct spiio *io, unsigned int nbytes, int *data)
{
    struct spireg *reg = io->reg;
    unsigned int words = nbytes >> 2;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = mips_bswap(*data++);
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            (void) reg->buf;
            nread++;
        }
    }
    reg->con = io->mode;
}

// Read a huge chunk of data as fast and as efficiently as
// possible.  Switches in to 32-bit mode regardless, and uses
// the enhanced buffer mode.
// Data should be a multiple of 32 bits.
void spi_bulk_read32_be(struct spiio *io, unsigned int nbytes, int *data)
{
    struct spireg *reg = io->reg;
    unsigned int words = nbytes >> 2;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = ~0;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *data++ = mips_bswap(reg->buf);
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_read32(struct spiio *io, unsigned int nbytes, int *data)
{
    struct spireg *reg = io->reg;
    unsigned int words = nbytes >> 2;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = ~0;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *data++ = reg->buf;
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_read16(struct spiio *io, unsigned int nbytes, short *data)
{
    struct spireg *reg = io->reg;
    unsigned int words = nbytes >> 1;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = ~0;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *data++ = mips_bswap(reg->buf);
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_rw32_be(struct spiio *io, unsigned int nbytes, int *writep)
{
    struct spireg *reg = io->reg;
    int *readp = writep;
    unsigned int words = nbytes >> 2;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = *writep++;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *readp++ = mips_bswap(reg->buf);
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_rw32(struct spiio *io, unsigned int nbytes, int *writep)
{
    struct spireg *reg = io->reg;
    int *readp = writep;
    unsigned int words = nbytes >> 2;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = *writep++;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *readp++ = reg->buf;
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_rw16(struct spiio *io, unsigned int nbytes, short *writep)
{
    struct spireg *reg = io->reg;
    short *readp = writep;
    unsigned int words = nbytes >> 1;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = *writep++;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *readp++ = mips_bswap(reg->buf);
            nread++;
        }
    }
    reg->con = io->mode;
}

/*
 * Return the name of the SPI bus for a device.
 */
const char *spi_name(struct spiio *io)
{
    static const char *name[6] = { "SPI1", "SPI2", "SPI3", "SPI4", "SPI5", "SPI6" };
    int i;

    for (i=0; i<6; i++) {
        if (io->reg == spi_base[i])
            return name[i];
    }
    return "SPI?";
}

/*
 * Return the port name (A-K) of the chip select pin for a device.
 */
char spi_csname(struct spiio *io)
{
    unsigned n = io->cs_port - (struct gpioreg*) &ANSELA;

    if (n < 10)
        return "ABCDEFGHJK"[n];
    return '?';
}

/*
 * Return the pin index of the chip select pin for a device.
 */
int spi_cspin(struct spiio *io)
{
    if (! io->cs_port)
        return 0;

    return io->cs_pin;
}

/*
 * Return the speed in kHz.
 */
unsigned int spi_get_speed(struct spiio *io)
{
    if (! io->reg)
        return 0;

    return CPU_KHZ / (io->divisor + 1) / 2;
}

/*
 * Open /dev/spi# device.
 * Use default SPI parameters:
 * - rate 250 kHz;
 * - no sleect pin.
 */
int spiopen (dev_t dev, int flag, int mode)
{
    int channel = minor (dev);

    if (channel >= NSPI)
        return ENXIO;

    if (curproc->p_ucred->cr_uid != 0)
            return EPERM;

    spi_setup(&spitab[channel].io, 0, 0);
    return 0;
}

int spiclose (dev_t dev, int flag, int mode)
{
    int channel = minor (dev);

    if (channel >= NSPI)
        return ENXIO;

    if (curproc->p_ucred->cr_uid != 0)
            return EPERM;

    /* Release the chip select pin. */
    spi_set_cspin(&spitab[channel].io, 0);
    return 0;
}

int spiread (dev_t dev, struct uio *uio, int flag)
{
    return 0;
}

int spiwrite (dev_t dev, struct uio *uio, int flag)
{
    return 0;
}

/*
 * SPI control operations:
 * - SPICTL_SETMODE   - set clock polarity and phase
 * - SPICTL_SETRATE   - set data rate in kHz
 * - SPICTL_SETSELPIN - set select pin
 * - SPICTL_IO8(n)    - n*8 bit RW transaction
 * - SPICTL_IO16(n)   - n*16 bit RW transaction
 * - SPICTL_IO32(n)   - n*32 bit RW transaction
 * - SPICTL_IO8R(n)   - n*8 bit R transaction
 * - SPICTL_IO16R(n)  - n*16 bit R transaction
 * - SPICTL_IO32R(n)  - n*32 bit R transaction
 * - SPICTL_IO8W(n)   - n*8 bit W transaction
 * - SPICTL_IO16W(n)  - n*16 bit W transaction
 * - SPICTL_IO32W(n)  - n*32 bit W transaction
 * - SPICTL_IO32RB(n) - n*32 bit RB transaction (B - swaps byte's order)
 * - SPICTL_IO32WB(n) - n*32 bit WB transaction
 * - SPICTL_IO32B(n)  - n*32 bit B transaction
 */
int spiioctl (dev_t dev, u_int cmd, caddr_t addr, int flag)
{
    int channel = minor (dev);
    unsigned char *cval = (unsigned char *)addr;
    struct spiio *io;
    int nelem;
    static unsigned volatile *const tris[8] = {
        0, &TRISA,&TRISB,&TRISC,&TRISD,&TRISE,&TRISF,&TRISG,
    };
    int mask, portnum;

    //PRINTDBG ("spi%d: ioctl (cmd=%08x, addr=%08x)\n", channel+1, cmd, addr);
    if (channel >= NSPI)
        return ENXIO;
    io = &spitab[channel].io;

    switch (cmd & ~(IOCPARM_MASK << 16)) {
    default:
        return ENODEV;

    case SPICTL_SETMODE:        /* set SPI mode */
        /*      --- Clock ----
         * Mode Polarity Phase
         *   0     0       0
         *   1     0       1
         *   2     1       0
         *   3     1       1
         */
        if ((unsigned int) addr & 0x01)
            spi_set(io, PIC32_SPICON_CKE);
        if ((unsigned int) addr & 0x02)
            spi_set(io, PIC32_SPICON_CKP);
        return 0;

    case SPICTL_SETRATE:        /* set clock rate, kHz */
        spi_set_speed(io, (unsigned int) addr);
        return 0;

    case SPICTL_SETSELPIN:      /* set select pin */
        mask = 1 << ((unsigned int) addr & 15);
        portnum = ((unsigned int) addr >> 8) & 7;
        if (! portnum)
            return 0;
        spi_set_cspin(io, (unsigned) addr & 0xFF);
        return 0;

    case SPICTL_IO8(0):         /* transfer n*8 bits */
        spi_select(io);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (baduaddr (addr) || baduaddr (addr + nelem - 1))
            return EFAULT;
        spi_bulk_rw(io, nelem, cval);
        spi_deselect(io);
        break;

    case SPICTL_IO16(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) ||
            baduaddr (addr) || baduaddr (addr + nelem*2 - 1))
            return EFAULT;
        spi_bulk_rw16(io, nelem<<1, (short*) addr);
        break;

    case SPICTL_IO32(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_rw32(io, nelem<<2, (int*) addr);
        break;

    case SPICTL_IO8R(0):         /* transfer n*8 bits */
        spi_select(io);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (baduaddr (addr) || baduaddr (addr + nelem - 1))
            return EFAULT;
        spi_bulk_read(io, nelem, cval);
        spi_deselect(io);
        break;

    case SPICTL_IO16R(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) ||
            baduaddr (addr) || baduaddr (addr + nelem*2 - 1))
            return EFAULT;
        spi_bulk_read16(io, nelem<<1, (short*) addr);
        break;

    case SPICTL_IO32R(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_read32(io, nelem<<2, (int*) addr);
        break;

    case SPICTL_IO8W(0):         /* transfer n*8 bits */
        spi_select(io);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (baduaddr (addr) || baduaddr (addr + nelem - 1))
            return EFAULT;
        spi_bulk_write(io, nelem, cval);
        spi_deselect(io);
        break;

    case SPICTL_IO16W(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) ||
            baduaddr (addr) || baduaddr (addr + nelem*2 - 1))
            return EFAULT;
        spi_bulk_write16(io, nelem<<1, (short*) addr);
        break;

    case SPICTL_IO32W(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_write32(io, nelem<<2, (int*) addr);
        break;

    case SPICTL_IO32RB(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_read32_be(io, nelem<<2, (int*) addr);
        break;

    case SPICTL_IO32WB(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_write32_be(io, nelem<<2, (int*) addr);
        break;

    case SPICTL_IO32B(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_write32_be(io, nelem<<2, (int*) addr);
        break;
    }
    return 0;
}

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 */
static int
spiprobe(config)
    struct scsi_device *config;
{
    int unit = config->sd_unit - 1;

    if (unit < 0 || unit >= NSPI)
        return 0;

    // TODO

    printf ("spi%u: sdi/sdo pins %c%d, %c%d\n",
        unit, ???);
    return 1;
}

/*
* Check for interrupts from all devices.
*/
void
spiintr(unit)
    register int unit;
{
    // TODO
}

struct	driver spidriver = {
    "spi", spiprobe, 0, 0, spiintr,
};
#endif /* NSPI */
