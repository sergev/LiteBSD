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
#include <mips/dev/mrf24g/wf_universal_driver.h>
#include <mips/dev/mrf24g/wf_registers.h>
#include <mips/dev/mrf24g/wf_raw.h>
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
 * To enable debug output, uncomment the following line.
 */
//#define PRINTDBG printf

/*
 * Wi-Fi software status per interface.
 */
struct wifi_port {
    struct arpcom arpcom;           /* Ethernet common part */
#define netif   arpcom.ac_if        /* network-visible interface */
#define macaddr arpcom.ac_enaddr    /* hardware Ethernet address */

    struct spiio spiio;             /* interface to SPI port */
    t_deviceInfo dev_info;          /* device type and version */
    unsigned    int_mask;           /* mask for interrupt pin */
    int         pin_irq;            /* /Int pin number */
    int         pin_reset;          /* /Reset pin number */
    int         pin_hibernate;      /* Hibernate pin number */
    int         is_up;              /* whether the link is up */

} wifi_port[NMRF];

#ifndef PRINTDBG
#   define PRINTDBG(...) /*empty*/
#else
static const char *reg_name(unsigned regno)
{
    static char buf[8] = "[00]";

    switch (regno) {
    /*
     * 8-bit registers
     */
    case WF_HOST_INTR_REG:              return "INTR";
    case WF_HOST_MASK_REG:              return "MASK";
    case RAW_0_DATA_REG:                return "RAW0_DATA";
    case RAW_1_DATA_REG:                return "RAW1_DATA";
    case RAW_2_DATA_REG:                return "RAW2_DATA";
    case RAW_3_DATA_REG:                return "RAW3_DATA";
    case RAW_4_DATA_REG:                return "RAW4_DATA";
    case RAW_5_DATA_REG:                return "RAW5_DATA";
    /*
     * 16-bit registers
     */
    case WF_HOST_MAIL_BOX_0_MSW_REG:    return "MBOX0_MSW";
    case WF_HOST_MAIL_BOX_0_LSW_REG:    return "MBOX0_LSW";
    case WF_HOST_RESET_REG:             return "RESET";
    case WF_HOST_INTR2_REG:             return "INTR2";
    case WF_HOST_INTR2_MASK_REG:        return "MASK2";
    case WF_HOST_WFIFO_BCNT0_REG:       return "WFIFO_BCNT0";
    case WF_HOST_WFIFO_BCNT1_REG:       return "WFIFO_BCNT1";
    case WF_HOST_RFIFO_BCNT0_REG:       return "RFIFO_BCNT0";
    case WF_HOST_RFIFO_BCNT1_REG:       return "RFIFO_BCNT1";
    case WF_PSPOLL_H_REG:               return "PSPOLL";
    case WF_INDEX_ADDR_REG:             return "ADDR";
    case WF_INDEX_DATA_REG:             return "DATA";
    case RAW_0_CTRL_0_REG:              return "RAW0_CTRL0";
    case RAW_0_CTRL_1_REG:              return "RAW0_CTRL1";
    case RAW_0_INDEX_REG:               return "RAW0_INDEX";
    case RAW_0_STATUS_REG:              return "RAW0_STATUS";
    case RAW_1_CTRL_0_REG:              return "RAW1_CTRL0";
    case RAW_1_CTRL_1_REG:              return "RAW1_CTRL1";
    case RAW_1_INDEX_REG:               return "RAW1_INDEX";
    case RAW_1_STATUS_REG:              return "RAW1_STATUS";
    case RAW_2_CTRL_0_REG:              return "RAW2_CTRL0";
    case RAW_2_CTRL_1_REG:              return "RAW2_CTRL1";
    case RAW_2_INDEX_REG:               return "RAW2_INDEX";
    case RAW_2_STATUS_REG:              return "RAW2_STATUS";
    case RAW_3_CTRL_0_REG:              return "RAW3_CTRL0";
    case RAW_3_CTRL_1_REG:              return "RAW3_CTRL1";
    case RAW_3_INDEX_REG:               return "RAW3_INDEX";
    case RAW_3_STATUS_REG:              return "RAW3_STATUS";
    case RAW_4_CTRL_0_REG:              return "RAW4_CTRL0";
    case RAW_4_CTRL_1_REG:              return "RAW4_CTRL1";
    case RAW_4_INDEX_REG:               return "RAW4_INDEX";
    case RAW_4_STATUS_REG:              return "RAW4_STATUS";
    case RAW_5_CTRL_0_REG:              return "RAW5_CTRL0";
    case RAW_5_CTRL_1_REG:              return "RAW5_CTRL1";
    case RAW_5_INDEX_REG:               return "RAW5_INDEX";
    case RAW_5_STATUS_REG:              return "RAW5_STATUS";
    }
    buf[1] = "0123456789abcdef"[regno>>4];
    buf[2] = "0123456789abcdef"[regno&15];
    return buf;
}
#endif /* PRINTDBG */

/*-------------------------------------------------------------
 * Read 8-bit register.
 * Return register value.
 */
unsigned WF_ReadByte(unsigned regno)
{
    struct wifi_port *w = &wifi_port[0];
    u_int8_t reply;

    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno | WF_READ_REGISTER_MASK);
    reply = spi_transfer(&w->spiio, 0xff);
    spi_deselect(&w->spiio);
    PRINTDBG("-- read byte %s -> %02x\n", reg_name(regno), reply);
    return reply;
}

/*
 * Write 8-bit register.
 */
void WF_WriteByte(unsigned regno, unsigned value)
{
    struct wifi_port *w = &wifi_port[0];

    PRINTDBG("-- write byte %s = %02x\n", reg_name(regno), value);
    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno);
    spi_transfer(&w->spiio, value);
    spi_deselect(&w->spiio);
}

/*
 * Read 16-bit register.
 * Return register value.
 */
unsigned WF_Read(unsigned regno)
{
    struct wifi_port *w = &wifi_port[0];
    u_int8_t reply[3];

    spi_select(&w->spiio);
    reply[0] = spi_transfer(&w->spiio, regno | WF_READ_REGISTER_MASK);
    reply[1] = spi_transfer(&w->spiio, 0xff);
    reply[2] = spi_transfer(&w->spiio, 0xff);
    spi_deselect(&w->spiio);
    PRINTDBG("-- read %s -> %04x\n", reg_name(regno), (reply[1] << 8) | reply[2]);
    return (reply[1] << 8) | reply[2];
}

/*
 * Write 16-bit register.
 */
void WF_Write(unsigned regno, unsigned value)
{
    struct wifi_port *w = &wifi_port[0];

    PRINTDBG("-- write %s = %04x\n", reg_name(regno), value);
    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno);
    spi_transfer(&w->spiio, value >> 8);
    spi_transfer(&w->spiio, value & 0xff);
    spi_deselect(&w->spiio);
}

/*
 * Read a block of data from a register.
 */
void WF_ReadArray(unsigned regno, u_int8_t *data, unsigned nbytes)
{
    struct wifi_port *w = &wifi_port[0];

    PRINTDBG("-- read %u bytes from %s\n", nbytes, reg_name(regno));
    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno | WF_READ_REGISTER_MASK);
    while (nbytes-- > 0) {
        *data++ = spi_transfer(&w->spiio, 0xff);
    }
    spi_deselect(&w->spiio);
}

/*
 * Write a data block to specified register.
 */
void WF_WriteArray(unsigned regno, const u_int8_t *data, unsigned nbytes)
{
    struct wifi_port *w = &wifi_port[0];

    PRINTDBG("-- write %u bytes to %s\n", nbytes, reg_name(regno));
    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno);
    while (nbytes-- > 0) {
        spi_transfer(&w->spiio, *data++);
    }
    spi_deselect(&w->spiio);
}

/*
 * Write to analog register via bitbang.
 */
static void WF_WriteAnalog(unsigned bank, unsigned address, unsigned value)
{
    unsigned reset, mask;

    /* Create register address byte. */
    address <<= 2;

    /* Enable the on-chip SPI and select the desired bank (0-3). */
    reset = HR_HOST_ANA_SPI_EN_MASK | (bank << 6);
    WF_Write(WF_HOST_RESET_REG, reset);

    /* Bit-bang the address byte, MS bit to LS bit. */
    for (mask = 0x80; mask; mask >>= 1) {
        if (address & mask)
            reset |= HR_HOST_ANA_SPI_DOUT_MASK;
        else
            reset &= ~HR_HOST_ANA_SPI_DOUT_MASK;

        WF_Write(WF_HOST_RESET_REG, reset);
        WF_Write(WF_HOST_RESET_REG, reset | HR_HOST_ANA_SPI_CLK_MASK);
    }

    /* Bit bang data from MS bit to LS bit. */
    for (mask = 0x8000; mask; mask >>= 1) {
        if (value & mask)
            reset |= HR_HOST_ANA_SPI_DOUT_MASK;
        else
            reset &= ~HR_HOST_ANA_SPI_DOUT_MASK;

        WF_Write(WF_HOST_RESET_REG, reset);
        WF_Write(WF_HOST_RESET_REG, reset | HR_HOST_ANA_SPI_CLK_MASK);
    }

    /* Disable the on-chip SPI. */
    reset &= ~HR_HOST_ANA_SPI_EN_MASK;
    WF_Write(WF_HOST_RESET_REG, reset);
}

/*-------------------------------------------------------------
 * Return the current value of the 1ms timer.
 */
unsigned WF_TimerRead()
{
    unsigned count = mfc0_Count();

    count /= CPU_KHZ / 2;
    return count;
}

int WF_TimerElapsed(unsigned start_time)
{
    int elapsed = WF_TimerRead() - start_time;

    if (elapsed < 0)
        elapsed += 0xffffffffU / (CPU_KHZ / 2);

    return elapsed;
}

/*-------------------------------------------------------------
 * Configure host processor external interrupt.
 * This line is asserted low by MRF24WG.
 * Config parameter WF_INT defines the interrupt pin to use:
 * one of INT1, INT2, INT3 or INT4.
 */
void WF_EintInit()
{
    struct wifi_port *w = &wifi_port[0];

#define LETH    4               /* Interrupt priority level for splimp() */

    /* Set interrupt priority. */
    switch (WF_INT) {
    case 0:                             /* External Interrupt 0 */
        /* INT0 pin is fixed mapped. */
        w->int_mask = 1 << PIC32_IRQ_INT0;
        IPC(PIC32_IRQ_INT0/4) |= PIC32_IPC_IP(0, 0, 0, LETH);
        break;
    case 1:                             /* External Interrupt 1 */
        INT1R = gpio_input_map1(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT1;
        IPC(PIC32_IRQ_INT1/4) |= PIC32_IPC_IP(LETH, 0, 0, 0);
        break;
    case 2:                             /* External Interrupt 2 */
        INT2R = gpio_input_map2(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT2;
        IPC(PIC32_IRQ_INT2/4) |= PIC32_IPC_IP(0, LETH, 0, 0);
        break;
    case 3:                             /* External Interrupt 3 */
        INT3R = gpio_input_map3(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT3;
        IPC(PIC32_IRQ_INT3/4) |= PIC32_IPC_IP(0, 0, LETH, 0);
        break;
    case 4:                             /* External Interrupt 4 */
        INT4R = gpio_input_map4(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT4;
        IPC(PIC32_IRQ_INT4/4) |= PIC32_IPC_IP(0, 0, 0, LETH);
        break;
    }

    IECCLR(0) = w->int_mask;            /* disable /INT interrupt */
    INTCONCLR = w->int_mask;            /* falling edge triggered */
    IFSCLR(0) = w->int_mask;            /* ensure the interrupt is cleared */
    IECSET(0) = w->int_mask;            /* enable the interrupt */
}

/*
 * Determines if the external interrupt is disabled.
 * Returns True if interrupt is disabled, else False.
 */
int WF_isEintDisabled()
{
    struct wifi_port *w = &wifi_port[0];

    return ! (IEC(0) & w->int_mask);
}

/*
 * Disable the MRF24WG external interrupt.
 */
void WF_EintDisable()
{
    struct wifi_port *w = &wifi_port[0];

    IECCLR(0) = w->int_mask;
}

/*
 * Enable the MRF24WG external interrupt.
 *
 * When using level-triggered interrupts it is possible that host MCU
 * could miss a falling edge; this can occur because during normal
 * operations as the Universal Driver disables the external interrupt for
 * short periods.  If the MRF24WG asserts the interrupt line while the
 * interrupt is disabled the falling edge won't be detected.  So, a
 * check must be made to determine if an interrupt is pending; if so, the
 * interrupt must be forced.
 * This is not an issue for level-triggered interrupts.
 */
void WF_EintEnable()
{
    struct wifi_port *w = &wifi_port[0];

    /* PIC32 uses level-triggered interrupts, so it is possible the Universal Driver
     * may have temporarily disabled the external interrupt, and then missed the
     * falling edge when the MRF24WG asserted the interrupt line.  The code below
     * checks for this condition and forces the interrupt if needed. */

    /* If interrupt line is low, then PIC32 may have missed the falling edge
     * while the interrupt was disabled. */
    if (gpio_get(w->pin_irq) == 0) {
        /* Need to force the interrupt for two reasons:
         *  1) there is an event that needs to be serviced
         *  2) MRF24WG won't generate another falling edge until the interrupt
         *     is processed. */
        IFSSET(0) = w->int_mask;
    }

    /* Enable the external interrupt. */
    IECSET(0) = w->int_mask;
}

/*-------------------------------------------------------------
 * Called by Universal Driver to inform application of MRF24WG events.
 *
 * Various events are reported to the application via this function callback.
 * The application should take appropriate action based on the event.
 */
#if 0
void WF_ProcessEvent(unsigned event_type, unsigned event_data)
{
    //TODO
    wfmrf24.priv.fMRFBusy = 0;
    wfmrf24.priv.lastEventType = eventType;
    wfmrf24.priv.lastEventData = eventData;

    switch (eventType) {
    case WF_EVENT_INITIALIZATION:
        if (eventData == WF_INIT_SUCCESSFUL) {
            wfmrf24.priv.initStatus = ForceIPStatus(InitMask | eventData);
        } else {
            wfmrf24.priv.initStatus = ForceIPError(InitMask | eventData);
        }
        break;

    case WF_EVENT_CONNECTION_SUCCESSFUL:
        wfmrf24.priv.connectionStatus = ipsSuccess;
        break;

    case WF_EVENT_CONNECTION_TEMPORARILY_LOST:
        wfmrf24.priv.connectionStatus = ForceIPStatus((CLMask | eventData));
        wfmrf24.priv.fMRFBusy = 1;  // don't do anything during the reconnect!
        break;

    case WF_EVENT_CONNECTION_REESTABLISHED:
        wfmrf24.priv.connectionStatus = ipsSuccess;
        break;

    case WF_EVENT_CONNECTION_PERMANENTLY_LOST:
        wfmrf24.priv.connectionStatus = ForceIPError((CLMask | eventData));
        break;

    case WF_EVENT_CONNECTION_FAILED:
        wfmrf24.priv.connectionStatus = ForceIPError((CFMask | eventData));
        break;

    case WF_EVENT_SCAN_RESULTS_READY:
        wfmrf24.priv.cScanResults = eventData;
        break;

    case WF_WPS_EVENT_KEY_CALCULATION_REQUEST:
        WF_WpsKeyGenerate();        // can be called here or later, but must be called
                                    // to complete WPS connection
        wfmrf24.priv.fMRFBusy = 1;  // wait for connection status or error.
        break;

    default:
        break;
    }
}
#endif

/*
 * Called by Universal Driver to notify application of incoming packet.
 */
void WF_ProcessRxPacket()
{
    //TODO
#if 0
    u_int16_t nbytes = WF_RxPacketLengthGet();

    if (nbytes > 0) {
        // now that buffer mounted it is safe to reenable interrupts, which were left disabled
        // in the WiFi interrupt handler.
        WF_EintEnable();

        char *data = malloc(nbytes);
        if (data != 0) {
            WF_RxPacketCopy(data, nbytes);
            WF_RxPacketDeallocate();

            //TODO: process the data.
        } else {
            // if we know we can never allocate this packet, then just drop it
            WF_RxPacketDeallocate();
        }
    }
#endif
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
 * External interrupt.
 *
 * This interrupt handler should:
 * 1) ensure the interrupt is disabled upon exit (Universal Driver will reenable it)
 * 2) clear the interrupt
 * 3) call WF_EintHandler()
 */
void mrfintr(dev_t dev)
{
    struct wifi_port *w = &wifi_port[0];
printf("---mrf0 interrupt\n");

    IFSCLR(0) = w->int_mask;            /* clear the interrupt */
    IECCLR(0) = w->int_mask;            /* disable external interrupt */
    WF_EintHandler();                   /* call handler function */
}

/*
 * Detect the presence of MRF24G controller at given SPI port.
 * Return -1 on error.
 */
static int mrf_detect(struct wifi_port *w)
{
    int mask2, v;

    /* Setup direction of signal pins. */
    gpio_set(w->pin_irq);
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

    /* Shuttle MRF24WG workaround (benign to production MRF24WG) */
    WF_WriteAnalog(2, PLL0_REG, 0x8021);
    WF_WriteAnalog(2, PLL0_REG, 0x6021);

    /* Production MRF24WG workaround (benign to shuttle MRF24WG) */
    WF_WriteAnalog(1, OSC0_REG, 0x6b80);
    WF_WriteAnalog(1, BIAS_REG, 0xc000);

    /* Check whether we really have MRF24G chip attached. */
    mask2 = WF_Read(WF_HOST_INTR2_MASK_REG);
    WF_Write(WF_HOST_INTR2_MASK_REG, 0xaa55);
    v = WF_Read(WF_HOST_INTR2_MASK_REG);
    if (v != 0xaa55)
        goto failed;
    WF_Write(WF_HOST_INTR2_MASK_REG, 0x55aa);
    v = WF_Read(WF_HOST_INTR2_MASK_REG);
    if (v != 0x55aa)
        goto failed;
    WF_Write(WF_HOST_INTR2_MASK_REG, mask2);

    /* MRF24G controller detected */
    return 0;

failed:
    gpio_set_input(w->pin_reset);
    gpio_set_input(w->pin_hibernate);
    return -1;
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

    if (unit < 0 || unit >= NMRF)
        return 0;

    if (spi_setup(io, config->dev_ctlr, pin_cs) != 0) {
        printf("mrf%u: cannot open SPI%u port\n", unit, config->dev_ctlr);
        return 0;
    }
    spi_set_speed(io, MRF_KHZ);
    spi_set(io, PIC32_SPICON_CKE);

    /* Detect mrf24g chip. */
    w->pin_irq = config->dev_flags >> 8 & 0xFF;
    w->pin_reset = config->dev_flags >> 16 & 0xFF;
    w->pin_hibernate = config->dev_flags >> 24 & 0xFF;
    if (mrf_detect(w) < 0) {
        printf("mrf%u not found at port %s, pin cs=%c%d\n",
            unit, spi_name(io), spi_csname(io), spi_cspin(io));
        return 0;
    }
    printf("mrf%u at port %s, pins cs=%c%d, irq=%c%d, reset=%c%d, hibernate=%c%d\n",
        unit, spi_name(io), spi_csname(io), spi_cspin(io),
        gpio_portname(w->pin_irq), gpio_pinno(w->pin_irq),
        gpio_portname(w->pin_reset), gpio_pinno(w->pin_reset),
        gpio_portname(w->pin_hibernate), gpio_pinno(w->pin_hibernate));

    /* Initialize the chip with interrupts disabled.
     * Extract the MAC address. */
    int s = splimp();
    WF_Init(&w->dev_info);
    WF_MacAddressGet(w->macaddr);
    splx(s);

    if (w->dev_info.deviceType != WF_MRF24WG_DEVICE) {
        printf("mrf%u: unknown device type, only MRF24WG is supported\n", unit);
        return 0;
    }
    printf("mrf%u: MRF24WG version %x.%x, MAC address %s\n",
        unit, w->dev_info.romVersion, w->dev_info.patchVersion,
        ether_sprintf(w->macaddr));

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
