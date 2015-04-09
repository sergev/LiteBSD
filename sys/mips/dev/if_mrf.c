/*
 * Network driver for Microchip MRF24WG0MA Wi-Fi controller.
 *
 * Copyright (c) 2015, Serge Vakulenko
 *
 * Loosely based on sources of Digilent deIPcK library by Keith Vogel.
 * Copyright (c) 2013-2014, Digilent <www.digilentinc.com>
 *
 * This program is free software; distributed under the terms of
 * BSD 3-clause license.
 */
#include "mrf.h"
#if NMRF > 0

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <mips/dev/device.h>
#include <mips/dev/spi.h>
#include <machine/pic32mz.h>
#include <machine/pic32_gpio.h>

#include <net/if.h>
#include <net/netisr.h>
#include <net/route.h>
#include <net/bpf.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#include "bpfilter.h"

#ifndef MRF_KHZ
#define MRF_KHZ         10000       /* SPI port speed 10 MHz */
#endif

/*
 * Wi-Fi software status per interface.
 */
struct wifi_port {
    struct arpcom arpcom;           /* Ethernet common part */
#define netif   arpcom.ac_if        /* network-visible interface */
#define macaddr arpcom.ac_enaddr    /* hardware Ethernet address */

    struct spiio spiio;             /* interface to SPI port */
    int         is_up;              /* whether the link is up */

} wifi_port[NMRF];

/*
 * Initialize hardware.
 */
static void mrf_setup()
{
    //TODO
}

/*
 * Setup output on interface.
 * Get another datagram to send off of the interface queue,
 * and map it to the interface before starting the output.
 * called only at splimp or interrupt level.
 */
static void mrf_start(struct ifnet *ifp)
{
    //TODO
}

/*
 * Initialization of interface; set up initialization block
 * and transmit/receive descriptor rings.
 */
static void mrf_init(int unit)
{
    //TODO
}

/*
 * Reset of interface.
 */
static void mrf_reset(int unit)
{
    //TODO
}

/*
 * Process an ioctl request.
 */
static int mrf_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
    //TODO
    return 0;
}

/*
 * Timeout routine.
 */
static void mrf_watchdog(int unit)
{
    //TODO
}

/*
 * Detect the presence of MRF24G controller at given SPI port.
 */
static int mrf_detect(struct spiio *io)
{
    //TODO
    return 0;
}

/*
 * Setup direction of signal pins.
 */
static void setup_signals(int pin_irq, int pin_reset, int pin_hibernate)
{
    //TODO
}

static int
mrf_probe(config)
    struct conf_device *config;
{
    int unit = config->dev_unit;
    int pin_cs = config->dev_flags & 0xFF;
    int pin_irq = config->dev_flags >> 8 & 0xFF;
    int pin_reset = config->dev_flags >> 16 & 0xFF;
    int pin_hib = config->dev_flags >> 24 & 0xFF;
    struct wifi_port *w = &wifi_port[unit];
    struct ifnet *ifp = &w->netif;
    struct spiio *io;
    int s;

    if (unit < 0 || unit >= NMRF)
        return 0;

    io = &w->spiio;
    if (spi_setup(io, config->dev_ctlr, pin_cs) != 0) {
        printf("sd%u: cannot open SPI%u port\n", unit, config->dev_ctlr);
        return 0;
    }
    spi_set_speed(io, MRF_KHZ);
    spi_set(io, PIC32_SPICON_CKE);

    //TODO: detect mrf24g at SPI port
    if (mrf_detect(io)) {
        printf("mrf%u not found at port %s, pin cs=%c%d\n",
            unit, spi_name(io), spi_csname(io), spi_cspin(io));
        return 0;
    }

    s = splimp();

    /* Board-dependent initialization. */
    setup_signals(pin_irq, pin_reset, pin_hib);

    /* Link is down. */
    w->is_up = 0;

    /* As per section 35.4.10 of the Pic32 Family Ref Manual. */
    mrf_setup();
#if 0
    /* Extract our MAC address */
    w->macaddr[0] = EMAC1SA2;
    w->macaddr[1] = EMAC1SA2 >> 8;
    w->macaddr[2] = EMAC1SA1;
    w->macaddr[3] = EMAC1SA1 >> 8;
    w->macaddr[4] = EMAC1SA0;
    w->macaddr[5] = EMAC1SA0 >> 8;
#endif
    splx(s);

    printf("mrf%u at port %s, MAC address %s\n", unit,
        spi_name(io), ether_sprintf(w->macaddr));
    printf("mrf%u: pins cs=%c%d, irq=%c%d, reset=%c%d, hibernate=%c%d\n",
        unit, spi_csname(io), spi_cspin(io),
        gpio_portname(pin_irq), gpio_pinno(pin_irq),
        gpio_portname(pin_reset), gpio_pinno(pin_reset),
        gpio_portname(pin_hib), gpio_pinno(pin_hib));

    ifp->if_unit = unit;
    ifp->if_name = "mrf";
    ifp->if_mtu = ETHERMTU;
    ifp->if_flags = IFF_BROADCAST | IFF_NOTRAILERS;
    ifp->if_init = mrf_init;
    ifp->if_output = ether_output;
    ifp->if_start = mrf_start;
    ifp->if_ioctl = mrf_ioctl;
    ifp->if_reset = mrf_reset;
    ifp->if_watchdog = mrf_watchdog;
    ifp->if_timer = 1;
#if NBPFILTER > 0
    bpfattach(&ifp->if_bpf, ifp, DLT_EN10MB, sizeof(struct ether_header));
#endif
    if_attach(ifp);
    return 1;
}

struct driver mrfdriver = {
    "mrf", mrf_probe,
};
#endif
