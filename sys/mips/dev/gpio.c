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
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/spi.h>
#include <sys/systm.h>

#include <mips/dev/device.h>
#include <machine/pic32mz.h>
#include <machine/pic32_gpio.h>

/*
 * To enable debug output, uncomment the first line.
 */
//#define PRINTDBG printf
#ifndef PRINTDBG
#   define PRINTDBG(...) /*empty*/
#endif

static const char pin_name[16] = "?ABCDEFGHJK?????";

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
    printf ("gpio: cannot map peripheral input pin %c%d, group 1\n",
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
    printf ("gpio: cannot map peripheral input pin %c%d, group 2\n",
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
    printf ("gpio: cannot map peripheral input pin %c%d, group 3\n",
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
    printf ("gpio: cannot map peripheral input pin %c%d, group 4\n",
        pin_name[pin>>4], pin & 15);
    return -1;
}

void gpio_set_input(int pin) {
    struct gpioreg *port = (struct gpioreg*) &ANSELA;
    port += (pin >> 4 & 15) - 1;
    port->trisset = (1 << (pin & 15));
    port->anselclr = (1 << (pin & 15));
}

void gpio_set_output(int pin) {
    struct gpioreg *port = (struct gpioreg*) &ANSELA;
    port += (pin >> 4 & 15) - 1;
    port->trisclr = (1 << (pin & 15));
    port->anselclr = (1 << (pin & 15));
}

void gpio_set_analog(int pin) {
    struct gpioreg *port = (struct gpioreg*) &ANSELA;
    port += (pin >> 4 & 15) - 1;
    port->trisset = (1 << (pin & 15));
    port->anselset = (1 << (pin & 15));
}

void gpio_set(int pin) {
    struct gpioreg *port = (struct gpioreg*) &ANSELA;
    port += (pin >> 4 & 15) - 1;
    port->latset = (1 << (pin & 15));
}

void gpio_clr(int pin) {
    struct gpioreg *port = (struct gpioreg*) &ANSELA;
    port += (pin >> 4 & 15) - 1;
    port->latclr = (1 << (pin & 15));
}

int gpio_get(int pin) {
    struct gpioreg *port = (struct gpioreg*) &ANSELA;
    port += (pin >> 4 & 15) - 1;
    return ((port->port & (1 << (pin & 15))) ? 1 : 0);
}

char gpio_portname(int pin) {
    return pin_name[(pin >> 4) & 15];
}

int gpio_pinno(int pin) {
    return pin & 15;
}

#include "gpio.h"
#if NGPIO > 0
#include <sys/gpio.h>

static unsigned char gpio_pin[4] = {0,0,0,0};

/*
 * Open /dev/gpio# device.
 */
int gpioopen (dev_t dev, int flag, int mode)
{
    unsigned char channel = minor(dev);

    if (channel >= NGPIO)
        return ENXIO;

    if (curproc->p_ucred->cr_uid != 0)
        return EPERM;

    return 0;
}

/*
 * Close /dev/gpio# device.
 */
int gpioclose (dev_t dev, int flag, int mode)
{
    unsigned char channel = minor(dev);

    if (channel >= NGPIO)
        return ENXIO;

    if (curproc->p_ucred->cr_uid != 0)
        return EPERM;

    return 0;
}

int gpioread (dev_t dev, struct uio *uio, int flag)
{
    return 0;
}

int gpiowrite (dev_t dev, struct uio *uio, int flag)
{
    return 0;
}

int gpioioctl (dev_t dev, u_int cmd, caddr_t addr, int flag)
{
    int channel = minor (dev);

    if (channel >= NGPIO)
        return ENXIO;

    switch (cmd) {
    case GPIO_CONFOUT:
        gpio_set_output(gpio_pin[channel]);
        break;

    case GPIO_CONFIN:
        gpio_set_input(gpio_pin[channel]);
        break;

    case GPIO_SET:
        gpio_set(gpio_pin[channel]);
        break;

    case GPIO_CLEAR:
        gpio_clr(gpio_pin[channel]);
        break;
    }
    return 0;
}

static int
gpioprobe(config)
    struct mips_ctlr *config;
{
    unsigned char channel = config->mips_unit - 1;
    if (channel < 0 || channel >= NGPIO)
        return 0;

    unsigned char flags = config->mips_flags;
    gpio_pin[channel] = flags;

    printf ("gpio%u at pin %c%d\n", channel+1,
        gpio_portname(flags), gpio_pinno(flags));
    return 1;
}

struct driver gpiodriver = {
    "gpio", gpioprobe,
};
#endif /* NGPIO */
