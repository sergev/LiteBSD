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
    unsigned    rom_version;        /* device ROM version */
    unsigned    int_mask;           /* mask for interrupt pin */
    unsigned    cpid;               /* connection profile ID */
    int         pin_irq;            /* /Int pin number */
    int         pin_reset;          /* /Reset pin number */
    int         pin_hibernate;      /* Hibernate pin number */
    int         is_up;              /* whether the link is up */
    int         is_connected;       /* whether the wifi is connected */
    int         is_powersave_active; /* power save mode enabled */
    int         is_scan_ready;      /* scan results ready */
    int         need_powersave;     /* reactivate PS mode when appropriate */
    key_info_t  key_info;

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
    case MRF24_REG_INTR:        return "INTR";
    case MRF24_REG_MASK:        return "MASK";
    case MRF24_REG_RAW0_DATA:   return "RAW0_DATA";
    case MRF24_REG_RAW1_DATA:   return "RAW1_DATA";
    case MRF24_REG_RAW2_DATA:   return "RAW2_DATA";
    case MRF24_REG_RAW3_DATA:   return "RAW3_DATA";
    case MRF24_REG_RAW4_DATA:   return "RAW4_DATA";
    case MRF24_REG_RAW5_DATA:   return "RAW5_DATA";
    /*
     * 16-bit registers
     */
    case MRF24_REG_MAILBOX0_HI: return "MBOX0_MSW";
    case MRF24_REG_MAILBOX0_LO: return "MBOX0_LSW";
    case MRF24_REG_RESET:       return "RESET";
    case MRF24_REG_INTR2:       return "INTR2";
    case MRF24_REG_MASK2:       return "MASK2";
    case MRF24_REG_WFIFO_BCNT0: return "WFIFO_BCNT0";
    case MRF24_REG_WFIFO_BCNT1: return "WFIFO_BCNT1";
    case MRF24_REG_RFIFO_BCNT0: return "RFIFO_BCNT0";
    case MRF24_REG_RFIFO_BCNT1: return "RFIFO_BCNT1";
    case MRF24_REG_PSPOLL:      return "PSPOLL";
    case MRF24_REG_ADDR:        return "ADDR";
    case MRF24_REG_DATA:        return "DATA";
    case MRF24_REG_RAW0_CTRL0:  return "RAW0_CTRL0";
    case MRF24_REG_RAW0_CTRL1:  return "RAW0_CTRL1";
    case MRF24_REG_RAW0_INDEX:  return "RAW0_INDEX";
    case MRF24_REG_RAW0_STATUS: return "RAW0_STATUS";
    case MRF24_REG_RAW1_CTRL0:  return "RAW1_CTRL0";
    case MRF24_REG_RAW1_CTRL1:  return "RAW1_CTRL1";
    case MRF24_REG_RAW1_INDEX:  return "RAW1_INDEX";
    case MRF24_REG_RAW1_STATUS: return "RAW1_STATUS";
    case MRF24_REG_RAW2_CTRL0:  return "RAW2_CTRL0";
    case MRF24_REG_RAW2_CTRL1:  return "RAW2_CTRL1";
    case MRF24_REG_RAW2_INDEX:  return "RAW2_INDEX";
    case MRF24_REG_RAW2_STATUS: return "RAW2_STATUS";
    case MRF24_REG_RAW3_CTRL0:  return "RAW3_CTRL0";
    case MRF24_REG_RAW3_CTRL1:  return "RAW3_CTRL1";
    case MRF24_REG_RAW3_INDEX:  return "RAW3_INDEX";
    case MRF24_REG_RAW3_STATUS: return "RAW3_STATUS";
    case MRF24_REG_RAW4_CTRL0:  return "RAW4_CTRL0";
    case MRF24_REG_RAW4_CTRL1:  return "RAW4_CTRL1";
    case MRF24_REG_RAW4_INDEX:  return "RAW4_INDEX";
    case MRF24_REG_RAW4_STATUS: return "RAW4_STATUS";
    case MRF24_REG_RAW5_CTRL0:  return "RAW5_CTRL0";
    case MRF24_REG_RAW5_CTRL1:  return "RAW5_CTRL1";
    case MRF24_REG_RAW5_INDEX:  return "RAW5_INDEX";
    case MRF24_REG_RAW5_STATUS: return "RAW5_STATUS";
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
unsigned mrf_read_byte(unsigned regno)
{
    struct wifi_port *w = &wifi_port[0];
    u_int8_t reply;

    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno | MRF24_READ_MODE);
    reply = spi_transfer(&w->spiio, 0xff);
    spi_deselect(&w->spiio);
    PRINTDBG("-- read %s -> %02x\n", reg_name(regno), reply);
    return reply;
}

/*
 * Write 8-bit register.
 */
void mrf_write_byte(unsigned regno, unsigned value)
{
    struct wifi_port *w = &wifi_port[0];

    PRINTDBG("-- write %s = %02x\n", reg_name(regno), value);
    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno);
    spi_transfer(&w->spiio, value);
    spi_deselect(&w->spiio);
}

/*
 * Read 16-bit register.
 * Return register value.
 */
unsigned mrf_read(unsigned regno)
{
    struct wifi_port *w = &wifi_port[0];
    u_int8_t reply[3];

    spi_select(&w->spiio);
    reply[0] = spi_transfer(&w->spiio, regno | MRF24_READ_MODE);
    reply[1] = spi_transfer(&w->spiio, 0xff);
    reply[2] = spi_transfer(&w->spiio, 0xff);
    spi_deselect(&w->spiio);
    PRINTDBG("-- read %s -> %04x\n", reg_name(regno), (reply[1] << 8) | reply[2]);
    return (reply[1] << 8) | reply[2];
}

/*
 * Write 16-bit register.
 */
void mrf_write(unsigned regno, unsigned value)
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
void mrf_read_array(unsigned regno, u_int8_t *data, unsigned nbytes)
{
    struct wifi_port *w = &wifi_port[0];

    PRINTDBG("-- read %u bytes from %s: ", nbytes, reg_name(regno));
    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno | MRF24_READ_MODE);
    while (nbytes-- > 0) {
        *data++ = spi_transfer(&w->spiio, 0xff);
        PRINTDBG("%02x", data[-1]);
        if (nbytes > 0)
            PRINTDBG("-");
    }
    PRINTDBG("\n");
    spi_deselect(&w->spiio);
}

/*
 * Write a data block to specified register.
 */
void mrf_write_array(unsigned regno, const u_int8_t *data, unsigned nbytes)
{
    struct wifi_port *w = &wifi_port[0];

    PRINTDBG("-- write %u bytes to %s: ", nbytes, reg_name(regno));
    spi_select(&w->spiio);
    spi_transfer(&w->spiio, regno);
    while (nbytes-- > 0) {
        PRINTDBG("%02x", *data);
        if (nbytes > 0)
            PRINTDBG("-");
        spi_transfer(&w->spiio, *data++);
    }
    PRINTDBG("\n");
    spi_deselect(&w->spiio);
}

/*
 * Write to analog register via bitbang.
 */
static void mrf_write_analog(unsigned bank, unsigned address, unsigned value)
{
    unsigned reset, mask;

    /* Create register address byte. */
    address <<= 2;

    /* Enable the on-chip SPI and select the desired bank (0-3). */
    reset = RESET_ANA_SPI_EN | (bank << 6);
    mrf_write(MRF24_REG_RESET, reset);

    /* Bit-bang the address byte, MS bit to LS bit. */
    for (mask = 0x80; mask; mask >>= 1) {
        if (address & mask)
            reset |= RESET_ANA_SPI_DOUT;
        else
            reset &= ~RESET_ANA_SPI_DOUT;

        mrf_write(MRF24_REG_RESET, reset);
        mrf_write(MRF24_REG_RESET, reset | RESET_ANA_SPI_CLK);
    }

    /* Bit bang data from MS bit to LS bit. */
    for (mask = 0x8000; mask; mask >>= 1) {
        if (value & mask)
            reset |= RESET_ANA_SPI_DOUT;
        else
            reset &= ~RESET_ANA_SPI_DOUT;

        mrf_write(MRF24_REG_RESET, reset);
        mrf_write(MRF24_REG_RESET, reset | RESET_ANA_SPI_CLK);
    }

    /* Disable the on-chip SPI. */
    reset &= ~RESET_ANA_SPI_EN;
    mrf_write(MRF24_REG_RESET, reset);
}

/*-------------------------------------------------------------
 * Return the current value of the 1ms timer.
 */
unsigned mrf_timer_read()
{
    unsigned count = mfc0_Count();

    count /= CPU_KHZ / 2;
    return count;
}

int mrf_timer_elapsed(unsigned start_time)
{
    int elapsed = mrf_timer_read() - start_time;

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
void mrf_intr_init()
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
    case 1:                             /* External Interrupt 1 - group 4 */
        INT1R = gpio_input_map4(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT1;
        IPC(PIC32_IRQ_INT1/4) |= PIC32_IPC_IP(LETH, 0, 0, 0);
        break;
    case 2:                             /* External Interrupt 2 - group 3 */
        INT2R = gpio_input_map3(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT2;
        IPC(PIC32_IRQ_INT2/4) |= PIC32_IPC_IP(0, LETH, 0, 0);
        break;
    case 3:                             /* External Interrupt 3 - group 1 */
        INT3R = gpio_input_map1(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT3;
        IPC(PIC32_IRQ_INT3/4) |= PIC32_IPC_IP(0, 0, LETH, 0);
        break;
    case 4:                             /* External Interrupt 4 - group 2 */
        INT4R = gpio_input_map2(w->pin_irq);
        w->int_mask = 1 << PIC32_IRQ_INT4;
        IPC(PIC32_IRQ_INT4/4) |= PIC32_IPC_IP(0, 0, 0, LETH);
        break;
    }

    IECCLR(0) = w->int_mask;            /* disable /INT interrupt */
    INTCONCLR = w->int_mask;            /* falling edge triggered */
    IFSCLR(0) = w->int_mask;            /* ensure the interrupt is cleared */
    IECSET(0) = w->int_mask;            /* enable the interrupt */
}

/*-------------------------------------------------------------
 * Called by Universal Driver to inform application of MRF24WG events.
 *
 * Various events are reported to the application via this function callback.
 * The application should take appropriate action based on the event.
 */
void mrf_event(event_t event_type, void *event_data)
{
    struct wifi_port *w = &wifi_port[0];

    switch (event_type) {
    case WF_EVENT_CONNECTION_SUCCESSFUL:
        printf("--- %s: connection successful\n", __func__);
        w->is_connected = 1;
        break;

    case WF_EVENT_CONNECTION_FAILED:
        printf("--- %s: connection failed, code=0x%x\n",
            __func__, (unsigned) event_data);
        w->is_connected = 0;
        break;

    case WF_EVENT_CONNECTION_TEMPORARILY_LOST:
        printf("--- %s: connection temporarily lost, code=0x%x\n",
            __func__, (unsigned) event_data);
        w->is_connected = 0;
        break;

    case WF_EVENT_CONNECTION_PERMANENTLY_LOST:
        printf("--- %s: connection lost, code=0x%x\n",
            __func__, (unsigned) event_data);
        w->is_connected = 0;
        break;

    case WF_EVENT_CONNECTION_REESTABLISHED:
        printf("--- %s: connection reestablished, code=0x%x\n",
            __func__, (unsigned) event_data);
        w->is_connected = 1;
        break;

    case WF_EVENT_SCAN_RESULTS_READY:
        printf("--- %s: scan results ready, count=%u\n", __func__, (unsigned) event_data);
        w->is_scan_ready = (unsigned) event_data;
        break;

    case WF_WPS_EVENT_KEY_CALCULATION_REQUEST:
        printf("--- %s: key calculation request\n", __func__);
        bcopy(event_data, &w->key_info, sizeof(w->key_info));
#if 0
        // Create the binary key.
        // This is too cpu-consuming to run with interrupts disabled.
        //TODO: reschedule to handle it in the watchdog routine.
        u_int8_t key[32];
        mrf_passphrase_to_key(w->key_info.pass, w->key_info.ssid,
            w->key_info.ssid_len, key);

        // Send it to MRF24WG.
        mrf_set_psk(key);
#endif
        break;

    case WF_EVENT_MRF24WG_MODULE_ASSERT:
        printf("--- %s: assert info=%04x\n", __func__, (unsigned) event_data);
        break;
    }
}

/*
 * Get the device out of power save mode before any message transmission.
 * Set flag to reactivate power save mode later.
 */
void mrf_awake()
{
    struct wifi_port *w = &wifi_port[0];

    if (w->is_powersave_active) {   /* if the application has enabled PS mode */
        mrf_powersave_activate(0);  /* wake up MRF24WG */
        w->is_powersave_active = 0;
        w->need_powersave = 1;      /* set flag to put PS mode back later */
    }
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
 * Scan the band for networksTimeout routine.
 */
void wifi_scan()
{
    struct wifi_port *w = &wifi_port[0];
    int s = splimp();

    mrf_scan_start(w->cpid);
    splx(s);
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
    unsigned intr, intr2 = 0, assert_info, nbytes;

    /* Read INTR register to determine cause of interrupt.
     * AND it with mask to determine which enabled interrupt has occurred. */
again:
    intr = mrf_read_byte(MRF24_REG_INTR);
    if (intr & INTR_INT2)
        intr2 = mrf_read(MRF24_REG_INTR2);
//printf("---mrf0 interrupt: intr = %02x:%02x, mask = %02x:%02x\n", intr, intr2, mrf_read_byte(MRF24_REG_MASK), mrf_read(MRF24_REG_MASK2));

    if (intr & INTR_INT2) {
        /*
         * Either a mgmt tx or mgmt rx Raw move complete occurred.
         * There is one other event to check for;
         * this interrupt is also used by the MRF24WG to signal that it has
         * hit an assert condition.  So, we check for that here.
         */
        if (intr2 & INTR2_MAILBOX) {
            /* MRF24WG has hit an assert condition.
             * Module number in upper 8 bits,
             * assert information in lower 20 bits. */
            assert_info = (mrf_read(MRF24_REG_MAILBOX0_HI) << 16) |
                          mrf_read(MRF24_REG_MAILBOX0_LO);
            mrf_event(WF_EVENT_MRF24WG_MODULE_ASSERT, (void*) assert_info);
        }

        /* Clear this interrupt. */
        mrf_write(MRF24_REG_INTR2, intr2);
    }
    if (intr & INTR_FIFO1) {
        /*
         * Got a FIFO 1 Threshold interrupt (Management Fifo).
         * Receive a mgmt msg, either confirm or indicate.
         */
        mrf_mgmt_receive_confirm();
#if 1
        if (w->is_scan_ready) {
            scan_result_t scan;
            int i, n = w->is_scan_ready;

            for (i=0; i<n; i++) {
                mrf_scan_get_result(i, &scan);
                scan.ssid[scan.ssidLen] = 0;
                printf("%02x:%02x:%02x:%02x:%02x:%02x ch%u, rssi=%u, ",
                    scan.bssid[0], scan.bssid[1], scan.bssid[2],
                    scan.bssid[3], scan.bssid[4], scan.bssid[5],
                    scan.channel, scan.rssi);
                if (scan.apConfig & 0x10) {
                    switch (scan.apConfig & 0xc0) {
                    case 0x00: printf("WEP"); break;
                    case 0x40: printf("WPA"); break;
                    case 0x80: printf("WPA2"); break;
                    case 0xc0: printf("WPA2+WPA"); break;
                    }
                } else {
                    printf("Open");
                }
                printf(", '%s'\n", scan.ssid);
            }
            w->is_scan_ready = 0;
        }
#endif
    }
    if (intr & INTR_FIFO0) {
        /*
         * Got a FIFO 0 Threshold Interrupt (Data Fifo).
         * Receive data packet.
         */
        nbytes = mrf_rx_get_length();
printf("--- got data message nbytes=%u\n", nbytes);
        if (nbytes > 0) {
#if 0
            char *data = malloc(nbytes);
            if (data != 0) {
                /* Extract received data. */
                mrf_raw_read(RAW_ID_RECEIVE, data, nbytes);
                //TODO: process the data.
            }
#endif
            /* Deallocate a Data Rx buffer. */
            mrf_raw_move(RAW_ID_RECEIVE, RAW_DATA_POOL, 0, 0);
        }
    }
    //TODO: process TX confirm interrupt.

    /* Clear this interrupt. */
    mrf_write(MRF24_REG_INTR, intr);

    /* Clear the interrupt flag on exit from the service routine. */
    IFSCLR(0) = w->int_mask;
    if (gpio_get(w->pin_irq) == 0)
        goto again;
    mrf_write_byte(MRF24_REG_MASK, INTR_FIFO0 | INTR_FIFO1 | INTR_RAW0 | INTR_RAW1);
    mrf_write(MRF24_REG_MASK2, INTR2_RAW2 | INTR2_RAW3 | INTR2_RAW4 | INTR2_RAW5 | INTR2_MAILBOX);

    /* Check whether PS-Poll was disabled temporarily and needs
     * to be reenabled, and we are in a connected state. */
    if (w->need_powersave && w->is_connected) {
        mrf_powersave_activate(1);
        w->need_powersave = 0;
        w->is_powersave_active = 1;
    }
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
    mrf_write_analog(2, ANALOG_REG_PLL0, 0x8021);
    mrf_write_analog(2, ANALOG_REG_PLL0, 0x6021);

    /* Production MRF24WG workaround (benign to shuttle MRF24WG) */
    mrf_write_analog(1, ANALOG_REG_OSC0, 0x6b80);
    mrf_write_analog(1, ANALOG_REG_BIAS, 0xc000);

    /* Check whether we really have MRF24G chip attached. */
    mask2 = mrf_read(MRF24_REG_MASK2);
    mrf_write(MRF24_REG_MASK2, 0xaa55);
    v = mrf_read(MRF24_REG_MASK2);
    if (v != 0xaa55)
        goto failed;
    mrf_write(MRF24_REG_MASK2, 0x55aa);
    v = mrf_read(MRF24_REG_MASK2);
    if (v != 0x55aa)
        goto failed;
    mrf_write(MRF24_REG_MASK2, mask2);

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
    int pin_cs = config->dev_pins[0];
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
    w->pin_irq = config->dev_pins[1];
    w->pin_reset = config->dev_pins[2];
    w->pin_hibernate = config->dev_pins[3];
    if (mrf_detect(w) < 0) {
        printf("mrf%u not found at port %s, pin cs=R%c%d\n",
            unit, spi_name(io), spi_csname(io), spi_cspin(io));
        return 0;
    }
    printf("mrf%u at port %s, pins cs=R%c%d, irq=R%c%d, reset=R%c%d, hibernate=R%c%d\n",
        unit, spi_name(io), spi_csname(io), spi_cspin(io),
        gpio_portname(w->pin_irq), gpio_pinno(w->pin_irq),
        gpio_portname(w->pin_reset), gpio_pinno(w->pin_reset),
        gpio_portname(w->pin_hibernate), gpio_pinno(w->pin_hibernate));

    /* Initialize the chip with interrupts disabled.
     * Extract the MAC address. */
    int s = splimp();
    w->rom_version = mrf_setup();
    mrf_get_mac_address(w->macaddr);
    w->cpid = mrf_profile_create();     /* Create a connection profile */
    splx(s);

    if (w->rom_version == 0) {
        printf("mrf%u: unknown device type, only MRF24WG is supported\n", unit);
        return 0;
    }
    printf("mrf%u: MRF24WG version %x.%x, MAC address %s\n",
        unit, w->rom_version >> 8, w->rom_version & 0xff,
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
