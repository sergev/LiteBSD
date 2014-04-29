/*
 * UART driver for pic32.
 */
#include <uart.h>
#if NUART > 0

#include <sys/param.h>
#include <sys/tty.h>

#include <mips/dev/device.h>

struct	tty uart_tty[NUART];

int
uartopen(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
        // TODO
	return (ENXIO);
}

int
uartclose(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
        // TODO
	return (0);
}

int
uartread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
{
        // TODO
	return (EIO);
}

int
uartwrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
{
        // TODO
	return (EIO);
}

int
uartioctl(dev, cmd, data, flag, p)
	dev_t dev;
	u_long cmd;
	caddr_t data;
	int flag;
	struct proc *p;
{
        // TODO
	return (ENOTTY);
}

/*
 * Stop output on a line.
 */
void
uartstop(tp, flag)
	register struct tty *tp;
{
        // TODO
}

/*
 * Get a char off the appropriate line via. a busy wait loop.
 */
int
uartGetc(dev)
	dev_t dev;
{
        // TODO
#if 1
        return 0;
#else
	register uart_regmap_t *regs;
	register int c, line;
	register unsigned value;
	int s;

	line = UARTLINE(dev);
	regs = (uart_regmap_t *)uart_softc[UARTUNIT(dev)].uart_pdma[line].p_addr;
	if (!regs)
		return (0);
	s = spltty();
	for (;;) {
		value = UART_READ_REG(regs, line, UART_RR0);
		if (value & UART_RR0_RX_AVAIL) {
			value = UART_READ_REG(regs, line, UART_RR1);
			UART_READ_DATA(regs, line, c);
			if (value & (UART_RR1_PARITY_ERR | UART_RR1_RX_OVERRUN |
				UART_RR1_FRAME_ERR)) {
				UART_WRITE_REG(regs, line, UART_WR0, UART_RESET_ERROR);
				UART_WRITE_REG(regs, UART_CHANNEL_A, UART_WR0,
					UART_RESET_HIGHEST_IUS);
			} else {
				UART_WRITE_REG(regs, UART_CHANNEL_A, UART_WR0,
					UART_RESET_HIGHEST_IUS);
				splx(s);
				return (unsigned char) c;
			}
		} else
			DELAY(10);
	}
#endif
}

/*
 * Send a char on a port, via a busy wait loop.
 */
void
uartPutc(dev, c)
	dev_t dev;
	int c;
{
        // TODO
#if 0
	register uart_regmap_t *regs;
	register int line;
	register u_char value;
	int s;

	s = spltty();
	line = UARTLINE(dev);
	regs = (uart_regmap_t *)uart_softc[UARTUNIT(dev)].uart_pdma[line].p_addr;

	/*
	 * Wait for transmitter to be not busy.
	 */
	do {
		UART_READ_REG(regs, line, UART_RR0, value);
		if (value & UART_RR0_TX_EMPTY)
			break;
		DELAY(100);
	} while (1);

	/*
	 * Send the char.
	 */
	UART_WRITE_DATA(regs, line, c);
	MachEmptyWriteBuffer();
	splx(s);
#endif
}

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 */
uartprobe(cp)
	register struct mips_ctlr *cp;
{
        // TODO
	return (0);
}

/*
 * Check for interrupts from all devices.
 */
void
uartintr(unit)
	register int unit;
{
        // TODO
}

struct	driver uartdriver = {
	"uart", uartprobe, 0, 0, uartintr,
};
#endif /* NUART */
