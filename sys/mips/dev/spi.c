/*
 * SPI driver for pic32.
 *
 * Copyright (C) 2014 Serge Vakulenko, <serge@vak.ru>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#include <spi.h>
#if NSPI > 0

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/spi.h>
#include <sys/systm.h>

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

struct spiio spitab[NSPI];

static const char pin_name[16] = "?ABCDEFGHJK?????";

/*
 * Default SPI bus speed in kHz.
 */
#ifndef SPI_KHZ
#define SPI_KHZ 10000
#endif

/* Convert port name/signal into a pin number. */
#define RP(x,n) (((x)-'A'+1) << 4 | (n))

/*
 * Setup SPI connection in default mode.
 * Use further function calls to set baud rate, clock phase, etc.
 */
int spi_setup(struct spiio *io, int unit, int pin)
{
    int channel = unit - 1;

    // Set up the device
    if (channel < 0 || channel >= NSPI)
        return ENXIO;

    io->reg = spitab[channel].reg;
    if (! io->reg)
        return ENXIO;

    io->mode = PIC32_SPICON_MSTEN | PIC32_SPICON_ON;
    io->cs_port = 0;
    spi_set_speed(io, SPI_KHZ);
    spi_set_cspin(io, pin);
    return 0;
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
        *data = spi_transfer(io, *data);
        data++;
    }
}

void spi_bulk_write16(struct spiio *io, unsigned int words, short *data)
{
    struct spireg *reg = io->reg;
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
void spi_bulk_write32(struct spiio *io, unsigned int words, int *data)
{
    struct spireg *reg = io->reg;
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

void spi_bulk_write32_be(struct spiio *io, unsigned int words, int *data)
{
    struct spireg *reg = io->reg;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = __bswap32__(*data++);
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
void spi_bulk_read32_be(struct spiio *io, unsigned int words, int *data)
{
    struct spireg *reg = io->reg;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = ~0;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *data++ = __bswap32__(reg->buf);
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_read32(struct spiio *io, unsigned int words, int *data)
{
    struct spireg *reg = io->reg;
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

void spi_bulk_read16(struct spiio *io, unsigned int words, short *data)
{
    struct spireg *reg = io->reg;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = ~0;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *data++ = __bswap16__(reg->buf);
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_rw32_be(struct spiio *io, unsigned int words, int *writep)
{
    struct spireg *reg = io->reg;
    int *readp = writep;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = *writep++;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *readp++ = __bswap32__(reg->buf);
            nread++;
        }
    }
    reg->con = io->mode;
}

void spi_bulk_rw32(struct spiio *io, unsigned int words, int *writep)
{
    struct spireg *reg = io->reg;
    int *readp = writep;
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

void spi_bulk_rw16(struct spiio *io, unsigned int words, short *writep)
{
    struct spireg *reg = io->reg;
    short *readp = writep;
    unsigned int nread = 0;
    unsigned int nwrite = words;

    reg->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while (nread < words) {
        if (nwrite > 0 && ! (reg->stat & PIC32_SPISTAT_SPITBF)) {
            reg->buf = *writep++;
            nwrite--;
        }
        if (! (reg->stat & PIC32_SPISTAT_SPIRBE)) {
            *readp++ = __bswap16__(reg->buf);
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
    static const char *name[6] = { "spi1", "spi2", "spi3", "spi4", "spi5", "spi6" };
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
    struct spiio *io = &spitab[channel];

    if (channel >= NSPI || ! io->reg)
        return ENXIO;

    if (curproc->p_ucred->cr_uid != 0)
        return EPERM;

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
    spi_set_cspin(&spitab[channel], 0);
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

    //PRINTDBG ("spi%d: ioctl (cmd=%08x, addr=%08x)\n", channel+1, cmd, addr);
    if (channel >= NSPI)
        return ENXIO;
    io = &spitab[channel];
    if (! io->reg)
        return ENODEV;

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
        spi_set_cspin(io, (unsigned) addr & 0xFF);
        return 0;

    case SPICTL_IO8(0):         /* transfer n*8 bits */
        spi_select(io);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (badaddr (addr, nelem))
            return EFAULT;
        spi_bulk_rw(io, nelem, cval);
        spi_deselect(io);
        break;

    case SPICTL_IO16(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) || badaddr (addr, nelem*2))
            return EFAULT;
        spi_bulk_rw16(io, nelem, (short*) addr);
        break;

    case SPICTL_IO32(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) || badaddr (addr, nelem*4))
            return EFAULT;
        spi_bulk_rw32(io, nelem, (int*) addr);
        break;

    case SPICTL_IO8R(0):         /* transfer n*8 bits */
        spi_select(io);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (badaddr (addr, nelem))
            return EFAULT;
        spi_bulk_read(io, nelem, cval);
        spi_deselect(io);
        break;

    case SPICTL_IO16R(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) || badaddr (addr, nelem*2))
            return EFAULT;
        spi_bulk_read16(io, nelem, (short*) addr);
        break;

    case SPICTL_IO32R(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) || badaddr (addr, nelem*4))
            return EFAULT;
        spi_bulk_read32(io, nelem, (int*) addr);
        break;

    case SPICTL_IO8W(0):         /* transfer n*8 bits */
        spi_select(io);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (badaddr (addr, nelem))
            return EFAULT;
        spi_bulk_write(io, nelem, cval);
        spi_deselect(io);
        break;

    case SPICTL_IO16W(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) || badaddr (addr, nelem*2))
            return EFAULT;
        spi_bulk_write16(io, nelem, (short*) addr);
        break;

    case SPICTL_IO32W(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) || badaddr (addr, nelem*4))
            return EFAULT;
        spi_bulk_write32(io, nelem, (int*) addr);
        break;

    case SPICTL_IO32RB(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) || badaddr (addr, nelem*4))
            return EFAULT;
        spi_bulk_read32_be(io, nelem, (int*) addr);
        break;

    case SPICTL_IO32WB(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) || badaddr (addr, nelem*4))
            return EFAULT;
        spi_bulk_write32_be(io, nelem, (int*) addr);
        break;

    case SPICTL_IO32B(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) || badaddr (addr, nelem*4))
            return EFAULT;
        spi_bulk_write32_be(io, nelem, (int*) addr);
        break;
    }
    return 0;
}

int gpio_input_map1(int pin)
{
    switch (pin) {
    case RP('D',2):  return 0;
    case RP('G',8):  return 1;
    case RP('F',4):  return 2;
    case RP('D',10): return 3;
    case RP('F',1):  return 4;
    case RP('B',9):  return 5;
    case RP('B',10): return 6;
    case RP('C',14): return 7;
    case RP('B',5):  return 8;
    case RP('C',1):  return 10;
    case RP('D',14): return 11;
    case RP('G',1):  return 12;
    case RP('A',14): return 13;
    case RP('D',6):  return 14;
    }
    printf ("spi: cannot map SDI pin %c%d, group 1\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

int gpio_input_map2(int pin)
{
    switch (pin) {
    case RP('D',3):  return 0;
    case RP('G',7):  return 1;
    case RP('F',5):  return 2;
    case RP('D',11): return 3;
    case RP('F',0):  return 4;
    case RP('B',1):  return 5;
    case RP('E',5):  return 6;
    case RP('C',13): return 7;
    case RP('B',3):  return 8;
    case RP('C',4):  return 10;
    case RP('D',15): return 11;
    case RP('G',0):  return 12;
    case RP('A',15): return 13;
    case RP('D',7):  return 14;
    }
    printf ("spi: cannot map SDI pin %c%d, group 2\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

int gpio_input_map3(int pin)
{
    switch (pin) {
    case RP('D',9):  return 0;
    case RP('G',6):  return 1;
    case RP('B',8):  return 2;
    case RP('B',15): return 3;
    case RP('D',4):  return 4;
    case RP('B',0):  return 5;
    case RP('E',3):  return 6;
    case RP('B',7):  return 7;
    case RP('F',12): return 9;
    case RP('D',12): return 10;
    case RP('F',8):  return 11;
    case RP('C',3):  return 12;
    case RP('E',9):  return 13;
    }
    printf ("spi: cannot map SDI pin %c%d, group 3\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

int gpio_input_map4(int pin)
{
    switch (pin) {
    case RP('D',1):  return 0;
    case RP('G',9):  return 1;
    case RP('B',14): return 2;
    case RP('D',0):  return 3;
    case RP('B',6):  return 5;
    case RP('D',5):  return 6;
    case RP('B',2):  return 7;
    case RP('F',3):  return 8;
    case RP('F',13): return 9;
    case RP('F',2):  return 11;
    case RP('C',2):  return 12;
    case RP('E',8):  return 13;
    }
    printf ("spi: cannot map SDI pin %c%d, group 3\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

/*
 * Assign SDIx signal to specified pin.
 */
static void assign_sdi(int channel, int pin)
{
    switch (channel) {
    case 0: SDI1R = gpio_input_map1(pin); break;
    case 1: SDI2R = gpio_input_map2(pin); break;
    case 2: SDI3R = gpio_input_map1(pin); break;
    case 3: SDI4R = gpio_input_map2(pin); break;
    case 4: SDI5R = gpio_input_map1(pin); break;
    case 5: SDI6R = gpio_input_map4(pin); break;
    }
}

static int output_map1 (unsigned channel)
{
    switch (channel) {
    case 0: return 5;   // 0101 = SDO1
    case 1: return 6;   // 0110 = SDO2
    case 2: return 7;   // 0111 = SDO3
    case 4: return 9;   // 1001 = SDO5
    }
    printf ("spi%u: cannot map SDO pin, group 1\n", channel);
    return 0;
}

static int output_map2 (unsigned channel)
{
    switch (channel) {
    case 0: return 5;   // 0101 = SDO1
    case 1: return 6;   // 0110 = SDO2
    case 2: return 7;   // 0111 = SDO3
    case 3: return 8;   // 1000 = SDO4
    case 4: return 9;   // 1001 = SDO5
    }
    printf ("spi%u: cannot map SDO pin, group 2\n", channel);
    return 0;
}

static int output_map3 (unsigned channel)
{
    switch (channel) {
    case 5: return 10;  // 1010 = SDO6
    }
    printf ("spi%u: cannot map SDO pin, group 3\n", channel);
    return 0;
}

static int output_map4 (unsigned channel)
{
    switch (channel) {
    case 3: return 8;   // 1000 = SDO4
    case 5: return 10;  // 1010 = SDO6
    }
    printf ("spi%u: cannot map SDO pin, group 4\n", channel);
    return 0;
}

/*
 * Assign SDOx signal to specified pin.
 */
static void assign_sdo(int channel, int pin)
{
    switch (pin) {
    case RP('A',14): RPA14R = output_map1(channel); return;
    case RP('A',15): RPA15R = output_map2(channel); return;
    case RP('B',0):  RPB0R  = output_map3(channel); return;
    case RP('B',10): RPB10R = output_map1(channel); return;
    case RP('B',14): RPB14R = output_map4(channel); return;
    case RP('B',15): RPB15R = output_map3(channel); return;
    case RP('B',1):  RPB1R  = output_map2(channel); return;
    case RP('B',2):  RPB2R  = output_map4(channel); return;
    case RP('B',3):  RPB3R  = output_map2(channel); return;
    case RP('B',5):  RPB5R  = output_map1(channel); return;
    case RP('B',6):  RPB6R  = output_map4(channel); return;
    case RP('B',7):  RPB7R  = output_map3(channel); return;
    case RP('B',8):  RPB8R  = output_map3(channel); return;
    case RP('B',9):  RPB9R  = output_map1(channel); return;
    case RP('C',13): RPC13R = output_map2(channel); return;
    case RP('C',14): RPC14R = output_map1(channel); return;
    case RP('C',1):  RPC1R  = output_map1(channel); return;
    case RP('C',2):  RPC2R  = output_map4(channel); return;
    case RP('C',3):  RPC3R  = output_map3(channel); return;
    case RP('C',4):  RPC4R  = output_map2(channel); return;
    case RP('D',0):  RPD0R  = output_map4(channel); return;
    case RP('D',10): RPD10R = output_map1(channel); return;
    case RP('D',11): RPD11R = output_map2(channel); return;
    case RP('D',12): RPD12R = output_map3(channel); return;
    case RP('D',14): RPD14R = output_map1(channel); return;
    case RP('D',15): RPD15R = output_map2(channel); return;
    case RP('D',1):  RPD1R  = output_map4(channel); return;
    case RP('D',2):  RPD2R  = output_map1(channel); return;
    case RP('D',3):  RPD3R  = output_map2(channel); return;
    case RP('D',4):  RPD4R  = output_map3(channel); return;
    case RP('D',5):  RPD5R  = output_map4(channel); return;
    case RP('D',6):  RPD6R  = output_map1(channel); return;
    case RP('D',7):  RPD7R  = output_map2(channel); return;
    case RP('D',9):  RPD9R  = output_map3(channel); return;
    case RP('E',3):  RPE3R  = output_map3(channel); return;
    case RP('E',5):  RPE5R  = output_map2(channel); return;
    case RP('E',8):  RPE8R  = output_map4(channel); return;
    case RP('E',9):  RPE9R  = output_map3(channel); return;
    case RP('F',0):  RPF0R  = output_map2(channel); return;
    case RP('F',12): RPF12R = output_map3(channel); return;
    case RP('F',13): RPF13R = output_map4(channel); return;
    case RP('F',1):  RPF1R  = output_map1(channel); return;
    case RP('F',2):  RPF2R  = output_map4(channel); return;
    case RP('F',3):  RPF3R  = output_map4(channel); return;
    case RP('F',4):  RPF4R  = output_map1(channel); return;
    case RP('F',5):  RPF5R  = output_map2(channel); return;
    case RP('F',8):  RPF8R  = output_map3(channel); return;
    case RP('G',0):  RPG0R  = output_map2(channel); return;
    case RP('G',1):  RPG1R  = output_map1(channel); return;
    case RP('G',6):  RPG6R  = output_map3(channel); return;
    case RP('G',7):  RPG7R  = output_map2(channel); return;
    case RP('G',8):  RPG8R  = output_map1(channel); return;
    case RP('G',9):  RPG9R  = output_map4(channel); return;
    }
    printf ("spi%u: cannot map SDO pin %c%d\n",
        channel, pin_name[pin>>4], pin & 15);
}

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 * SPI ports are always present, if configured.
 */
static int
spiprobe(config)
    struct mips_ctlr *config;
{
    int channel = config->mips_unit - 1;
    struct spiio *io = &spitab[channel];
    int sdi = config->mips_flags >> 8 & 0xFF;
    int sdo = config->mips_flags & 0xFF;
    int sck;
    static const int sck_tab[6] = {
        RP('D',1),  /* SCK1 */
        RP('G',6),  /* SCK2 */
        RP('B',14), /* SCK3 */
        RP('D',10), /* SCK4 */
        RP('F',13), /* SCK5 */
        RP('D',15), /* SCK6 */
    };

    if (channel < 0 || channel >= NSPI)
        return 0;
    sck = sck_tab[channel];
    printf ("spi%u at pins sdi=%c%d/sdo=%c%d/sck=%c%d\n", channel+1,
        pin_name[sdi>>4], sdi & 15,
        pin_name[sdo>>4], sdo & 15,
        pin_name[sck>>4], sck & 15);

    /* Assign SDI and SDO pins. */
    assign_sdi (channel, sdi);
    assign_sdo (channel, sdo);

    io->reg = spi_base[channel];
    spi_setup(io, 0, 0);
    return 1;
}

/*
* Check for interrupts from all devices.
*/
void
spiintr(channel)
    register int channel;
{
    // TODO
}

struct	driver spidriver = {
    "spi", spiprobe, 0, 0, spiintr,
};
#endif /* NSPI */
