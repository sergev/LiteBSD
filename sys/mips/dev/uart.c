/*
 * UART driver for pic32.
 */
#include <uart.h>
#if NUART > 0

#include <sys/param.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/file.h>

#include <mips/dev/device.h>
#include <machine/pic32mz.h>

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
uart_regmap_t *const uart[6] = {
    (uart_regmap_t*) &U1MODE,
    (uart_regmap_t*) &U2MODE,
    (uart_regmap_t*) &U3MODE,
    (uart_regmap_t*) &U4MODE,
    (uart_regmap_t*) &U5MODE,
    (uart_regmap_t*) &U6MODE
};

struct uart_irq {
    int er;
    int rx;
    int tx;
};

const struct uart_irq uartirq[6] = {
    { PIC32_IRQ_U1E, PIC32_IRQ_U1RX, PIC32_IRQ_U1TX },
    { PIC32_IRQ_U2E, PIC32_IRQ_U2RX, PIC32_IRQ_U2TX },
    { PIC32_IRQ_U3E, PIC32_IRQ_U3RX, PIC32_IRQ_U3TX },
    { PIC32_IRQ_U4E, PIC32_IRQ_U4RX, PIC32_IRQ_U4TX },
    { PIC32_IRQ_U5E, PIC32_IRQ_U5RX, PIC32_IRQ_U5TX },
    { PIC32_IRQ_U6E, PIC32_IRQ_U6RX, PIC32_IRQ_U6TX },
};

struct tty uart_tty[NUART];

extern dev_t cn_dev;

/*
 * Control modem signals.
 * No hardware modem signals are actually present.
 */
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
static int
uartparam(tp, t)
    struct tty *tp;
    struct termios *t;
{
    int unit = minor(tp->t_dev);
    uart_regmap_t *reg = uart[unit];
    unsigned rxirq = uartirq[unit].rx;
    unsigned cflag = t->c_cflag;
    unsigned mode;

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

    /* Reset line. */
    reg->mode = 0;
    reg->staset = 0;
    DELAY(25);

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

    /* Setup the line. */
    reg->sta = 0;
    reg->brg = PIC32_BRG_BAUD (CPU_KHZ * 1000, tp->t_ospeed);
    reg->mode = mode;
    reg->staset = PIC32_USTA_URXEN | PIC32_USTA_UTXEN;

    /* Enable receive interrupt. */
    IECSET(rxirq >> 5) = 1 << (rxirq & 31);
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
    uart_regmap_t *reg = uart[unit];
    unsigned txirq = uartirq[unit].tx;
    int c, s;

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

    if (reg->sta & PIC32_USTA_TRMT) {
        /* Send the first char. */
        c = getc(&tp->t_outq);
        reg->txreg = (unsigned char) c;
        tp->t_state |= TS_BUSY;
    }

    /* Enable transmit interrupt. */
    IECSET(txirq >> 5) = 1 << (txirq & 31);
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
            tp->t_ispeed = TTYDEF_SPEED;
            tp->t_ospeed = TTYDEF_SPEED;
        }
        uartparam(tp, &tp->t_termios);
        ttsetwater(tp);

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
    uart_regmap_t *reg = uart[unit];

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
    uart_regmap_t *reg = uart[unit];
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
    int unit = minor(tp->t_dev);
    uart_regmap_t *reg = uart[unit];
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
    uart_regmap_t *reg = uart[unit];
    unsigned rxirq = uartirq[unit].rx;
    unsigned txirq = uartirq[unit].tx;
    unsigned erirq = uartirq[unit].er;
    int c;

    /* Receive */
    while (reg->sta & PIC32_USTA_URXDA) {
        c = reg->rxreg;
        ttyinput(c, tp);
    }
    if (reg->sta & PIC32_USTA_OERR) {
        reg->staclr = PIC32_USTA_OERR;
    }
    /* Clear RX interrupt. */
    IFSCLR(rxirq >> 5) = 1 << (rxirq & 31) | 1 << (erirq & 31);

    /* Transmit */
    if (reg->sta & PIC32_USTA_TRMT) {
        /* Clear TX interrupt. */
        IECCLR(txirq >> 5) = 1 << (txirq & 31);
        IFSCLR(txirq >> 5) = 1 << (txirq & 31);

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
uartGetc(dev)
    dev_t dev;
{
    int unit = minor(dev);
    uart_regmap_t *reg = uart[unit];
    unsigned rxirq = uartirq[unit].rx;
    unsigned erirq = uartirq[unit].er;
    int s, c;

    s = spltty();
    for (;;) {
        /* Wait for input char. */
        if (reg->sta & PIC32_USTA_URXDA) {
            c = (unsigned char) reg->rxreg;
            break;
        }
    }

    IFSCLR(rxirq >> 5) = 1 << (rxirq & 31) | 1 << (erirq & 31);
    splx(s);
    return c;
}

/*
 * Send a char on a port, via a busy wait loop.
 */
void
uartPutc(dev, c)
    dev_t dev;
    int c;
{
    int unit = minor(dev);
    struct tty *tp = &uart_tty[unit];
    uart_regmap_t *reg = uart[unit];
    unsigned txirq = uartirq[unit].tx;
    int timo, s;

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
    if (tp->t_state & TS_BUSY) {
            uartintr (dev);
            goto again;
    }
    reg->txreg = (unsigned char) c;

    timo = 30000;
    while (! (reg->sta & PIC32_USTA_TRMT))
        if (--timo == 0)
            break;

    /* Clear TX interrupt. */
    IECCLR(txirq >> 5) = 1 << (txirq & 31);
    splx(s);
}

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 */
uartprobe(cp)
    struct mips_ctlr *cp;
{
    uart_regmap_t *reg;
    struct tty *tp;
    int unit = cp->mips_unit;

    if (unit >= NUART)
        return 0;
    reg = uart[unit];

    tp = &uart_tty[unit];
    tp->t_dev = unit;

    /* Reset chip. */
    reg->brg = 0;
    reg->sta = 0;
    reg->mode = PIC32_UMODE_PDSEL_8NPAR | PIC32_UMODE_ON;
    reg->staset = 0;

    /*
     * Special handling for consoles.
     */
    if (minor(cn_dev) == unit) {
        struct tty ctty;
        struct termios cterm;
        int s;

        s = spltty();
        ctty.t_dev = cn_dev;
        cterm.c_cflag = CS8;
        cterm.c_ospeed = cterm.c_ispeed = TTYDEF_SPEED;
        uartparam(&ctty, &cterm);
        DELAY(1000);
        splx(s);
    }
    printf("uart%d at address 0x%x irq %u,%u,%u\n",
        unit, reg, uartirq[unit].er, uartirq[unit].rx, uartirq[unit].tx);
    return 1;
}

struct  driver uartdriver = {
    "uart", uartprobe, 0, 0, uartintr,
};
#endif /* NUART */
