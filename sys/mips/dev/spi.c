/*
 * SPI driver for pic32.
 */
#include <spi.h>
#if NSPI > 0

#include <sys/param.h>

#include <mips/dev/device.h>

/*
 * Test to see if device is present.
 * Return true if found and initialized ok.
 */
spiprobe(cp)
    register struct mips_ctlr *cp;
{
    // TODO
    return (0);
}

/*
* Check for interrupts from all devices.
*/
void
spiintr(unit)
    register int unit;
{
    // TODO
}

struct	driver spidriver = {
    "spi", spiprobe, 0, 0, spiintr,
};
#endif /* NSPI */
