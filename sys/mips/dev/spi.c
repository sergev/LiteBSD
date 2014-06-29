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
static int
spiprobe(cp)
    struct scsi_device *config;
{
    int unit = config->sd_unit - 1;

    if (unit < 0 || unit >= NSPI)
        return 0;

    // TODO

    printf ("spi%u: sdi/sdo pins %c%d, %c%d\n",
        unit, ???);
    return 1;
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
