/*
 * SPI driver for pic32.
 */
#include <spi.h>
#if NSPI > 0

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/spi.h>

#include <mips/dev/device.h>
//#include <mips/dev/spiio.h>
#include <machine/pic32mz.h>

/*
 * To enable debug output, uncomment the first line.
 */
//#define PRINTDBG printf
#ifndef PRINTDBG
#   define PRINTDBG(...) /*empty*/
#endif

struct spireg {
    volatile unsigned con;		/* Control */
    volatile unsigned conclr;
    volatile unsigned conset;
    volatile unsigned coninv;
    volatile unsigned stat;		/* Status */
    volatile unsigned statclr;
    volatile unsigned statset;
    volatile unsigned statinv;
    volatile unsigned buf;		/* Transmit and receive buffer */
    volatile unsigned unused1;
    volatile unsigned unused2;
    volatile unsigned unused3;
    volatile unsigned brg;		/* Baud rate generator */
    volatile unsigned brgclr;
    volatile unsigned brgset;
    volatile unsigned brginv;
};

struct spi_dev {
    struct spireg *bus;
    unsigned int *cs_tris;
    unsigned int cs_pin;
    unsigned int baud;
    unsigned int mode;
};

int spi_fd[NSPI];

#define MAXSPIDEV 10

struct spi_dev spi_devices[MAXSPIDEV];

// Default SPI bus speed

#ifndef SPI_MHZ
#define SPI_MHZ 10
#endif


// Open an SPI device in default mode.  Use further function calls to
// set baud rate, clock phase, etc.
// Returns an integer for the number of the device (ala fd).
// Returns -1 if no devices are available.

int spi_open(unsigned int bus, unsigned int *tris, unsigned int pin)
{
    int dno;

    // Find a free device
    for(dno=0; dno<MAXSPIDEV && spi_devices[dno].bus != NULL; dno++);

    // or return if not found
    if(dno == MAXSPIDEV)
        return -1;

    // Set up the device
    switch(bus)
    {
        case 1:
            spi_devices[dno].bus = (struct spireg *)&SPI1CON;
            break;
        case 2:
            spi_devices[dno].bus = (struct spireg *)&SPI2CON;
            break;
        case 3:
            spi_devices[dno].bus = (struct spireg *)&SPI3CON;
            break;
        case 4:
            spi_devices[dno].bus = (struct spireg *)&SPI4CON;
            break;
        default:
            return -1;
    }
    spi_devices[dno].cs_tris = tris;
    spi_devices[dno].cs_pin = pin;
    spi_devices[dno].baud = (CPU_KHZ / SPI_MHZ / 1000 + 1) / 2 - 1;
    spi_devices[dno].mode = PIC32_SPICON_MSTEN | PIC32_SPICON_ON;

    if(tris)
    {
        // Configure the CS pin
        LAT_SET(*tris) = 1<<pin;
        TRIS_CLR(*tris) = 1<<pin;
    }
    // return the ID of the device.
    return dno;
}

void spi_set_cspin(int dno, unsigned int *tris, unsigned int pin)
{
    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    // Revert the old CS pin to an input (release it)
    if(spi_devices[dno].cs_tris)
    {
        // Configure the CS pin
        TRIS_SET(*spi_devices[dno].cs_tris) = 1<<pin;
    }

    spi_devices[dno].cs_tris = tris;
    spi_devices[dno].cs_pin = pin;
    if(tris)
    {
        // Configure the CS pin
        LAT_SET(*tris) = 1<<pin;
        TRIS_CLR(*tris) = 1<<pin;
    }
}

// Close an SPI device
// Free up the device entry, and turn off the CS pin (set it to input)

void spi_close(int dno)
{
    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    if (spi_devices[dno].cs_tris != NULL) {
        // Revert the CS pin to input.
        TRIS_CLR(*spi_devices[dno].cs_tris) = 1<<spi_devices[dno].cs_pin;
    }
    spi_devices[dno].cs_tris = NULL;

    // Disable the device (remove the bus pointer)
    spi_devices[dno].bus = NULL;
}

// Assert the CS pin of a device.
// Not only do we set the CS pin, but before we do so we also reconfigure
// the SPI bus to the required settings for this device.
void spi_select(int dno)
{
    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    if (spi_devices[dno].cs_tris == NULL)
        return;

    spi_devices[dno].bus->brg = spi_devices[dno].baud;
    spi_devices[dno].bus->con = spi_devices[dno].mode;

    LAT_CLR(*spi_devices[dno].cs_tris) = 1<<spi_devices[dno].cs_pin;
}

// Deassert the CS pin of a device.
void spi_deselect(int dno)
{
    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    if (spi_devices[dno].cs_tris == NULL)
        return;

    LAT_SET(*spi_devices[dno].cs_tris) = 1<<spi_devices[dno].cs_pin;
}

// Set a mode setting or two - just updates the internal records, the
// actual mode is changed next time the CS is asserted
void spi_set(int dno, unsigned int set)
{
    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    spi_devices[dno].mode |= set;
}

// Clear a mode setting or two - just updates the internal records, the
// actual mode is changed next time the CS is asserted
void spi_clr(int dno, unsigned int set)
{
    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    spi_devices[dno].mode &= ~set;
}

// Return the current status of the SPI bus for the device in question
// Just returns the ->stat entry in the register set.
unsigned int spi_status(int dno)
{
    if(dno >= MAXSPIDEV)
        return 0;

    if(spi_devices[dno].bus==NULL)
        return 0;

    return spi_devices[dno].bus->stat;
}

// Transfer one word of data, and return the read word of
// data.  The actual number of bits sent depends on the
// mode of the transfer.
// This is blocking, and waits for the transfer to complete
// before returning.  Times out after a certain period.
unsigned char spi_transfer(int dno, unsigned char data)
{
    unsigned int to = 100000;

    if(dno >= MAXSPIDEV)
        return 0xF0;

    if(spi_devices[dno].bus==NULL)
        return 0xF1;

    spi_devices[dno].bus->con = spi_devices[dno].mode;
    spi_devices[dno].bus->brg = spi_devices[dno].baud;

    spi_devices[dno].bus->buf = data;
    while ((--to > 0) && (!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBF)))
        asm volatile ("nop");

    if(to  == 0)
        return 0xF2;

    return spi_devices[dno].bus->buf;
}

// Write a huge chunk of data as fast and as efficiently as
// possible.  Switches in to 32-bit mode regardless, and uses
// the enhanced buffer mode.
// Data should be a multiple of 32 bits.
void spi_bulk_write_32_be(int dno, unsigned int len, char *data)
{
    int *data32 = (int *)data;
    unsigned int words = len >> 2;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = mips_bswap(*data32++);
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            (void) spi_devices[dno].bus->buf;
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_write_32(int dno, unsigned int len, char *data)
{
    int *data32 = (int *)data;
    unsigned int words = len >> 2;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = *data32++;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            (void) spi_devices[dno].bus->buf;
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_write_16(int dno, unsigned int len, char *data)
{
    short *data16 = (short *)data;
    unsigned int words = len >> 1;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = *data16++;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            (void) spi_devices[dno].bus->buf;
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_write(int dno, unsigned int len, unsigned char *data)
{
    unsigned char *data8 = data;
    unsigned int i;
    unsigned char out;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    for(i=0; i<len; i++)
    {
        out = *data8;
        spi_transfer(dno, out);
        data8++;
    }
}

// Read a huge chunk of data as fast and as efficiently as
// possible.  Switches in to 32-bit mode regardless, and uses
// the enhanced buffer mode.
// Data should be a multiple of 32 bits.
void spi_bulk_read_32_be(int dno, unsigned int len, char *data)
{
    int *data32 = (int *)data;
    unsigned int words = len >> 2;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = ~0;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            *data32++ = mips_bswap(spi_devices[dno].bus->buf);
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_read_32(int dno, unsigned int len, char *data)
{
    int *data32 = (int *)data;
    unsigned int words = len >> 2;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = ~0;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            *data32++ = spi_devices[dno].bus->buf;
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_read_16(int dno, unsigned int len, char *data)
{
    short *data16 = (short *)data;
    unsigned int words = len >> 1;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = ~0;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            *data16++ = mips_bswap(spi_devices[dno].bus->buf);
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_read(int dno, unsigned int len, unsigned char *data)
{
    unsigned char *data8 = data;
    unsigned int i;
    unsigned char in,out;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    for(i=0; i<len; i++)
    {
        out = 0xFF;
        in = spi_transfer(dno, out);
        *data8 = in;
        data8++;
    }
}

void spi_bulk_rw_32_be(int dno, unsigned int len, char *data)
{
    int *read32 = (int *)data;
    int *write32 = (int *)data;
    unsigned int words = len >> 2;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = *write32++;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            *read32++ = mips_bswap(spi_devices[dno].bus->buf);
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_rw_32(int dno, unsigned int len, char *data)
{
    int *read32 = (int *)data;
    int *write32 = (int *)data;
    unsigned int words = len >> 2;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE32 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = *write32++;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            *read32++ = spi_devices[dno].bus->buf;
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_rw_16(int dno, unsigned int len, char *data)
{
    short *read16 = (short *)data;
    short *write16 = (short *)data;
    unsigned int words = len >> 1;
    unsigned int nread;
    unsigned int nwritten;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    nread = 0;
    nwritten = words;

    spi_devices[dno].bus->conset = PIC32_SPICON_MODE16 | PIC32_SPICON_ENHBUF;
    while(nread < words)
    {
        if(nwritten > 0 && !(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPITBF))
        {
            spi_devices[dno].bus->buf = *write16++;
            nwritten--;
        }

        if(!(spi_devices[dno].bus->stat & PIC32_SPISTAT_SPIRBE))
        {
            *read16++ = mips_bswap(spi_devices[dno].bus->buf);
            nread++;
        }
    }
    spi_devices[dno].bus->con = spi_devices[dno].mode;
}

void spi_bulk_rw(int dno, unsigned int len, unsigned char *data)
{
    unsigned char *data8 = data;
    unsigned int i;
    unsigned char in,out;

    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    for(i=0; i<len; i++)
    {
        out = *data8;
        in = spi_transfer(dno, out);
        *data8 = in;
        data8++;
    }
}

// Set the SPI baud rate for a device (in KHz)

void spi_brg(int dno, unsigned int baud)
{
    if(dno >= MAXSPIDEV)
        return;

    if(spi_devices[dno].bus==NULL)
        return;

    spi_devices[dno].baud = (CPU_KHZ / baud + 1) / 2 - 1;
}

// Return the name of the SPI bus for a device

char *spi_name(int dno)
{
    if(dno >= MAXSPIDEV)
        return "SPI?";

    if(spi_devices[dno].bus==NULL)
        return "SPI?";

    if(spi_devices[dno].bus == (struct spireg *)&SPI1CON)
        return "SPI1";

    if(spi_devices[dno].bus == (struct spireg *)&SPI2CON)
        return "SPI2";

    if(spi_devices[dno].bus == (struct spireg *)&SPI3CON)
        return "SPI3";

    if(spi_devices[dno].bus == (struct spireg *)&SPI4CON)
        return "SPI4";

    return "SPI?";
}

// Return the port name of the CS pin for a device
char spi_csname(int dno)
{
    if(dno >= MAXSPIDEV)
        return '?';

    if(spi_devices[dno].bus==NULL)
        return '?';

    switch((unsigned int)spi_devices[dno].cs_tris)
    {
        case (unsigned int)&TRISA: return 'A';
        case (unsigned int)&TRISB: return 'B';
        case (unsigned int)&TRISC: return 'C';
        case (unsigned int)&TRISD: return 'D';
        case (unsigned int)&TRISE: return 'E';
        case (unsigned int)&TRISF: return 'F';
        case (unsigned int)&TRISG: return 'G';
    }
    return '?';
}

int spi_cspin(int dno)
{
    if(dno >= MAXSPIDEV)
        return '?';

    if(spi_devices[dno].bus==NULL)
        return '?';

    return spi_devices[dno].cs_pin;
}

unsigned int spi_get_brg(int dno)
{
    if(dno >= MAXSPIDEV)
        return 0;

    if(spi_devices[dno].bus==NULL)
        return 0;

    return CPU_KHZ / (spi_devices[dno].baud + 1) / 2;
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

    spi_fd[channel] = spi_open(channel+1, 0, 0);
    if(spi_fd[channel]==-1)
        return ENODEV;
    return 0;
}

int spiclose (dev_t dev, int flag, int mode)
{
    int channel = minor (dev);

    if (channel >= NSPI)
        return ENXIO;

    if (curproc->p_ucred->cr_uid != 0)
            return EPERM;

    spi_close(spi_fd[channel]);
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
    int nelem;
    static unsigned volatile *const tris[8] = {
        0, &TRISA,&TRISB,&TRISC,&TRISD,&TRISE,&TRISF,&TRISG,
    };
    int mask, portnum;

    //PRINTDBG ("spi%d: ioctl (cmd=%08x, addr=%08x)\n", channel+1, cmd, addr);
    if (channel >= NSPI)
        return ENXIO;

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
        if((unsigned int) addr & 0x01)
            spi_set(spi_fd[channel], PIC32_SPICON_CKE);
        if((unsigned int) addr & 0x02)
            spi_set(spi_fd[channel], PIC32_SPICON_CKP);
        return 0;

    case SPICTL_SETRATE:        /* set clock rate, kHz */
        spi_brg(spi_fd[channel], (unsigned int) addr);
        return 0;

    case SPICTL_SETSELPIN:      /* set select pin */
        mask = 1 << ((unsigned int) addr & 15);
        portnum = ((unsigned int) addr >> 8) & 7;
        if (! portnum)
            return 0;
        spi_set_cspin(spi_fd[channel], (unsigned int *)tris[((unsigned int) addr >> 8) & 7], (unsigned int) addr & 15);
        return 0;

    case SPICTL_IO8(0):         /* transfer n*8 bits */
        spi_select(spi_fd[channel]);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (baduaddr (addr) || baduaddr (addr + nelem - 1))
            return EFAULT;
        spi_bulk_rw(spi_fd[channel], nelem, cval);
        spi_deselect(spi_fd[channel]);
        break;

    case SPICTL_IO16(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) ||
            baduaddr (addr) || baduaddr (addr + nelem*2 - 1))
            return EFAULT;
        spi_bulk_rw_16(spi_fd[channel], nelem<<1, (char *)addr);
        break;

    case SPICTL_IO32(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_rw_32(spi_fd[channel], nelem<<2, (char *)addr);
        break;

    case SPICTL_IO8R(0):         /* transfer n*8 bits */
        spi_select(spi_fd[channel]);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (baduaddr (addr) || baduaddr (addr + nelem - 1))
            return EFAULT;
        spi_bulk_read(spi_fd[channel], nelem, cval);
        spi_deselect(spi_fd[channel]);
        break;

    case SPICTL_IO16R(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) ||
            baduaddr (addr) || baduaddr (addr + nelem*2 - 1))
            return EFAULT;
        spi_bulk_read_16(spi_fd[channel], nelem<<1, (char *)addr);
        break;

    case SPICTL_IO32R(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_read_32(spi_fd[channel], nelem<<2, (char *)addr);
        break;

    case SPICTL_IO8W(0):         /* transfer n*8 bits */
        spi_select(spi_fd[channel]);
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (baduaddr (addr) || baduaddr (addr + nelem - 1))
            return EFAULT;
        spi_bulk_write(spi_fd[channel], nelem, cval);
        spi_deselect(spi_fd[channel]);
        break;

    case SPICTL_IO16W(0):        /* transfer n*16 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 1) ||
            baduaddr (addr) || baduaddr (addr + nelem*2 - 1))
            return EFAULT;
        spi_bulk_write_16(spi_fd[channel], nelem<<1, (char *)addr);
        break;

    case SPICTL_IO32W(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_write_32(spi_fd[channel], nelem<<2, (char *)addr);
        break;

    case SPICTL_IO32RB(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_read_32_be(spi_fd[channel], nelem<<2, (char *)addr);
        break;

    case SPICTL_IO32WB(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_write_32_be(spi_fd[channel], nelem<<2, (char *)addr);
        break;

    case SPICTL_IO32B(0):        /* transfer n*32 bits */
        nelem = (cmd >> 16) & IOCPARM_MASK;
        if (((unsigned) addr & 3) ||
            baduaddr (addr) || baduaddr (addr + nelem*4 - 1))
            return EFAULT;
        spi_bulk_write_32_be(spi_fd[channel], nelem<<2, (char *)addr);
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
