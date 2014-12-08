/*
 * PIC32 SPI control registers.
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

/*
 * Data structure for SPI support library.
 */
struct spiio {
    struct spireg   *reg;
    int cs;
    unsigned int    divisor;
    unsigned int    mode;
};

/*
 * Setup SPI connection on a given port (0..5) with a specified chip select pin.
 * Use default speed.
 * Return 0 if the SPI port is not configured.
 */
int spi_setup(struct spiio *io, int port, int pin);

/*
 * Set the SPI bit rate for a device (in kHz).
 */
void spi_set_speed(struct spiio *io, unsigned int khz);

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
void spi_set_cspin(struct spiio *io, int pin);

/*
 * Assert the CS pin of a device.
 */
void spi_select(struct spiio *io);

/*
 * Deassert the CS pin of a device.
 */
void spi_deselect(struct spiio *io);

/*
 * Set a mode setting or two - just updates the internal records,
 * the actual mode is changed next time the CS is asserted.
 */
void spi_set(struct spiio *io, unsigned int set);

/*
 * Clear a mode setting or two - just updates the internal records,
 * the actual mode is changed next time the CS is asserted.
 */
void spi_clr(struct spiio *io, unsigned set);

/*
 * Return the current status of the SPI bus for the device in question.
 * Just returns the ->stat entry in the register set.
 */
unsigned int spi_status(struct spiio *io);

/*
 * Return the name of the SPI bus for a device.
 */
const char *spi_name(struct spiio *io);

/*
 * Return the port name (A-K) of the chip select pin for a device.
 */
char spi_csname(struct spiio *io);

/*
 * Return the pin index of the chip select pin for a device.
 */
int spi_cspin(struct spiio *io);

/*
 * Return the speed in kHz.
 */
unsigned int spi_get_speed(struct spiio *io);

/*
 * Transfer one word of data, and return the read word of data.
 * The actual number of bits sent depends on the mode of the transfer.
 * This is blocking, and waits for the transfer to complete
 * before returning.  Times out after a certain period.
 */
unsigned spi_transfer(struct spiio *io, unsigned data);

/*
 * Transmit a chunk of 8-bit data.
 */
void spi_bulk_write(struct spiio *io, unsigned int nbytes, unsigned char *data);
void spi_bulk_read(struct spiio *io, unsigned int nbytes, unsigned char *data);
void spi_bulk_rw(struct spiio *io, unsigned int nbytes, unsigned char *data);

/*
 * Transmit a chunk of 16-bit data.
 */
void spi_bulk_write16(struct spiio *io, unsigned int nelem, short *data);
void spi_bulk_read16(struct spiio *io, unsigned int nelem, short *data);
void spi_bulk_rw16(struct spiio *io, unsigned int nelem, short *data);

/*
 * Transmit a chunk of 32-bit data.
 */
void spi_bulk_write32(struct spiio *io, unsigned int nelem, int *data);
void spi_bulk_read32(struct spiio *io, unsigned int nelem, int *data);
void spi_bulk_rw32(struct spiio *io, unsigned int nelem, int *data);

/*
 * Transmit a chunk of 32-bit data with reversed endianness.
 */
void spi_bulk_write32_be(struct spiio *io, unsigned int nelem, int *data);
void spi_bulk_read32_be(struct spiio *io, unsigned int nelem, int *data);
void spi_bulk_rw32_be(struct spiio *io, unsigned int nelem, int *data);
