/*
 * GPIO driver for pic32.
 *
 * Copyright (C) 2014 Serge Vakulenko, <serge@vak.ru>
 * Copyright (C) 2015 Jaume Olivé, <jolive@iberoxarxa.com>
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
#include "gpio.h"
#if NGPIO > 0
#include <machine/gpio.h>

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/spi.h>
#include <sys/systm.h>

#include <mips/dev/device.h>
#include <machine/pic32mz.h>
#include <machine/pic32_gpio.h>

/*
 * To enable debug output, uncomment the following line.
 */
//#define PRINTDBG printf
#ifndef PRINTDBG
#   define PRINTDBG(...) /*empty*/
#endif

/*
 * Devices:
 *      /dev/porta ... /dev/portk
 *      /dev/confa ... /dev/confk
 *
 * Example:
 *      echo ....oiiid....iiii > /dev/confa
 *      echo ....1...0........ > /dev/porta
 *      cat /dev/conf?
 *      cat /dev/port?
 *
 * Write to /dev/confX:
 *      'i' - configure the corresponding port pin as an input;
 *      'o' - configure the corresponding port pin as an output;
 *      'd' - configure the corresponding port pin as an open-drain output;
 *      '.' - no action.
 *
 * Write to /dev/portX:
 *      '0' - set output pin low;
 *      '1' - set output pin high;
 *      '+' - invert the value of output pin;
 *      '.' - no action.
 */

/*
 * Use ioctl() on any of devices to control pins from the user program.
 *       ioctl(fd, GPIO_PORTA | GPIO_CONFIN, mask)  - configure as input, port A
 *       ioctl(fd, GPIO_PORTB | GPIO_CONFOUT,mask)  - configure as output, port B
 *       ioctl(fd, GPIO_PORTC | GPIO_CONFOD, mask)  - configure as open drain, port C
 *       ioctl(fd, GPIO_PORTE | GPIO_STORE,  val)   - set values of all pins, port E
 *       ioctl(fd, GPIO_PORTF | GPIO_SET,    mask)  - set to 1 by mask, port F
 *       ioctl(fd, GPIO_PORTG | GPIO_CLEAR,  mask)  - set to 0 by mask, port G
 *       ioctl(fd, GPIO_PORTH | GPIO_INVERT, mask)  - invert by mask, port H
 * val = ioctl(fd, GPIO_PORTJ | GPIO_POLL,   0)     - get input values, port J
 *
 * Several operations can be combined in one call.
 * For example, to toggle pin A2 high thew low, and get value
 * of all PORTA pins:
 * val = ioctl(fd, GPIO_PORTA | GPIO_SET | GPIO_CLEAR | GPIO_POLL, 1<<3);
 *
 * You can use GPIO_PORT(0) instead of GPIO_PORTA,
 * GPIO_PORT(1) instead of GPIO_PORTB and so on.
 */
#undef NGPIO
#define NGPIO           10              /* Ports A, B, C, D, E, F, G, H, J, K */
#define NPINS           16              /* Number of pins per port */

#define MINOR_CONF      0x40            /* Minor mask: /dev/confX */
#define MINOR_UNIT      0x0f            /* Minor mask: unit number */

/*
 * Mask of configured pins, default empty.
 */
u_int gpio_confmask[NGPIO];

static void
gpio_print(dev, buf)
    dev_t dev;
    char *buf;
{
    u_int unit = minor(dev) & MINOR_UNIT;
    struct gpioreg *reg = unit + (struct gpioreg*) &ANSELA;
    u_int mask, conf, tris;
    char c;

    conf = gpio_confmask[unit];
    tris = reg->tris;
    if (minor(dev) & MINOR_CONF) {
        /* /dev/confX device: port configuration mask */
        u_int odc = reg->odc;
        for (mask=1<<(NPINS-1); mask; mask>>=1) {
            if (! (conf & mask))
                c = '-';
            else if (tris & mask)
                c = 'i';
            else
                c = (odc & mask) ? 'd' : 'o';
            *buf++ = c;
        }
    } else {
        /* /dev/portX device: port value mask */
        u_int lat = reg->lat;
        u_int port = reg->port;
        for (mask=1<<(NPINS-1); mask; mask>>=1) {
            if (! (conf & mask))
                c = '-';
            else if (tris & mask)
                c = (port & mask) ? '1' : '0';
            else
                c = (lat & mask) ? '1' : '0';
            *buf++ = c;
        }
    }
    *buf++ = '\n';
    *buf = 0;
}

static void
gpio_parse(dev, buf)
    dev_t dev;
    char *buf;
{
    u_int unit = minor(dev) & MINOR_UNIT;
    struct gpioreg *reg = unit + (struct gpioreg*) &ANSELA;
    u_int mask;
    char c;

    if (minor(dev) & MINOR_CONF) {
        /* /dev/confX device: port configuration mask */
        for (mask=1<<(NPINS-1); mask; mask>>=1) {
            c = *buf++;
            if (c <= ' ' || c > '~')
                break;
            if (! (gpio_confmask[unit] & mask))
                continue;
            if (c == 'i' || c == 'I')
                reg->trisset = mask;
            else if (c == 'o' || c == 'O') {
                reg->odcclr = mask;
                reg->trisclr = mask;
            } else if (c == 'd' || c == 'D') {
                reg->odcset = mask;
                reg->trisclr = mask;
            }
        }
    } else {
        /* /dev/portX device: port value mask */
        u_int conf = gpio_confmask[unit];
        u_int tris = reg->tris;
        for (mask=1<<(NPINS-1); mask; mask>>=1) {
            c = *buf++;
            if (c <= ' ' || c > '~')
                break;
            if (! (conf & mask) || (tris & mask))
                continue;
            if (c == '0')
                reg->latclr = mask;
            else
                reg->latset = mask;
        }
    }
}

/*
 * Open /dev/portX or /dev/confX device.
 */
int gpioopen(dev_t dev, int flag, int mode)
{
    int unit = minor(dev) & MINOR_UNIT;

    if (unit >= NGPIO)
        return ENXIO;
    if (curproc->p_ucred->cr_uid != 0)
        return EPERM;
    return 0;
}

/*
 * Close /dev/portX or /dev/confX device.
 */
int gpioclose(dev_t dev, int flag, int mode)
{
    return 0;
}

int gpioread(dev_t dev, struct uio *uio, int flag)
{
    int unit = minor(dev) & MINOR_UNIT;
    u_int cnt = NPINS + 1;
    char buf[20];

    /* I/o size should be large enough. */
    if (uio->uio_iov->iov_len < cnt)
        return EIO;

    /* Read only cnt bytes.
     * If port not configured, return empty string. */
    if (uio->uio_offset >= cnt || gpio_confmask[unit] == 0)
        return 0;
    cnt -= uio->uio_offset;

    /* Print port status to buffer. */
    gpio_print(dev, buf);
    //PRINTDBG("gpioread -> %s", buf);

    return uiomove(buf + uio->uio_offset, cnt, uio);
}

int gpiowrite(dev_t dev, struct uio *uio, int flag)
{
    u_int cnt = NPINS;
    char buf[20];
    int error;

    /* I/o size should be large enough. */
    if (uio->uio_iov->iov_len < cnt)
        return EIO;

    error = uiomove(buf, cnt, uio);
    if (error)
        return error;

    PRINTDBG("gpiowrite('%s')\n", buf);
    gpio_parse(dev, buf);
    return 0;
}

/*
 * Commands:
 * GPIO_CONFIN  - configure as input
 * GPIO_CONFOUT - configure as output
 * GPIO_CONFOD  - configure as open drain
 * GPIO_STORE   - store all outputs
 * GPIO_SET     - set to 1 by mask
 * GPIO_CLEAR   - set to 0 by mask
 * GPIO_INVERT  - invert by mask
 * GPIO_POLL    - poll
 *
 * Use GPIO_PORT(n) to set port number.
 */
int gpioioctl(dev_t dev, u_int cmd, caddr_t addr, int flag)
{
    u_int unit, mask, value;
    struct gpioreg *reg;

    PRINTDBG("gpioioctl(cmd=%08x, addr=%08x, flag=%d)\n", cmd, addr, flag);
    unit = cmd & 0xff;
    cmd &= ~0xff;

    if ((cmd & (IOC_INOUT | IOC_VOID)) != IOC_VOID ||
        ((cmd >> 8) & 0xff) != 'g')
        return EINVAL;
    if (unit >= NGPIO)
        return ENXIO;

    reg = unit + (struct gpioreg*) &ANSELA;
    mask = (u_int) addr & gpio_confmask[unit];

    if (cmd & GPIO_COMMAND & GPIO_CONFIN) {
        /* configure as input */
        PRINTDBG("TRIS%cSET %08x := %04x\n", unit+'A', &reg->trisset, mask);
        reg->trisset = mask;
    }
    if (cmd & GPIO_COMMAND & (GPIO_CONFOUT | GPIO_CONFOD)) {
        if (cmd & GPIO_COMMAND & GPIO_CONFOUT) {
            /* configure as output */
            PRINTDBG("ODC%cCLR %08x := %04x\n", unit+'A', &reg->odcclr, mask);
            reg->odcclr = mask;
        } else {
            /* configure as open drain */
            PRINTDBG("ODC%cSET %08x := %04x\n", unit+'A', &reg->odcset, mask);
            reg->odcset = mask;
        }
        PRINTDBG("TRIS%cCLR %08x := %04x\n", unit+'A', &reg->trisclr, mask);
        reg->trisclr = mask;
    }
    if (cmd & GPIO_COMMAND & GPIO_STORE) {
        /* store all outputs */
        value = reg->lat;
        PRINTDBG("LAT%c %08x -> %04x\n", unit+'A', &reg->lat, value);
        value &= ~gpio_confmask[unit];
        value |= mask;
        PRINTDBG("LAT%c %08x := %04x\n", unit+'A', &reg->lat, value);
        reg->lat = value;
    }
    if (cmd & GPIO_COMMAND & GPIO_SET) {
        /* set to 1 by mask */
        PRINTDBG("LAT%cSET %08x := %04x\n", unit+'A', &reg->latset, mask);
        reg->latset = mask;
    }
    if (cmd & GPIO_COMMAND & GPIO_CLEAR) {
        /* set to 0 by mask */
        PRINTDBG("LAT%cCLR %08x := %04x\n", unit+'A', &reg->latclr, mask);
        reg->latclr = mask;
    }
    if (cmd & GPIO_COMMAND & GPIO_INVERT) {
        /* invert by mask */
        PRINTDBG("LAT%cINV %08x := %04x\n", unit+'A', &reg->latinv, mask);
        reg->latinv = mask;
    }
    if (cmd & GPIO_COMMAND & GPIO_POLL) {
        /* send current pin values to user */
        value = reg->port & reg->tris;
        value |= reg->lat & ~reg->tris;
        PRINTDBG("PORT%c %08x -> %04x\n", unit+'A', &reg->port, value);
        curproc->p_md.md_rval[0] = value & gpio_confmask[unit];
    }
    return 0;
}

static int
gpioprobe(config)
    struct conf_device *config;
{
    int unit = config->dev_unit;
    int flags = config->dev_flags;
    char buf[20];

    if (unit < 0 || unit >= NGPIO)
        return 0;

    gpio_confmask[unit] = flags;

    gpio_print(unit | MINOR_CONF, buf);
    printf("gpio%u at port%c, pins %s", unit,
        unit + (unit<8 ? 'A' : 'B'), buf);
    return 1;
}

struct driver gpiodriver = {
    "gpio", gpioprobe,
};
#endif /* NGPIO */
