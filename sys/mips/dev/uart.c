/*
 * UART driver for pic32.
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
#include <uart.h>
#if NUART > 0

#include <sys/param.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/systm.h>

#include <mips/dev/device.h>
#include <machine/pic32mz.h>
#include <machine/pic32_gpio.h>

#ifndef UART_BUFSZ
#define UART_BUFSZ  256
#endif

#ifndef UART_BAUD
#define UART_BAUD   TTYDEF_SPEED
#endif

/*
 * PIC32 UART registers.
 */
typedef struct {
    volatile unsigned mode;		/* Mode */
    volatile unsigned modeclr;
    volatile unsigned modeset;
    volatile unsigned modeinv;
    volatile unsigned sta;		/* Status and control */
    volatile unsigned staclr;
    volatile unsigned staset;
    volatile unsigned stainv;
    volatile unsigned txreg;            /* Transmit */
    volatile unsigned unused1;
    volatile unsigned unused2;
    volatile unsigned unused3;
    volatile unsigned rxreg;            /* Receive */
    volatile unsigned unused4;
    volatile unsigned unused5;
    volatile unsigned unused6;
    volatile unsigned brg;		/* Baud rate */
    volatile unsigned brgclr;
    volatile unsigned brgset;
    volatile unsigned brginv;
} uart_regmap_t;

#if NUART > 6
#error Max 6 UARTs supported.
#endif
static uart_regmap_t *const uart_base[6] = {
    (uart_regmap_t*) &U1MODE,
    (uart_regmap_t*) &U2MODE,
    (uart_regmap_t*) &U3MODE,
    (uart_regmap_t*) &U4MODE,
    (uart_regmap_t*) &U5MODE,
    (uart_regmap_t*) &U6MODE,
};

struct uart_irq {
    int er;                             /* Receive error interrupt number */
    int rx;                             /* Receive interrupt number */
    int tx;                             /* Transmit interrupt number */
    unsigned er_mask;                   /* Receive error irq bitmask */
    unsigned rx_mask;                   /* Receive irq bitmask */
    unsigned tx_mask;                   /* Transmit irq bitmask */
    volatile unsigned *enable_rx_intr;  /* IECSET pointer for receive */
    volatile unsigned *enable_tx_intr;  /* IECSET pointer for transmit */
    volatile unsigned *disable_tx_intr; /* IECCLR pointer for transmit */
    volatile unsigned *clear_er_intr;   /* IFSCLR pointer for receive error */
    volatile unsigned *clear_rx_intr;   /* IFSCLR pointer for receive */
    volatile unsigned *clear_tx_intr;   /* IFSCLR pointer for transmit */
};

#define UART_IRQ_INIT(name) { \
        name##E,  \
        name##RX, \
        name##TX, \
        1 << (name##E  & 31), \
        1 << (name##RX & 31), \
        1 << (name##TX & 31), \
        &IECSET(name##RX >> 5), \
        &IECSET(name##TX >> 5), \
        &IECCLR(name##TX >> 5), \
        &IFSCLR(name##E  >> 5), \
        &IFSCLR(name##RX >> 5), \
        &IFSCLR(name##TX >> 5), \
    }

static const struct uart_irq uartirq[6] = {
    UART_IRQ_INIT(PIC32_IRQ_U1),
    UART_IRQ_INIT(PIC32_IRQ_U2),
    UART_IRQ_INIT(PIC32_IRQ_U3),
    UART_IRQ_INIT(PIC32_IRQ_U4),
    UART_IRQ_INIT(PIC32_IRQ_U5),
    UART_IRQ_INIT(PIC32_IRQ_U6),
};

struct tty uart_tty[NUART];
int uart_cnt = NUART;           /* Needed for pstat */

static const char pin_name[16] = "?ABCDEFGHJK?????";

/* Convert port name/signal into a pin number. */
#define RP(x,n) (((x)-'A'+1) << 4 | (n))

/*
 * Control modem signals.
 * No hardware modem signals are actually present.
 */
int
uartmctl(dev, bits, how)
    dev_t dev;
    int bits, how;
{
    int unit = minor(dev);
    int mbits, s;

    s = spltty();

    /* Pretend DTR, DSR and DCD are always active. */
    mbits = TIOCM_DTR | TIOCM_DSR | TIOCM_CAR;
    switch (how) {
    case DMSET:
        mbits = bits;
        break;
    case DMBIS:
        mbits |= bits;
        break;
    case DMBIC:
        mbits &= ~bits;
        break;
    case DMGET:
        splx(s);
        return mbits;
    }
    if (mbits & TIOCM_DTR)
        uart_tty[unit].t_state |= TS_CARR_ON;
    splx(s);
    return mbits;
}

/*
 * Set parameters of tty device, as specified by termios data structure.
 */
int
uartparam(tp, t)
    struct tty *tp;
    struct termios *t;
{
    int unit = minor(tp->t_dev);
    uart_regmap_t *reg = uart_base[unit];
    const struct uart_irq *irq = &uartirq[unit];
    unsigned cflag = t->c_cflag;
    unsigned mode, divisor, sta, s, timo;

    /* Check whether the port is configured. */
    if (! reg) {
        if (tp->t_dev != cn_dev)
            return ENXIO;
        reg = uart_base[unit];
    }

    /* Receive speed must be equal to transmit speed. */
    if (t->c_ispeed && t->c_ispeed != t->c_ospeed) {
        return EINVAL;
    }

    /* Only 8-bit data supported. */
    if ((cflag & CSIZE) != CS8) {
        return EINVAL;
    }

    /* Copy parameters to tty */
    tp->t_ispeed = t->c_ispeed;
    tp->t_ospeed = t->c_ospeed;
    tp->t_cflag = cflag;
    if (tp->t_ospeed <= 0) {
        uartmctl(tp->t_dev, 0, DMSET);	/* hang up line */
        return 0;
    }

    /* Compute mode bits. */
    mode = PIC32_UMODE_ON;              /* UART Enable */
    if (cflag & CSTOPB)
        mode |= PIC32_UMODE_STSEL;      /* 2 Stop bits */

    if (cflag & PARENB) {
        if (cflag & PARODD)
            mode |= PIC32_UMODE_PDSEL_8ODD;	/* 8-bit data, odd parity */
        else
            mode |= PIC32_UMODE_PDSEL_8EVEN;	/* 8-bit data, even parity */
    }

    /* Compute baud rate divisor. */
    divisor = PIC32_BRG_BAUD (CPU_KHZ * 500, tp->t_ospeed);

    /* Modify setting only when there is any change. */
    sta = PIC32_USTA_UTXISEL_EMP | PIC32_USTA_URXEN | PIC32_USTA_UTXEN;
    s = spltty();
    if (reg->sta != sta || reg->mode != mode || reg->brg != divisor) {
        /* Wait until transmit buffer empty. */
        timo = 30000;
        while (! (reg->sta & PIC32_USTA_TRMT))
            if (--timo == 0)
                break;

        /* Reset line. */
        reg->sta = PIC32_USTA_UTXISEL_EMP;  /* TX interrupt when buffer empty */
        reg->mode = 0;
        udelay(25);

        /* Setup the line. */
        reg->brg = divisor;
        reg->mode = mode;
        reg->sta = sta;

        /* Resume pending trasmit. */
        if (tp->t_state & TS_BUSY)
            uartintr(tp->t_dev);
    }

    /* Enable receive interrupt. */
    *irq->enable_rx_intr = irq->rx_mask;
    splx(s);
    return 0;
}

/*
 * Start the transmitter, send one char.
 */
static void
uartstart(tp)
    struct tty *tp;
{
    int unit = minor(tp->t_dev);
    uart_regmap_t *reg = uart_base[unit];
    const struct uart_irq *irq = &uartirq[unit];
    int c, s, x;

    s = spltty();
    if (tp->t_state & (TS_TIMEOUT|TS_BUSY|TS_TTSTOP))
        goto out;

    if (tp->t_outq.c_cc <= tp->t_lowat) {
        if (tp->t_state & TS_ASLEEP) {
            tp->t_state &= ~TS_ASLEEP;
            wakeup((caddr_t)&tp->t_outq);
        }
        selwakeup(&tp->t_wsel);
    }
    if (tp->t_outq.c_cc == 0)
        goto out;

    /* Fill transmit FIFO: put as much data as we can. */
    while (! (reg->sta & PIC32_USTA_UTXBF)) {
        /* Send next char. */
        c = getc(&tp->t_outq);

        /* Need to clear tx interrupt flag immediately
         * after writing a data byte.
         * Disable interrupts to avoid timer interrupt
         * in between, otherwise it could cause
         * the interrupt loss on simulator. */
        x = splhigh();
        reg->txreg = (unsigned char) c;
        *irq->clear_tx_intr = irq->tx_mask;
        splx(x);

        tp->t_state |= TS_BUSY;
        if (tp->t_outq.c_cc == 0)
            break;
    }

    /* Enable transmit interrupt. */
    *irq->enable_tx_intr = irq->tx_mask;
out:
    splx(s);
}

/*
 * Open the device.
 */
int
uartopen(dev, flag, mode, p)
    dev_t dev;
    int flag, mode;
    struct proc *p;
{
    int unit = minor(dev);
    struct tty *tp = &uart_tty[unit];
    int s, error = 0;

    if (unit >= NUART)
        return ENXIO;

    if (!tp->t_rawq.c_cs)
        clalloc(&tp->t_rawq, UART_BUFSZ, 1);
    if (!tp->t_canq.c_cs)
        clalloc(&tp->t_canq, UART_BUFSZ, 1);
    /* output queue doesn't need quoting */
    if (!tp->t_outq.c_cs)
        clalloc(&tp->t_outq, UART_BUFSZ, 0);

    tp->t_oproc = uartstart;
    tp->t_param = uartparam;
    tp->t_dev = dev;
    if (! (tp->t_state & TS_ISOPEN)) {
        tp->t_state |= TS_WOPEN;
        ttychars(tp);
        if (tp->t_ispeed == 0) {
            tp->t_iflag = TTYDEF_IFLAG;
            tp->t_oflag = TTYDEF_OFLAG;
            tp->t_cflag = TTYDEF_CFLAG;
            tp->t_lflag = TTYDEF_LFLAG;
            tp->t_ispeed = UART_BAUD;
            tp->t_ospeed = UART_BAUD;
        }
        uartparam(tp, &tp->t_termios);

        /* Set watermarks. */
        tp->t_hiwat = UART_BUFSZ - 1;
        tp->t_lowat = UART_BUFSZ / 8;

    } else if ((tp->t_state & TS_XCLUDE) && curproc->p_ucred->cr_uid != 0) {
        return EBUSY;
    }

    /* Activate DTR modem signal. */
    uartmctl(dev, TIOCM_DTR, DMSET);

    s = spltty();
    while (! (flag & O_NONBLOCK) && ! (tp->t_cflag & CLOCAL) &&
       ! (tp->t_state & TS_CARR_ON)) {
        tp->t_state |= TS_WOPEN;
        error = ttysleep(tp, (caddr_t)&tp->t_rawq, TTIPRI | PCATCH, ttopen, 0);
        if (error)
            break;
    }
    splx(s);
    if (error)
        return error;
    return (*linesw[tp->t_line].l_open)(dev, tp);
}

/*
 * Close the device.
 */
int
uartclose(dev, flag, mode, p)
    dev_t dev;
    int flag, mode;
    struct proc *p;
{
    int unit = minor(dev);
    struct tty *tp = &uart_tty[unit];
    uart_regmap_t *reg = uart_base[unit];

    if (reg->sta & PIC32_USTA_UTXBRK) {
        /* Stop sending break. */
        reg->staclr = PIC32_USTA_UTXBRK;
        ttyoutput(0, tp);
    }
    (*linesw[tp->t_line].l_close)(tp, flag);

    if ((tp->t_cflag & HUPCL) || (tp->t_state & TS_WOPEN) ||
        /* Deactivate DTR and RTS modem signals. */
        ! (tp->t_state & TS_ISOPEN)) {
        uartmctl(dev, 0, DMSET);
    }
    return ttyclose(tp);
}

/*
 * Read data from device.
 */
int
uartread(dev, uio, flag)
    dev_t dev;
    struct uio *uio;
{
    struct tty *tp = &uart_tty[minor(dev)];

    return (*linesw[tp->t_line].l_read)(tp, uio, flag);
}

/*
 * Write data to device.
 */
int
uartwrite(dev, uio, flag)
    dev_t dev;
    struct uio *uio;
{
    struct tty *tp = &uart_tty[minor(dev)];

    return (*linesw[tp->t_line].l_write)(tp, uio, flag);
}

/*
 * Handle ioctl() call.
 */
int
uartioctl(dev, cmd, data, flag, p)
    dev_t dev;
    u_long cmd;
    caddr_t data;
    int flag;
    struct proc *p;
{
    int unit = minor(dev);
    struct tty *tp = &uart_tty[unit];
    uart_regmap_t *reg = uart_base[unit];
    int error;

    error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag, p);
    if (error >= 0)
            return error;
    error = ttioctl(tp, cmd, data, flag);
    if (error >= 0)
            return error;

    switch (cmd) {
    case TIOCSBRK:
        reg->staset = PIC32_USTA_UTXBRK;
        ttyoutput(0, tp);
        break;
    case TIOCCBRK:
        reg->staclr = PIC32_USTA_UTXBRK;
        ttyoutput(0, tp);
        break;
    case TIOCSDTR:
        uartmctl(dev, TIOCM_DTR|TIOCM_RTS, DMBIS);
        break;
    case TIOCCDTR:
        uartmctl(dev, TIOCM_DTR|TIOCM_RTS, DMBIC);
        break;
    case TIOCMSET:
        uartmctl(dev, *(int*)data, DMSET);
        break;
    case TIOCMBIS:
        uartmctl(dev, *(int*)data, DMBIS);
        break;
    case TIOCMBIC:
        uartmctl(dev, *(int*)data, DMBIC);
        break;
    case TIOCMGET:
        *(int*)data = uartmctl(dev, 0, DMGET);
        break;
    default:
        return ENOTTY;
    }
    return 0;
}

/*
 * Stop output on a line.
 */
void
uartstop(tp, flag)
    struct tty *tp;
{
    int s;

    s = spltty();
    if (tp->t_state & TS_BUSY) {
        /* Stop transmit. */
        if (! (tp->t_state & TS_TTSTOP))
            tp->t_state |= TS_FLUSH;
    }
    splx(s);
}

/*
 * Check for interrupts from the device.
 */
void
uartintr(dev)
    dev_t dev;
{
    int unit = minor(dev);
    struct tty *tp = &uart_tty[unit];
    uart_regmap_t *reg = uart_base[unit];
    const struct uart_irq *irq = &uartirq[unit];
    int c;

    /* Receive */
    while (reg->sta & PIC32_USTA_URXDA) {
        c = reg->rxreg;
        (*linesw[tp->t_line].l_rint)(c, tp);
//        ttyinput(c, tp);
    }
    if (reg->sta & PIC32_USTA_OERR) {
        reg->staclr = PIC32_USTA_OERR;
    }
    /* Clear RX interrupt. */
    *irq->clear_rx_intr = irq->rx_mask;
    *irq->clear_er_intr = irq->er_mask;

    /* Transmit */
    if (! (reg->sta & PIC32_USTA_UTXBF)) {
        /* Clear and disable TX interrupt. */
        *irq->disable_tx_intr = irq->tx_mask;
        *irq->clear_tx_intr = irq->tx_mask;

        if (tp->t_state & TS_BUSY) {
            tp->t_state &= ~TS_BUSY;
            ttstart(tp);
        }
    }
}

/*
 * Get a char off the appropriate line via. a busy wait loop.
 */
int
uart_getc(dev)
    dev_t dev;
{
    int unit = minor(dev);
    uart_regmap_t *reg = uart_base[unit];
    const struct uart_irq *irq = &uartirq[unit];
    int s, c;

    /* Check whether the port is configured. */
    if (! reg) {
        if (dev != cn_dev)
            return -1;
        reg = uart_base[unit];
    }

    s = spltty();
    for (;;) {
        /* Wait for input char. */
        if (reg->sta & PIC32_USTA_URXDA) {
            c = (unsigned char) reg->rxreg;
            break;
        }
    }

    *irq->clear_rx_intr = irq->rx_mask;
    *irq->clear_er_intr = irq->er_mask;
    splx(s);
    return c;
}

/*
 * Send a char on a port, via a busy wait loop.
 */
void
uart_putc(dev, c)
    dev_t dev;
    int c;
{
    int unit = minor(dev);
    struct tty *tp = &uart_tty[unit];
    uart_regmap_t *reg = uart_base[unit];
    const struct uart_irq *irq = &uartirq[unit];
    int timo, s;

    /* Check whether the port is configured. */
    if (! reg) {
        if (dev != cn_dev)
            return;
        tp = 0;
        reg = uart_base[unit];
    }

    s = spltty();
again:
    /*
     * Wait for transmitter to be not busy.
     * Give up after a reasonable time.
     */
    timo = 30000;
    while (! (reg->sta & PIC32_USTA_TRMT))
        if (--timo == 0)
            break;
    if (tp && tp->t_state & TS_BUSY) {
        uartintr (dev);
        goto again;
    }
    reg->txreg = (unsigned char) c;

    /* Clear TX interrupt. */
    *irq->clear_tx_intr = irq->tx_mask;

    timo = 30000;
    while (! (reg->sta & PIC32_USTA_TRMT))
        if (--timo == 0)
            break;
    splx(s);
}

/*
 * Assign UxRX signal to specified pin.
 */
static void assign_rx(int channel, int pin)
{
    switch (channel) {
    case 0: U1RXR = gpio_input_map1(pin); break;
    case 1: U2RXR = gpio_input_map3(pin); break;
    case 2: U3RXR = gpio_input_map2(pin); break;
    case 3: U4RXR = gpio_input_map4(pin); break;
    case 4: U5RXR = gpio_input_map1(pin); break;
    case 5: U6RXR = gpio_input_map4(pin); break;
    }
}

static int output_map1 (unsigned channel)
{
    switch (channel) {
    case 2: return 1;   // 0001 = U3TX
    }
    printf ("uart%u: cannot map TX pin, group 1\n", channel);
    return 0;
}

static int output_map2 (unsigned channel)
{
    switch (channel) {
    case 0: return 1;   // 0001 = U1TX
    case 4: return 3;   // 0011 = U5TX
    }
    printf ("uart%u: cannot map TX pin, group 2\n", channel);
    return 0;
}

static int output_map3 (unsigned channel)
{
    switch (channel) {
    case 3: return 2;   // 0010 = U4TX
    case 5: return 4;   // 0100 = U6TX
    }
    printf ("uart%u: cannot map TX pin, group 3\n", channel);
    return 0;
}

static int output_map4 (unsigned channel)
{
    switch (channel) {
    case 1: return 2;   // 0010 = U2TX
    case 5: return 4;   // 0100 = U6TX
    }
    printf ("uart%u: cannot map TX pin, group 4\n", channel);
    return 0;
}

/*
 * Assign UxTX signal to specified pin.
 */
static void assign_tx(int channel, int pin)
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
    printf ("uart%u: cannot map TX pin R%c%d\n",
        channel, pin_name[pin>>4], pin & 15);
}

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 */
int
uartprobe(config)
    struct conf_device *config;
{
    uart_regmap_t *reg;
    struct tty *tp;
    int unit = config->dev_unit - 1;
    int rx = config->dev_pins[0];
    int tx = config->dev_pins[1];
    int is_console = (CONS_MAJOR == 17 && CONS_MINOR == unit);

    if (unit < 0 || unit >= NUART)
        return 0;
    reg = uart_base[unit];

    tp = &uart_tty[unit];
    tp->t_dev = unit;
//    tp->t_sc = reg;

    printf("uart%d at pins rx=R%c%d/tx=R%c%d, interrupts %u/%u/%u",
        unit+1, pin_name[rx>>4], rx & 15, pin_name[tx>>4], tx & 15,
        uartirq[unit].er, uartirq[unit].rx, uartirq[unit].tx);
    if (is_console)
        printf(", console");
    printf("\n");

    if (! is_console) {
        /* Reset chip. */
        reg->sta = 0;
        reg->brg = 0;
        reg->mode = PIC32_UMODE_PDSEL_8NPAR | PIC32_UMODE_ON;

        /* Assign RX and TX pins - except console.
         * Console pins must be assigned by bootloader,
         * or in mach_init(). */
        assign_rx (unit, rx);
        assign_tx (unit, tx);
    }
    return 1;
}

struct driver uartdriver = {
    "uart", uartprobe,
};
#endif /* NUART */
