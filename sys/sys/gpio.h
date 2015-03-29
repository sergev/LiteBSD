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
#ifndef _GPIO_H
#define _GPIO_H

#include <sys/ioctl.h>

#define GPIO_CONFIN     (IOC_VOID | 1 << 16 | 'g'<<8)   /* configure as input */
#define GPIO_CONFOUT    (IOC_VOID | 1 << 17 | 'g'<<8)   /* configure as output */
#define GPIO_CONFOD     (IOC_VOID | 1 << 18 | 'g'<<8)   /* configure as open drain */
#define GPIO_DECONF     (IOC_VOID | 1 << 19 | 'g'<<8)   /* deconfigure */
#define GPIO_STORE      (IOC_VOID | 1 << 20 | 'g'<<8)   /* store all outputs */
#define GPIO_SET        (IOC_VOID | 1 << 21 | 'g'<<8)   /* set to 1 by mask */
#define GPIO_CLEAR      (IOC_VOID | 1 << 22 | 'g'<<8)   /* set to 0 by mask */
#define GPIO_INVERT     (IOC_VOID | 1 << 23 | 'g'<<8)   /* invert by mask */
#define GPIO_POLL       (IOC_VOID | 1 << 24 | 'g'<<8)   /* poll */
#define GPIO_LOL        (IOC_IN   | 1 << 25 | 'g'<<8)   /* display lol picture */

#endif
