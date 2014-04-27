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
