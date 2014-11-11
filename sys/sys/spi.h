/*
 * Ioctl definitions for SPI driver.
 *
 * Author: Serge Vakulenko
 * (C) 2012 RetroBSD project
 */
#ifndef _SPI_H
#define _SPI_H

#include <sys/ioctl.h>

#define SPICTL_SETMODE      _IO ('p', 0)        /* set SPI mode */
#define SPICTL_SETRATE      _IO ('p', 1)        /* set clock rate, kHz */
#define SPICTL_SETSELPIN    _IO ('p', 2)        /* set select pin */

#define SPICTL_IO8(n)       _ION('p', 3, n)     /* transfer n*8 bits */
#define SPICTL_IO16(n)      _ION('p', 4, n)     /* transfer n*16 bits */
#define SPICTL_IO32(n)      _ION('p', 5, n)     /* transfer n*32 bits */

#define SPICTL_IO8R(n)      _ION('p', 6, n)     /* read n*8 bits */
#define SPICTL_IO16R(n)     _ION('p', 7, n)     /* read n*16 bits */
#define SPICTL_IO32R(n)     _ION('p', 8, n)     /* read n*32 bits */

#define SPICTL_IO8W(n)      _ION('p', 9, n)     /* write n*8 bits */
#define SPICTL_IO16W(n)     _ION('p', 10, n)    /* write n*16 bits */
#define SPICTL_IO32W(n)     _ION('p', 11, n)    /* write n*32 bits */

#define SPICTL_IO32B(n)     _ION('p', 12, n)    /* transfer n*32 bits big endian */
#define SPICTL_IO32RB(n)    _ION('p', 13, n)    /* read n*32 bits big endian */
#define SPICTL_IO32WB(n)    _ION('p', 14, n)    /* write n*32 bits big endian */

#endif
