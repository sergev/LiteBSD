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

static int
mrf_probe(config)
    struct conf_device *config;
{
    int unit = config->dev_unit;
    int pin_cs = config->ctlr_flags & 0xFF;
    int pin_irq = config->ctlr_flags >> 8 & 0xFF;
    int pin_reset = config->ctlr_flags >> 16 & 0xFF;
    int pin_hib = config->ctlr_flags >> 24 & 0xFF;
    struct wifi_port *w = &wifi_port[unit];
    struct ifnet *ifp = &w->netif;
    struct spiio *io;
    int s;

    if (unit < 0 || unit >= NMRF)
        return 0;

    io = &du->spiio;
    if (spi_setup(io, config->dev_ctlr, pin_cs) != 0) {
        printf("sd%u: cannot open SPI%u port\n", unit, config->dev_ctlr);
        return 0;
    }
    spi_set_speed(io, MRF_KHZ);
    spi_set(io, PIC32_SPICON_CKE);

    //TODO: detect mrf24g at SPI port
    if (mrf_detect()) {
        printf("mrf%u not found at port %s, pin cs=%c%d\n",
            unit, spi_name(io), spi_csname(io), spi_cspin(io));
        return 0
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
        unit, spi_csname(io), spi_cspin(io)
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
