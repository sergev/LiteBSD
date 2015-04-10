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
#define MRF_KHZ         10000       /* SPI port speed, max 25 MHz */
#endif

/*
 * Wi-Fi software status per interface.
 */
struct wifi_port {
    struct arpcom arpcom;           /* Ethernet common part */
#define netif   arpcom.ac_if        /* network-visible interface */
#define macaddr arpcom.ac_enaddr    /* hardware Ethernet address */

    struct spiio spiio;             /* interface to SPI port */
    int         pin_irq;            /* /Int pin number */
    int         pin_reset;          /* /Reset pin number */
    int         pin_hibernate;      /* Hibernate pin number */
    int         is_up;              /* whether the link is up */

} wifi_port[NMRF];

/*
 * TODO: routines for mrf24g integration:
 *
 * 1) Interrupt routines
 *     WF_EintDisable
 *     WF_EintEnable
 *     WF_EintInit
 *     WF_isEintDisabled
 *
 * 2) GPIO stuff
 *     WF_GpioInit
 *     WF_GpioSetHibernate
 *     WF_GpioSetReset
 *
 * 3) SPI routines
 *     WF_SpiDisableChipSelect
 *     WF_SpiEnableChipSelect
 *     WF_SpiInit
 *     WF_SpiTxRx
 *
 * 4) Timer read
 *     WF_TimerInit
 *     WF_TimerRead
 *
 * 5) Processing of received data
 *     WF_ProcessEvent
 *     WF_ProcessRxPacket
 */

/*
 * Initialize hardware.
 */
static void mrf_setup(struct wifi_port *w)
{
    /* MRF24WG silicon work-around.
     * Needed for A1 silicon to initialize PLL values correctly. */
    //reset_pll();

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
static int mrf_detect(struct wifi_port *w)
{
    /* Setup direction of signal pins. */
    gpio_set_input(w->pin_irq);         /* /Int input */
    gpio_clr(w->pin_reset);
    gpio_set_output(w->pin_reset);      /* /Reset output low (active) */
    gpio_set(w->pin_hibernate);
    gpio_set_output(w->pin_hibernate);  /* Hibernate output high (active) */

    /* Take chip out of hibernate and out of reset. */
    udelay(2000);
    gpio_clr(w->pin_hibernate);         /* Hibernate signal low (inactive) */
    udelay(300000);
    gpio_set(w->pin_reset);             /* /Reset signal high (inactive) */
    udelay(5000);

    //TODO
    goto failed;

failed:
    gpio_set_input(w->pin_reset);
    gpio_set_input(w->pin_hibernate);
    return 0;
}

static int
mrf_probe(config)
    struct conf_device *config;
{
    int unit = config->dev_unit;
    int pin_cs = config->dev_flags & 0xFF;
    struct wifi_port *w = &wifi_port[unit];
    struct ifnet *ifp = &w->netif;
    struct spiio *io = &w->spiio;
    int s;

    if (unit < 0 || unit >= NMRF)
        return 0;

    if (spi_setup(io, config->dev_ctlr, pin_cs) != 0) {
        printf("sd%u: cannot open SPI%u port\n", unit, config->dev_ctlr);
        return 0;
    }
    spi_set_speed(io, MRF_KHZ);
    spi_set(io, PIC32_SPICON_CKE);

    /* Detect mrf24g chip. */
    w->pin_irq = config->dev_flags >> 8 & 0xFF;
    w->pin_reset = config->dev_flags >> 16 & 0xFF;
    w->pin_hibernate = config->dev_flags >> 24 & 0xFF;
    if (mrf_detect(w)) {
        printf("mrf%u not found at port %s, pin cs=%c%d\n",
            unit, spi_name(io), spi_csname(io), spi_cspin(io));
        return 0;
    }

    s = splimp();

    /* Link is down. */
    w->is_up = 0;

    /* As per section 35.4.10 of the Pic32 Family Ref Manual. */
    mrf_setup(w);
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
        gpio_portname(w->pin_irq), gpio_pinno(w->pin_irq),
        gpio_portname(w->pin_reset), gpio_pinno(w->pin_reset),
        gpio_portname(w->pin_hibernate), gpio_pinno(w->pin_hibernate));

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
