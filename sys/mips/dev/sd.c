/*
 * SD flash card disk driver.
 */
#include "sd.h"
#if NSD > 0

#include <sys/param.h>
#include <sys/buf.h>

#include <mips/dev/device.h>

int
sdopen(dev, flags, mode, p)
	dev_t dev;
	int flags, mode;
	struct proc *p;
{
        // TODO
	return (ENXIO);
}

void
sdstrategy(bp)
	register struct buf *bp;
{
        // TODO
	bp->b_flags |= B_ERROR;
	biodone(bp);
}

int
sdioctl(dev, cmd, data, flag, p)
	dev_t dev;
	u_long cmd;
	caddr_t data;
	int flag;
	struct proc *p;
{
        // TODO
	return (EINVAL);
}

int
sdsize(dev)
	dev_t dev;
{
        // TODO
        return (-1);
}

/*
 * Non-interrupt driven, non-dma dump routine.
 */
int
sddump(dev)
	dev_t dev;
{
        // TODO
	return (ENXIO);
}

int
sdread(dev, uio)
	dev_t dev;
	struct uio *uio;
{
        // TODO
	return (EPERM);
}

int
sdwrite(dev, uio)
	dev_t dev;
	struct uio *uio;
{
        // TODO
	return (EPERM);
}

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 */
sdprobe(cp)
	register struct mips_ctlr *cp;
{
        // TODO
	return (0);
}

/*
 * Check for interrupts from all devices.
 */
void
sdintr(unit)
	register int unit;
{
        // TODO
}

struct	driver sddriver = {
	"sd", sdprobe, 0, 0, sdintr,
};
#endif
