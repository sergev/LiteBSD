/*
 * Network driver for the PIC32 internal Ethernet controller.
 * Several PHY variants are supported:
 *      SMSC LAN8720A
 *      SMSC LAN8740A
 *      SMSC LAN9303
 *      IC+ IP101G
 *
 * Pretty much any RMII PHY should work in 100Mbps full duplex mode.
 * To support other modes, the driver should be modified to be able
 * to detect the result of autonegotiation, which is PHY-depedent.
 * See function is_phy_linked() for details.
 *
 * Copyright (c) 2015, Serge Vakulenko
 *
 * Loosely based on sources of Digilent deIPcK library by Keith Vogel.
 * Copyright (c) 2013-2014, Digilent <www.digilentinc.com>
 *
 * This program is free software; distributed under the terms of
 * BSD 3-clause license.
 */
#include "en.h"
#if NEN > 0

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/syslog.h>

#include <mips/dev/device.h>
#include <vm/vm_param.h>
#include <machine/pte.h>
#include <machine/pic32mz.h>

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

#define ETHER_MAX_LEN       1536
#define RX_PACKETS          4
#define RX_BYTES_PER_DESC   MCLBYTES
#define RX_BYTES            (RX_PACKETS * ETHER_MAX_LEN)
#define RX_DESCRIPTORS      (RX_BYTES / RX_BYTES_PER_DESC)
#define TX_DESCRIPTORS      16

/*
 * DMA buffer descriptor.
 */
typedef struct {
    u_int32_t hdr;              /* Flags */
    u_int32_t paddr;            /* Phys address of data buffer */
    u_int32_t ctl;              /* TX options / RX filter status */
    u_int32_t status;           /* Status */
} eth_desc_t;

/* Start of packet */
#define DESC_SOP(d)             ((d)->hdr & 0x80000000)
#define DESC_SET_SOP(d)         ((d)->hdr |= 0x80000000)

/* End of packet */
#define DESC_EOP(d)             ((d)->hdr & 0x40000000)
#define DESC_SET_EOP(d)         ((d)->hdr |= 0x40000000)

/* Number of data bytes */
#define DESC_BYTECNT(d)         ((d)->hdr >> 16 & 0x7ff)
#define DESC_SET_BYTECNT(d,n)   ((d)->hdr = ((d)->hdr & ~0x7ff0000) | (n) << 16)

/* Next descriptor pointer valid */
#define DESC_SET_NPV(d)         ((d)->hdr |= 0x00000100)
#define DESC_CLEAR_NPV(d)       ((d)->hdr &= ~0x00000100)

/* Eth controller owns this desc */
#define DESC_EOWN(d)            ((d)->hdr & 0x00000080)
#define DESC_SET_EOWN(d)        ((d)->hdr |= 0x00000080)
#define DESC_CLEAR_EOWN(d)      ((d)->hdr &= ~0x00000080)

/* Size of received packet */
#define DESC_FRAMESZ(d)         ((d)->status & 0xffff)

/* Receive filter status */
#define DESC_RXF(d)             ((d)->ctl >> 24)
#define DESC_SET_RXF(d,n)       ((d)->ctl = ((d)->ctl & 0xffffff) | (n) << 24)

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure `netif',
 * which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct eth_port {
    struct arpcom arpcom;           /* Ethernet common part */
#define netif   arpcom.ac_if        /* network-visible interface */
#define macaddr arpcom.ac_enaddr    /* hardware Ethernet address */

    int         is_up;              /* whether the link is up */
    unsigned    multiport_mask;     /* mask of active ports for switch */
    unsigned    receive_index;      /* next RX descriptor to look */
    int         phy_addr;           /* 5-bit PHY address on MII bus */
    int         phy_id;             /* PHY vendor and chip model */
#define PHY_ID_LAN8720A         0x0007c0f0  /* SMSC LAN8720A */
#define PHY_ID_LAN8740A         0x0007c110  /* SMSC LAN8740A */
#define PHY_ID_LAN9303          0x0007c0d0  /* SMSC LAN9303 */
#define PHY_ID_IP101G           0x02430c50  /* IC+ IP101G */

    caddr_t     rx_buf[RX_DESCRIPTORS];     /* receive buffers */
    eth_desc_t  rx_desc[RX_DESCRIPTORS+1];  /* an additional terminating descriptor */

    struct mbuf *tx_packet;                 /* current packet in transmit */
    eth_desc_t  tx_desc[TX_DESCRIPTORS+1];  /* an additional terminating descriptor */

} eth_port[NEN];

/*-------------------------------------------------------------
 * PHY registers.
 */
#define PHY_CONTROL                 0       /* Basic Control Register */
#define PHY_STATUS                  1       /* Basic Status Register */
#define PHY_ID1                     2       /* PHY identifier 1 */
#define PHY_ID2                     3       /* PHY identifier 2 */
#define PHY_ADVRT                   4       /* Auto-negotiation advertisement */

#define PHY_CONTROL_DPLX            0x0100  /* Full duplex */
#define PHY_CONTROL_ANEG_RESTART    0x0200  /* Write 1 to restart autoneg */
#define PHY_CONTROL_ISOLATE         0x0400  /* MII interface disable */
#define PHY_CONTROL_PDN             0x0800  /* Powerdown enable */
#define PHY_CONTROL_ANEG_EN         0x1000  /* Auto-negotiation enable */
#define PHY_CONTROL_SPEED_100       0x2000  /* Select 100 Mbps speed */
#define PHY_CONTROL_LPBK            0x4000  /* Loopback enable */
#define PHY_CONTROL_RESET           0x8000  /* Reset, bit self cleared */
#define PHY_CONTROL_BITS "\20"\
"\11dplx\12aneg-rst\13isolate\14pdn\15aneg-en\16speed100\17lpbk\20rst"

#define PHY_STATUS_EXCAP            0x0001  /* Extended capabilities regs present */
#define PHY_STATUS_JAB              0x0002  /* Jabber detected */
#define PHY_STATUS_LINK             0x0004  /* Link valid */
#define PHY_STATUS_CAP_ANEG         0x0008  /* Auto-negotiation available */
#define PHY_STATUS_REM_FLT          0x0010  /* Remote fault detected */
#define PHY_STATUS_ANEG_ACK         0x0020  /* Auto-negotiation acknowledge */
#define PHY_STATUS_EXSTATUS         0x0100  /* Extended status reg present */
#define PHY_STATUS_CAP_100T2_HDX    0x0200  /* Can do 100Base-T2 half duplex */
#define PHY_STATUS_CAP_100T2_FDX    0x0400  /* Can do 100Base-T2 full duplex */
#define PHY_STATUS_CAP_10_HDX       0x0800  /* Can do 10Base-TX half duplex */
#define PHY_STATUS_CAP_10_FDX       0x1000  /* Can do 10Base-TX full duplex */
#define PHY_STATUS_CAP_100_HDX      0x2000  /* Can do 100Base-TX half duplex */
#define PHY_STATUS_CAP_100_FDX      0x4000  /* Can do 100Base-TX full duplex */
#define PHY_STATUS_CAP_100_T4       0x8000  /* Can do 100Base-T4 */
#define PHY_STATUS_BITS "\20"\
"\1exreg\2jab\3link\4cap-aneg\5rem-flt\6aneg-ack"\
"\14hdx10\15fdx10\16hdx100\17fdx100\20t4-100"

#define PHY_ADVRT_CSMA              0x0001  /* Capable of 802.3 CSMA operation */
#define PHY_ADVRT_10_HDX            0x0020  /* Can do 10Base-TX half duplex */
#define PHY_ADVRT_10_FDX            0x0040  /* Can do 10Base-TX full duplex */
#define PHY_ADVRT_100_HDX           0x0080  /* Can do 100Base-TX half duplex */
#define PHY_ADVRT_100_FDX           0x0100  /* Can do 100Base-TX full duplex */
#define PHY_ADVRT_RF                0x2000  /* Remote fault */
#define PHY_ADVRT_BITS "\20"\
"\1csma\6hdx10\7fdx10\10hdx100\11fdx100\16rf"

/*
 * Register #31 for SMSC LAN8720A, LAN8740A.
 */
#define PHY_LAN87x0A_AUTODONE       0x1000  /* Auto-negotiation is done */
#define PHY_LAN87x0A_4B5B           0x0040  /* Enable 4B5B encoding */
#define PHY_LAN87x0A_FDX            0x0010  /* Full duplex */
#define PHY_LAN87x0A_100            0x0008  /* Speed 100 Mbps */
#define PHY_LAN87x0A_10             0x0004  /* Speed 10 Mbps */
#define PHY_LAN87x0A_BITS "\20"\
"\3speed10\4speed100\5fdx\15autodone"

/*
 * Page 16, register #30 for IC+ IP101G.
 */
#define PHY_IP101G_LINK_UP          0x0100  /* Link status is OK */
#define PHY_IP101G_FORCE_MDIX       0x0008  /* Force MDIX mode */
#define PHY_IP101G_FDX              0x0004  /* Full duplex */
#define PHY_IP101G_100              0x0002  /* Speed 100 Mbps */
#define PHY_IP101G_10               0x0001  /* Speed 10 Mbps */
#define PHY_IP101G_BITS "\20"\
"\1speed10\2speed100\3fdx\4mdix\11linkup"

/*
 * Register #31 for virtual or physical PHY of SMSC LAN9393.
 */
#define PHY_LAN9303_AUTODONE        0x1000  /* Auto-negotiation is done */
#define PHY_LAN9303_LOOPBACK        0x4000  /* Switch loopback port 0 */
#define PHY_LAN9303_AUTODONE        0x1000  /* Auto-negotiation is done */
#define PHY_LAN9303_TURBO           0x0400  /* 200Mbps data rate */
#define PHY_LAN9303_MODE_MAC        0x0000  /* MII MAC mode */
#define PHY_LAN9303_MODE_MII_PHY    0x0100  /* MII PHY mode */
#define PHY_LAN9303_MODE_RMII_PHY   0x0200  /* RMII PHY mode */
#define PHY_LAN9303_COLL_TEST       0x0080  /* Switch collision test port 0 */
#define PHY_LAN9303_OUTCLK          0x0040  /* OUTCLK signal as output */
#define PHY_LAN9303_CLK_STRENGTH    0x0020  /* High current clock signal */
#define PHY_LAN9303_FDX             0x0010  /* Full duplex */
#define PHY_LAN9303_100             0x0008  /* Speed 100 Mbps */
#define PHY_LAN9303_10              0x0004  /* Speed 10 Mbps */
#define PHY_LAN9303_SQEOFF          0x0001  /* SQE test disable */
#define PHY_LAN9303_BITS "\20"\
"\1sqeoff\3speed10\4speed100\5fdx\6clkstrength\7outclk"\
"\10colltest\11mii_phy\12rmii_phy\13turbo\15autodone\17loopback"

/*
 * Read PHY register.
 * Return -1 when failed.
 */
static int phy_read(int phy_addr, int reg_num, unsigned msec)
{
    unsigned time_start = mfc0_Count();
    unsigned timeout = msec * CPU_KHZ / 2;

    /* Clear any commands. */
    EMAC1MCMD = 0;
    while (EMAC1MIND & PIC32_EMAC1MIND_MIIMBUSY) {
        if (mfc0_Count() - time_start > timeout) {
            return -1;
        }
    }

    EMAC1MADR = PIC32_EMAC1MADR(phy_addr, reg_num);
    EMAC1MCMDSET = PIC32_EMAC1MCMD_READ;
    udelay(1);

    /* Wait to finish. */
    time_start = mfc0_Count();
    while (EMAC1MIND & PIC32_EMAC1MIND_MIIMBUSY) {
        if (mfc0_Count() - time_start > timeout) {
            EMAC1MCMD = 0;
            return -1;
        }
    }

    EMAC1MCMD = 0;
    return EMAC1MRDD & 0xffff;
}

/*
 * Scan PHY register for expected value.
 * Return -1 when failed.
 */
static int phy_scan(int phy_addr, int reg_num,
    int scan_mask, int expected_value, unsigned msec)
{
    unsigned time_start = mfc0_Count();
    unsigned timeout = msec * CPU_KHZ / 2;

    /* Clear any commands. */
    EMAC1MCMD = 0;
    while (EMAC1MIND & PIC32_EMAC1MIND_MIIMBUSY) {
        if (mfc0_Count() - time_start > timeout) {
            return -1;
        }
    }

    /* Scan the PHY until it is ready. */
    EMAC1MADR = PIC32_EMAC1MADR(phy_addr, reg_num);
    EMAC1MCMDSET = PIC32_EMAC1MCMD_SCAN;
    udelay(1);

    /* Wait for it to become valid. */
    time_start = mfc0_Count();
    while (EMAC1MIND & PIC32_EMAC1MIND_NOTVALID) {
        if (mfc0_Count() - time_start > timeout) {
            return -1;
        }
    }

    /* Wait until we hit our mask. */
    time_start = mfc0_Count();
    while (((EMAC1MRDD & scan_mask) == scan_mask) != expected_value) {
        if (mfc0_Count() - time_start > timeout) {
            return -1;
        }
    }

    /* Kill the scan. */
    EMAC1MCMD = 0;
    udelay(1);
    time_start = mfc0_Count();
    while (EMAC1MIND & PIC32_EMAC1MIND_MIIMBUSY) {
        if (mfc0_Count() - time_start > timeout) {
            return -1;
        }
    }
    return 0;
}

/*
 * Write PHY register.
 * Return -1 when failed.
 */
static int phy_write(int phy_addr, int reg_num, int value, unsigned msec)
{
    unsigned time_start = mfc0_Count();
    unsigned timeout = msec * CPU_KHZ / 2;

    /* Clear any commands. */
    EMAC1MCMD = 0;
    while (EMAC1MIND & PIC32_EMAC1MIND_MIIMBUSY) {
        if (mfc0_Count() - time_start > timeout) {
            return -1;
        }
    }

    EMAC1MADR = PIC32_EMAC1MADR(phy_addr, reg_num);
    EMAC1MWTD = value;
    udelay(1);

    /* Wait to finish. */
    time_start = mfc0_Count();
    while (EMAC1MIND & PIC32_EMAC1MIND_MIIMBUSY) {
        if (mfc0_Count() - time_start > timeout) {
            return -1;
        }
    }
    return 0;
}

/*
 * Reset the PHY via MIIM interface.
 * Return -1 on failure.
 */
static int phy_reset(int phy_addr)
{
    int advrt;
    int advertise_all = PHY_ADVRT_10_HDX | PHY_ADVRT_10_FDX |
        PHY_ADVRT_100_HDX | PHY_ADVRT_100_FDX;

    /* Check ADVRT register is writable. */
    phy_write(phy_addr, PHY_ADVRT, 0, 100);
    advrt = phy_read(phy_addr, PHY_ADVRT, 1);
    if (advrt & advertise_all)
        return -1;
    phy_write(phy_addr, PHY_ADVRT, PHY_ADVRT_CSMA | advertise_all, 100);
    advrt = phy_read(phy_addr, PHY_ADVRT, 1);
    if ((advrt & advertise_all) != advertise_all)
        return -1;

    /* Send a reset to the PHY. */
    if (phy_write(phy_addr, PHY_CONTROL, PHY_CONTROL_RESET, 100) < 0)
        return -1;

    /* Wait for the reset pin to autoclear. */
    if (phy_scan(phy_addr, PHY_CONTROL, PHY_CONTROL_RESET, 0, 500) < 0)
        return -1;

    /* Advertise both 100Mbps and 10Mbps modes, full or half duplex. */
    phy_write(phy_addr, PHY_ADVRT, PHY_ADVRT_CSMA | advertise_all, 100);

    /* Restart autonegotiation. */
    phy_write(phy_addr, PHY_CONTROL, PHY_CONTROL_ANEG_EN | PHY_CONTROL_ANEG_RESTART, 100);
    return 0;
}

/*
 * Get the speed and duplex mode of LAN87x0A chip.
 */
static void phy_lan87x0a_poll(int phy_addr, int *speed_100, int *full_duplex)
{
    /* Read 87x0A-specific register #31. */
    int special = phy_read(phy_addr, 31, 1);

    if (special & PHY_LAN87x0A_AUTODONE) {
        /* Auto-negotiation is done - get the speed. */
        *speed_100 = (special & PHY_LAN87x0A_100) != 0;
        *full_duplex = (special & PHY_LAN87x0A_FDX) != 0;
    }
#if 0
    int special_modes = phy_read(phy_addr, 18, 1);
    int error_counter = phy_read(phy_addr, 26, 1);
    int cable_length  = phy_read(phy_addr, 28, 1);
    printf("     SPECIAL=%b\n", special, PHY_LAN87x0A_BITS);
    printf("     Special_Modes=%04x, Error_Counter=%04x, Cable_Length=%04x\n",
        special_modes, error_counter, cable_length);
#endif
}

/*
 * Get the speed and duplex mode of IP101G chip.
 */
static void phy_ip101g_poll(int phy_addr, int *speed_100, int *full_duplex)
{
    /* Read ip101g-specific register #30, page 16. */
    int special = phy_read(phy_addr, 30, 1);

    if (special & PHY_IP101G_LINK_UP) {
        /* Auto-negotiation is done - get the speed. */
        *speed_100 = (special & PHY_IP101G_100) != 0;
        *full_duplex = (special & PHY_IP101G_FDX) != 0;
    }
#if 0
    printf("     SPECIAL=%b\n", special, PHY_IP101G_BITS);
#endif
}

/*
 * LAN9303 switch: get the mask of active ports.
 */
static int phy_lan9303_poll(struct eth_port *e)
{
    /* Read status of physical PHYs, register #31. */
    int special1 = phy_read(e->phy_addr + 1, 31, 1);
    int special2 = phy_read(e->phy_addr + 2, 31, 1);
    int mask = 0;

    if (special1 & PHY_LAN87x0A_AUTODONE) {
        mask |= 1;
        if (! (e->multiport_mask & 1))
            log(LOG_ERR, "en0: port 1 up, %s, %s duplex\n",
                (special1 & PHY_LAN87x0A_100) ? "100Mbps" : "10Mbps",
                (special1 & PHY_LAN87x0A_FDX) ? "full" : "half");
    }
    if (special2 & PHY_LAN87x0A_AUTODONE) {
        mask |= 2;
        if (! (e->multiport_mask & 2))
            log(LOG_ERR, "en0: port 2 up, %s, %s duplex\n",
                (special2 & PHY_LAN87x0A_100) ? "100Mbps" : "10Mbps",
                (special2 & PHY_LAN87x0A_FDX) ? "full" : "half");
    }
#if 0
    printf("     port1 SPECIAL=%b\n", special1, PHY_LAN9303_BITS);
    printf("     port1 SPECIAL=%b\n", special2, PHY_LAN9303_BITS);
#endif
    return mask;
}

/*
 * Determine whether the link is up.
 * When up, get the speed and duplex mode.
 */
static int is_phy_linked(struct eth_port *e, int *speed_100, int *full_duplex)
{
    int status = phy_read(e->phy_addr, PHY_STATUS, 1);
    if (status < 0)
        return 0;
    //printf("--- %s: STATUS=%b\n", __func__, status, PHY_STATUS_BITS);

    if (! (status & PHY_STATUS_LINK))           /* Is link up? */
        return 0;
    if (! (status & PHY_STATUS_ANEG_ACK))       /* Is auto-negotiation done? */
        return 0;

    switch (e->phy_id) {
    case PHY_ID_LAN8720A:
    case PHY_ID_LAN8740A:
        phy_lan87x0a_poll(e->phy_addr, speed_100, full_duplex);
        break;

    case PHY_ID_IP101G:
        phy_ip101g_poll(e->phy_addr, speed_100, full_duplex);
        break;

    case PHY_ID_LAN9303:
        /* Get mask of active ports. */
        e->multiport_mask = phy_lan9303_poll(e);
        if (e->multiport_mask == 0)
            return 0;

        /* Virtual PHY is always 100Mbit/sec full duplex. */
        *speed_100 = 1;
        *full_duplex = 1;
        break;

    default:
        /* Unknown PHY: assume 100Mbit/sec full duplex. */
        *speed_100 = 1;
        *full_duplex = 1;
        break;
    }
    return 1;
}

/*
 * Initialize the Ethernet Controller.
 */
static void en_setup()
{
    /* Disable the ethernet interrupt. */
    IECCLR(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);

    /* Turn the Ethernet cotroller off. */
    ETHCON1CLR = PIC32_ETHCON1_ON | PIC32_ETHCON1_RXEN | PIC32_ETHCON1_TXRTS;

    /* Wait for abort to finish. */
    while (ETHSTAT & PIC32_ETHSTAT_ETHBUSY)
        continue;

    /* Turn on the ethernet controller. */
    ETHCON1 = PIC32_ETHCON1_ON;

    /* Clear the interrupt flag bit. */
    IFSCLR(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);

    /* Clear interrupts. */
    ETHIEN = 0;
    ETHIRQ = 0;

    /* Clear discriptor pointers; for now. */
    ETHTXST = 0;
    ETHRXST = 0;

    /* Enable auto flow control. */
    //ETHCON1SET = PIC32_ETHCON1_PTV(1);  /* the max number of pause timeouts */
    //ETHCON1SET = PIC32_ETHCON1_AUTOFC;

    /* High and low watermarks. */
    int empty_watermark = ETHER_MAX_LEN / RX_BYTES_PER_DESC;
    int full_watermark  = RX_DESCRIPTORS - (ETHER_MAX_LEN * 2) / RX_BYTES_PER_DESC;
    ETHRXWM = PIC32_ETHRXWM_FWM(full_watermark) |
              PIC32_ETHRXWM_EWM(empty_watermark);

    /* Set RX descriptor buffer size in bytes (aligned to 16 bytes). */
    ETHCON2 = RX_BYTES_PER_DESC;

    /* Set our Rx filters. */
    ETHRXFC = PIC32_ETHRXFC_CRCOKEN |   /* enable checksum filter */
              PIC32_ETHRXFC_RUNTEN |    /* enable short packets */
              PIC32_ETHRXFC_UCEN |      /* enable unicast filter */
              PIC32_ETHRXFC_BCEN;       /* enable broadcast filter */

    /* Hash table, not used. */
    ETHHT0 = 0;
    ETHHT1 = 0;

    /* Pattern match, not used. */
    ETHPMM1 = 0;
    ETHPMM1 = 0;

    /* Byte in TCP like checksum pattern calculation. */
    ETHPMCS = 0;
}

/*
 * Initialize the MAC.
 */
static void en_setup_mac()
{
    /* Reset the MAC. */
    EMAC1CFG1 = PIC32_EMAC1CFG1_SOFTRESET;
    udelay(1);

    /* Pull it out of reset. */
    EMAC1CFG1 = 0;
    udelay(1);
    EMAC1CFG1 = PIC32_EMAC1CFG1_RXENABLE |  /* Receive enable */
                PIC32_EMAC1CFG1_TXPAUSE |   /* MAC TX flow control */
                PIC32_EMAC1CFG1_RXPAUSE;    /* MAC RX flow control */

    EMAC1CFG2 = PIC32_EMAC1CFG2_PADENABLE | /* Pad/CRC enable */
                PIC32_EMAC1CFG2_CRCENABLE | /* CRC enable */
                PIC32_EMAC1CFG2_EXCESSDFR |
                PIC32_EMAC1CFG2_AUTOPAD |
                PIC32_EMAC1CFG2_LENGTHCK;

    EMAC1MAXF = 6*256;                      /* max frame size in bytes */
    EMAC1IPGR = PIC32_EMAC1IPGR(12, 18);    /* non-back-to-back interpacket gap */
    EMAC1CLRT = PIC32_EMAC1CLRT(55, 15);    /* collision window/retry limit */
}

/*
 * Initialize RMII and MIIM.
 */
static void en_setup_mii()
{
    EMAC1SUPP = PIC32_EMAC1SUPP_RESETRMII;  /* reset RMII */
    udelay(1);
    EMAC1SUPP = 0;

    EMAC1MCFG = PIC32_EMAC1MCFG_RESETMGMT;  /* reset the management fuctions */
    udelay(1);
    EMAC1MCFG = 0;

    /* The IEEE 802.3 spec says no faster than 2.5MHz.
     * 80 / 40 = 2MHz
     */
    EMAC1MCFG = PIC32_EMAC1MCFG_CLKSEL_40;
}

/*
 * Set DMA descriptors.
 */
static void en_setup_dma(struct eth_port *e)
{
    int i;

    /* Set Rx discriptor list.
     * All owned by the ethernet controller. */
    bzero(e->rx_desc, sizeof(e->rx_desc));
    for (i=0; i<RX_DESCRIPTORS; i++) {
        MCLALLOC(e->rx_buf[i], M_WAIT);
        if (! e->rx_buf[i])
            panic("en: failed to allocate RX buffer\n");

        DESC_SET_EOWN(&e->rx_desc[i]);
        DESC_CLEAR_NPV(&e->rx_desc[i]);
        e->rx_desc[i].paddr = kvtophys(e->rx_buf[i]);
    }

    /* Loop the list back to the begining.
     * This is a circular array descriptor list. */
    e->rx_desc[RX_DESCRIPTORS].hdr = MACH_VIRT_TO_PHYS(e->rx_desc);
    DESC_SET_NPV(&e->rx_desc[RX_DESCRIPTORS-1]);

    /* Set RX at the start of the list. */
    e->receive_index = 0;
    ETHRXST = MACH_VIRT_TO_PHYS(&e->rx_desc[0]);

    /* Set up the transmitt descriptors all owned by
     * the software; clear it completely out. */
    bzero(e->tx_desc, sizeof(e->tx_desc));
    ETHTXST = MACH_VIRT_TO_PHYS(e->tx_desc);
}

/*
 * Free DMA resources.
 */
static void en_clear_dma(struct eth_port *e)
{
    int i;

    /* Deallocate all RX buffers. */
    for (i=0; i<RX_DESCRIPTORS; i++) {
        if (e->rx_buf[i]) {
            MCLFREE(e->rx_buf[i]);
            e->rx_buf[i] = 0;
        }
    }
}

/*
 * Setup output on interface.
 * Get another datagram to send off of the interface queue,
 * and map it to the interface before starting the output.
 * called only at splimp or interrupt level.
 */
static void
en_start(ifp)
    struct ifnet *ifp;
{
    struct eth_port *e = &eth_port[ifp->if_unit];
    eth_desc_t *desc;
    struct mbuf *m;

    /* Link not ready yet. */
    if (! e->is_up)
        return;

    /* The previous transmit has not completed yet. */
    if (e->tx_packet)
        return;

    /* Interface is administratively deactivated. */
    if (! (e->netif.if_flags & IFF_RUNNING))
        return;
again:
    /* Get a packet from the transmit queue. */
    IF_DEQUEUE(&e->netif.if_snd, m);
    if (m == 0)
        return;
#if NBPFILTER > 0
    /*
     * If bpf is listening on this interface, let it
     * see the packet before we commit it to the wire.
     */
    if (ifp->if_bpf)
        bpf_mtap(ifp->if_bpf, m);
#endif
    /*
     * Setup transmit descriptors.
     */
    e->tx_packet = m;
    for (desc=e->tx_desc; m!=0; m=m->m_next) {
        if (desc >= &e->tx_desc[TX_DESCRIPTORS]) {
            e->netif.if_oerrors++;
            m_freem(m);
            log(LOG_ERR, "en%d: too many fragments in transmit packet\n",
                ifp->if_unit);
            goto again;
        }
        if (m->m_len == 0)
            continue;

        desc->hdr = 0;
        desc->paddr = kvtophys(mtod(m, caddr_t));
        DESC_SET_BYTECNT(desc, m->m_len);
        if (desc == e->tx_desc)
            DESC_SET_SOP(desc);
        DESC_SET_EOWN(desc);
        //printf("--- tx desc%u = %08x %08x (%u bytes)\n", desc - e->tx_desc, desc->hdr, desc->paddr, m->m_len);
        desc++;
    }
    DESC_SET_EOP(desc - 1);

    /* Set the descriptor table to be transmitted. */
    ETHTXST = MACH_VIRT_TO_PHYS(e->tx_desc);

    /* Start transmitter. */
    ETHCON1SET = PIC32_ETHCON1_TXRTS;
}

/*
 * Initialization of interface; set up initialization block
 * and transmit/receive descriptor rings.
 */
static void
en_init(unit)
    int unit;
{
    struct eth_port *e = &eth_port[unit];
    struct ifnet *ifp = &e->netif;
    int s;

    if (ifp->if_addrlist == (struct ifaddr *)0)
        return;
    //printf("--- %s\n", __func__);

    if (! (ifp->if_flags & IFF_RUNNING)) {
        /* Enable transmitter. */
        s = splimp();
        e->netif.if_flags |= IFF_RUNNING;
        en_start(ifp);
        splx(s);
    }
}

/*
 * Reset of interface.
 */
static void
en_reset(unit)
    int unit;
{
    struct eth_port *e = &eth_port[unit];

    if (unit >= NEN)
        return;
    printf("en%d: reset\n", unit);
    if (e->tx_packet) {
        m_freem(e->tx_packet);
        e->tx_packet = 0;
    }
    en_init(unit);
}

/*
 * Ethernet interface receiver interface.
 * Decapsulate packet based on type and pass to type specific
 * higher-level input routine.
 */
static void
en_recv(e)
    struct eth_port *e;
{
    /* Get the size of received Ethernet packet. */
    eth_desc_t *desc = &e->rx_desc[e->receive_index];
    unsigned frame_size = DESC_FRAMESZ(desc);
    unsigned read_nbytes = 0;
    struct mbuf *m = 0, **tail = 0;
    struct ether_header eth_header;

    if (frame_size <= 0) {
        /* Cannot happen. */
        printf("en_recv: bad frame size = %u\n", frame_size);
        return;
    }

    int start_of_packet = DESC_SOP(desc);
    if (! start_of_packet) {
        /* Start of packet is expected. */
        printf("en_recv: no SOP flag (%x)\n", desc->hdr);
    }
#if NBPFILTER > 0
    /* Get receive filter status. */
    int rxf = DESC_RXF(desc);

#ifndef MULTICAST
    /* We don't accept multicast. */
    rxf &= ~0x80;
#endif
#endif

    while (read_nbytes < frame_size) {
        int end_of_packet = DESC_EOP(desc);
        int buf_index = desc - e->rx_desc;
        caddr_t vaddr = e->rx_buf[buf_index];
        unsigned nbytes = DESC_BYTECNT(desc);

        if (DESC_EOWN(desc)) {
            printf("en_recv: unexpected EOWN flag (desc %04x = %x)\n",
                MACH_VIRT_TO_PHYS(desc), desc->hdr);
            break;
        }
        if (nbytes > frame_size - read_nbytes)
            nbytes = frame_size - read_nbytes;
#if 0
        unsigned char *p = (unsigned char*) vaddr;
        int i;
        printf("--- %u bytes from descriptor %04x: %02x",
            nbytes, MACH_VIRT_TO_PHYS(desc), p[0]);
        for (i=1; i<nbytes; i++)
            printf("-%02x", p[i]);
        printf("\n");
#endif
        if (read_nbytes == 0 && start_of_packet) {
            /* Allocate packet header. */
            MGETHDR(m, M_DONTWAIT, MT_DATA);
            m->m_pkthdr.rcvif = &e->netif;
            m->m_pkthdr.len = frame_size - sizeof(eth_header);
            m->m_len = 0;

            /* Store Ethernet header separately. */
            bcopy(vaddr, &eth_header, sizeof(eth_header));
            nbytes -= sizeof(eth_header);
            read_nbytes += sizeof(eth_header);
            vaddr += sizeof(eth_header);
        }

        /*
         * Copy data from rx buffer to mbuf chain.
         */
        if (m && m->m_len == 0 && m->m_pkthdr.len > MHLEN) {
            /* First mbuf chunk is large - allocate a cluster. */
            MCLGET(m, M_DONTWAIT);
            if (! (m->m_flags & M_EXT)) {
                /* Cannot allocate cluster - ignore the packet. */
                m_free(m);
                m = 0;
            }
        }

        if (m) {
            if (m->m_len == 0) {
                if (end_of_packet) {
                    /* Single buffer - copy to mbuf. */
                    m->m_len = m->m_pkthdr.len;
                } else {
                    /* First chunk of mbuf chain. */
                    m->m_len = nbytes;
                }
                bcopy(vaddr, mtod(m, caddr_t), m->m_len);
                tail = &m->m_next;
            } else {
                /* Append rx buffer to mbuf chain. */
                struct mbuf *n;
                MGET(n, M_DONTWAIT, MT_DATA);
                if (! n) {
                    m_freem(m);
                    m = 0;
                } else {
                    /* Allocate new rx buffer. */
                    MCLALLOC(e->rx_buf[buf_index], M_NOWAIT);
                    if (! e->rx_buf[buf_index]) {
                        /* Failed to allocate RX buffer. */
                        m_freem(m);
                        m_freem(n);
                        m = 0;
                        e->rx_buf[buf_index] = vaddr;
                    } else {
                        /* Move current rx buffer to mbuf. */
                        n->m_ext.ext_buf = vaddr;
                        n->m_data = vaddr;
                        n->m_flags |= M_EXT;
                        n->m_ext.ext_size = MCLBYTES;
                        n->m_len = nbytes;
                        *tail = n;
                        tail = &n->m_next;
                    }
                    desc->paddr = kvtophys(e->rx_buf[buf_index]);
                }
            }
        }
        read_nbytes += nbytes;

        /* Free the receive descriptor. */
        DESC_SET_EOWN(desc);                        /* give up ownership */
        ETHCON1SET = PIC32_ETHCON1_BUFCDEC;         /* decrement the BUFCNT */
        e->receive_index++;                         /* next descriptor */
        if (e->receive_index >= RX_DESCRIPTORS)
            e->receive_index = 0;
        desc = &e->rx_desc[e->receive_index];
    }

    if (! m) {
        /* Damaged packet: ignore. */
        e->netif.if_iqdrops++;
        return;
    }

#if NBPFILTER > 0
    /*
     * Check if there's a bpf filter listening on this interface.
     * If so, hand off the raw packet to enet.
     */
    if (e->netif.if_bpf) {
        bpf_mtap(e->netif.if_bpf, m);

        if (ETHRXFC & PIC32_ETHRXFC_NOTMEEN) {
            /* In promisc mode, use RXF_RSV field of descriptor
             * to filter broadcast/multicast/unicast packets.
             * Keep the packet if it's a broadcast or has our
             * physical ethernet address (or if we support
             * multicast and it's one). */
            if (! (rxf & 0xe0)) {
                m_freem(m);
                return;
            }
        }
    }
#endif
    eth_header.ether_type = ntohs(eth_header.ether_type);
    ether_input(&e->netif, &eth_header, m);
}

/*
 * Timeout routine.
 */
static void
en_watchdog(unit)
    int unit;
{
    struct eth_port *e = &eth_port[unit];
    int receiver_enabled = (ETHCON1 & PIC32_ETHCON1_RXEN);
    int speed_100 = 1, full_duplex = 1;

    /* Call it every second. */
    e->netif.if_timer = 1;

    /* Poll whether the link is active.
     * Get speed and duplex status from the PHY. */
    e->is_up = is_phy_linked(e, &speed_100, &full_duplex);

    if (! (e->netif.if_flags & IFF_RUNNING))
        return;

    /* Check whether RX is enabled. */
    if (e->is_up && ! receiver_enabled) {
        /* Link activated. */
        if (! e->multiport_mask)
            log(LOG_ERR, "en0: link up, %s, %s duplex\n",
                speed_100 ? "100Mbps" : "10Mbps",
                full_duplex ? "full" : "half");

        /* Set speed. */
        if (speed_100) {
            EMAC1SUPPSET = PIC32_EMAC1SUPP_SPEEDRMII;
        } else {
            EMAC1SUPPCLR = PIC32_EMAC1SUPP_SPEEDRMII;
        }

        /* Set duplex. */
        if (full_duplex) {
            EMAC1CFG2SET = PIC32_EMAC1CFG2_FULLDPLX;
        } else {
            EMAC1CFG2CLR = PIC32_EMAC1CFG2_FULLDPLX;
        }

        /* Set gap size. */
        EMAC1IPGT = full_duplex ? 21 : 18;

        /* Enable receiver. */
        en_setup_dma(e);
        ETHCON1SET = PIC32_ETHCON1_RXEN;
        //printf("--- %s: RXEN\n", __func__);
    }
    else if (! e->is_up && receiver_enabled) {
        /* Link down. */
        log(LOG_ERR, "en0: link down\n");

        /* Disable the receiver. */
        ETHCON1CLR = PIC32_ETHCON1_RXEN;
        while (ETHSTAT & PIC32_ETHSTAT_RXBUSY)
            continue;

        en_clear_dma(e);
    }
}

/*
 * Controller interrupt.
 */
void
enintr(dev)
    dev_t dev;
{
    int unit = minor(dev);
    struct eth_port *e = &eth_port[unit];
    int irq;

    /* Read the interrupt requests and acknowledge the interrupt. */
    irq = ETHIRQ;
    ETHIRQCLR = irq;
    //printf("--- %s: irq=%04x\n", __func__, irq);

    /* Receiver error. */
    if (irq & (PIC32_ETHIRQ_RXBUSE |
               PIC32_ETHIRQ_RXBUFNA |
               PIC32_ETHIRQ_RXOVFLW)) {
        e->netif.if_ierrors++;
        log(LOG_ERR, "en%d: receive error: irq %x\n", unit, irq);
    }

    if (ETHCON1 & PIC32_ETHCON1_RXEN) {
        /* Receiver enabled. */
        while (! DESC_EOWN(&e->rx_desc[e->receive_index])) {
            /* We received something. */
            e->netif.if_ipackets++;
            en_recv(e);
        }
    }

    /* Transmit error. */
    if (irq & (PIC32_ETHIRQ_TXBUSE | PIC32_ETHIRQ_TXABORT)) {
        log(LOG_ERR, "en%d: transmit error: irq %x\n", unit, irq);
        e->netif.if_oerrors++;
    }

    /* Packet Transmitted */
    if (irq & PIC32_ETHIRQ_TXDONE) {
        ++e->netif.if_opackets;
        e->netif.if_collisions += ETHSCOLFRM + ETHMCOLFRM;
    }

    /* Transmitter is idle. */
    if (! (ETHCON1 & PIC32_ETHCON1_TXRTS)) {
        if (e->tx_packet) {
            m_freem(e->tx_packet);
            e->tx_packet = 0;
        }
        if (e->is_up) {
            /* Any more to send? */
            en_start(&e->netif);
        }
    }

    /* Clear the interrupt flag on exit from the service routine. */
    IFSCLR(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);
}

/*
 * Process an ioctl request.
 */
static int
en_ioctl(ifp, cmd, data)
    struct ifnet *ifp;
    int cmd;
    caddr_t data;
{
    struct ifaddr *ifa = (struct ifaddr *)data;
    int s, error = 0;

    s = splimp();
    switch (cmd) {

    case SIOCSIFADDR:
        /* Change IP address. */
        ifp->if_flags |= IFF_UP;
        switch (ifa->ifa_addr->sa_family) {
#ifdef INET
        case AF_INET:
            en_init(ifp->if_unit);   /* before arpwhohas */
            ((struct arpcom *)ifp)->ac_ipaddr = IA_SIN(ifa)->sin_addr;
            arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
            break;
#endif
        default:
            en_init(ifp->if_unit);
            break;
        }
        break;

    case SIOCSIFFLAGS:
        /* Start/stop the network interface. */
        if (! (ifp->if_flags & IFF_UP) &&
            (ifp->if_flags & IFF_RUNNING))
        {
            ETHCON1CLR = PIC32_ETHCON1_TXRTS;
            while (ETHSTAT & PIC32_ETHSTAT_TXBUSY)
                continue;

            /* Disable receiver. */
            while (ETHSTAT & PIC32_ETHSTAT_RXBUSY)
                continue;
            ETHCON1CLR = PIC32_ETHCON1_RXEN;
            while (ETHSTAT & PIC32_ETHSTAT_RXBUSY)
                continue;

            ifp->if_flags &= ~IFF_RUNNING;
            en_clear_dma(&eth_port[ifp->if_unit]);
        }
        else if ((ifp->if_flags & IFF_UP) &&
            ! (ifp->if_flags & IFF_RUNNING))
        {
            en_init(ifp->if_unit);
        }
        break;

#ifdef notdef
    case SIOCGHWADDR:
        /* Change MAC address. */
        struct ifreq *ifr = (struct ifreq *)data;
        u_char *addr = (u_char*) &ifr->ifr_data;
        struct eth_port *e = &eth_port[ifp->if_unit];
        e->macaddr[0] = addr[0];
        e->macaddr[1] = addr[1];
        e->macaddr[2] = addr[2];
        e->macaddr[3] = addr[3];
        e->macaddr[4] = addr[4];
        e->macaddr[5] = addr[5];
        EMAC1SA2 = e->macaddr[0] | (e->macaddr[1] << 8);
        EMAC1SA1 = e->macaddr[2] | (e->macaddr[3] << 8);
        EMAC1SA0 = e->macaddr[4] | (e->macaddr[5] << 8);
        break;
#endif

    default:
        error = EINVAL;
    }
    splx(s);
    return error;
}

/*
 * Different devices can have different pin assignments,
 * depending on pin count and DEVCFG.FETHIO configuration setting.
 */
static void setup_signals()
{
    switch (DEVID & 0x0fffffff) {
    case 0x05104053:            /* MZ2048ECG064 */
    case 0x05109053:            /* MZ2048ECH064 */
    case 0x05131053:            /* MZ2048ECM064 */
    case 0x07203053:            /* MZ1024EFG064 */
    case 0x07204053:            /* MZ2048EFG064 */
    case 0x07208053:            /* MZ1024EFH064 */
    case 0x07209053:            /* MZ2048EFH064 */
    case 0x07230053:            /* MZ1024EFM064 */
    case 0x07231053:            /* MZ2048EFM064 */
        if (*(unsigned*)DEVCFG3 & DEVCFG3_FETHIO) {
            /*
             * Default setup for 64-pin device.
             */
            ANSELECLR = 1 << 4;         /* Disable analog pad on RE4 for ERXERR */
            ANSELECLR = 1 << 6;         /* Disable analog pad on RE6 for ETXD0 */
            ANSELECLR = 1 << 7;         /* Disable analog pad on RE7 for ETXD1 */
            ANSELECLR = 1 << 5;         /* Disable analog pad on RE5 for ETXEN */
            ANSELBCLR = 1 << 15;        /* Disable analog pad on RB15 for EMDC */

            LATECLR = 1 << 6; TRISECLR = 1 << 6;    /* set RE6 as output for ETXD0 */
            LATECLR = 1 << 7; TRISECLR = 1 << 7;    /* set RE7 as output for ETXD1 */
            LATECLR = 1 << 5; TRISECLR = 1 << 5;    /* set RE5 as output for ETXEN */
        } else {
            /*
             * Alternative setup for 64-pin device.
             */
            ANSELBCLR = 1 << 15;        /* Disable analog pad on RB15 for AEMDC */

            LATFCLR = 1 << 1; TRISFCLR = 1 << 1;    /* set RF1 as output for AETXD0 */
            LATFCLR = 1 << 0; TRISFCLR = 1 << 0;    /* set RF0 as output for AETXD1 */
            LATDCLR = 1 << 2; TRISDCLR = 1 << 2;    /* set RD2 as output for AETXEN */
        }
        break;

    case 0x0510E053:            /* MZ2048ECG100 */
    case 0x05113053:            /* MZ2048ECH100 */
    case 0x0513B053:            /* MZ2048ECM100 */
    case 0x0720D053:            /* MZ1024EFG100 */
    case 0x0720E053:            /* MZ2048EFG100 */
    case 0x07212053:            /* MZ1024EFH100 */
    case 0x07213053:            /* MZ2048EFH100 */
    case 0x0723A053:            /* MZ1024EFM100 */
    case 0x0723B053:            /* MZ2048EFM100 */
        if (*(unsigned*)DEVCFG3 & DEVCFG3_FETHIO) {
            /*
             * Default setup for 100-pin devices.
             */
            ANSELGCLR = 1 << 9;         /* Disable analog pad on RG9 for EREFCLK */
            ANSELBCLR = 1 << 12;        /* Disable analog pad on RB12 for ERXD0 */
            ANSELBCLR = 1 << 13;        /* Disable analog pad on RB13 for ERXD1 */
            ANSELGCLR = 1 << 8;         /* Disable analog pad on RG8 for ECRSDV */
            ANSELBCLR = 1 << 11;        /* Disable analog pad on RB11 for ERXERR */

            LATFCLR = 1 << 1; TRISFCLR = 1 << 1;    /* set RF1 as output for ETXD0 */
            LATFCLR = 1 << 0; TRISFCLR = 1 << 0;    /* set RF0 as output for ETXD1 */
            LATDCLR = 1 << 2; TRISDCLR = 1 << 2;    /* set RD2 as output for ETXEN */
        } else {
            /*
             * Alternative setup for 100-pin devices.
             */
            ANSELGCLR = 1 << 9;         /* Disable analog pad on RG9 for AEREFCLK */
            ANSELECLR = 1 << 8;         /* Disable analog pad on RE8 for AERXD0 */
            ANSELECLR = 1 << 9;         /* Disable analog pad on RE9 for AERXD1 */
            ANSELGCLR = 1 << 8;         /* Disable analog pad on RG8 for AECRSDV */
            ANSELGCLR = 1 << 15;        /* Disable analog pad on RG15 for AERXERR */
            ANSELDCLR = 1 << 14;        /* Disable analog pad on RD14 for AETXD0 */
            ANSELDCLR = 1 << 15;        /* Disable analog pad on RD15 for AETXD1 */

            LATDCLR = 1 << 14; TRISDCLR = 1 << 14;  /* set RD14 as output for AETXD0 */
            LATDCLR = 1 << 15; TRISDCLR = 1 << 15;  /* set RD15 as output for AETXD1 */
            LATACLR = 1 << 15; TRISACLR = 1 << 15;  /* set RA15 as output for AETXEN */
        }
        break;

    case 0x05118053:            /* MZ2048ECG124 */
    case 0x0511D053:            /* MZ2048ECH124 */
    case 0x05145053:            /* MZ2048ECM124 */
    case 0x07217053:            /* MZ1024EFG124 */
    case 0x07218053:            /* MZ2048EFG124 */
    case 0x0721C053:            /* MZ1024EFH124 */
    case 0x0721D053:            /* MZ2048EFH124 */
    case 0x07244053:            /* MZ1024EFM124 */
    case 0x07245053:            /* MZ2048EFM124 */
        panic("en: 124-pin devices not supported yet\n");
        break;

    case 0x05122053:            /* MZ2048ECG144 */
    case 0x05127053:            /* MZ2048ECH144 */
    case 0x0514F053:            /* MZ2048ECM144 */
    case 0x07221053:            /* MZ1024EFG144 */
    case 0x07222053:            /* MZ2048EFG144 */
    case 0x07226053:            /* MZ1024EFH144 */
    case 0x07227053:            /* MZ2048EFH144 */
    case 0x0724E053:            /* MZ1024EFM144 */
    case 0x0724F053:            /* MZ2048EFM144 */
        /*
         * Setup for 144-pin devices.
         */
        ANSELJCLR = 1 << 11;        /* Disable analog pad on RJ11 for EREFCLK */
        ANSELHCLR = 1 << 5;         /* Disable analog pad on RH5 for ERXD1 */
        ANSELHCLR = 1 << 4;         /* Disable analog pad on RH4 for ERXERR */
        ANSELJCLR = 1 << 8;         /* Disable analog pad on RJ8 for ETXD0 */
        ANSELJCLR = 1 << 9;         /* Disable analog pad on RJ9 for ETXD1 */

        LATJCLR = 1 << 8; TRISJCLR = 1 << 8;    /* set RJ8 as output for ETXD0 */
        LATJCLR = 1 << 9; TRISJCLR = 1 << 9;    /* set RJ9 as output for ETXD1 */
        LATDCLR = 1 << 6; TRISDCLR = 1 << 6;    /* set RD6 as output for ETXEN */
        break;

    default:
        panic("en: DEVID not recognized\n");
    }
}

static int
en_probe(config)
    struct conf_device *config;
{
    int unit = config->dev_unit;
    struct eth_port *e = &eth_port[unit];
    struct ifnet *ifp = &e->netif;
    int s, id;

    /* Only one Ethernet device is supported by this driver. */
    if (unit != 0)
        return 0;

    s = splimp();

    /* Board-dependent initialization. */
    setup_signals();

    /* Link is down. */
    e->is_up = 0;

    /* As per section 35.4.10 of the Pic32 Family Ref Manual. */
    en_setup();
    en_setup_mac();
    en_setup_mii();

#ifdef ETHERNET_PHY_ADDR
    /* PHY address defined in the kernel configuration file. */
    e->phy_addr = ETHERNET_PHY_ADDR;
    if (phy_reset(e->phy_addr) < 0) {
        ETHCON1 = 0;
        printf("Ethernet PHY not detected at address=%u\n", e->phy_addr);
        return 0;
    }
#else
    /* Auto-detect the PHY address, 0-31. */
    for (e->phy_addr=0; e->phy_addr<32; e->phy_addr++) {
        if (phy_reset(e->phy_addr) >= 0)
            break;
    }
    if (e->phy_addr >= 32) {
        ETHCON1 = 0;
        printf("Ethernet PHY not detected\n");
        return 0;
    }
#endif

    /* Extract our MAC address */
    e->macaddr[0] = EMAC1SA2;
    e->macaddr[1] = EMAC1SA2 >> 8;
    e->macaddr[2] = EMAC1SA1;
    e->macaddr[3] = EMAC1SA1 >> 8;
    e->macaddr[4] = EMAC1SA0;
    e->macaddr[5] = EMAC1SA0 >> 8;

    /* Enable interrupts. */
    ETHIENSET = PIC32_ETHIRQ_TXBUSE |       /* Transmit Bus Error */
                PIC32_ETHIRQ_TXDONE |       /* Transmit Done */
                PIC32_ETHIRQ_TXABORT |      /* Transmit Abort */
                PIC32_ETHIRQ_RXBUSE |       /* Receive Bus Error */
                PIC32_ETHIRQ_RXDONE |       /* Receive Done */
                PIC32_ETHIRQ_RXBUFNA |      /* Receive Buffer Not Available */
                PIC32_ETHIRQ_RXOVFLW;       /* Receive FIFO Overflow */

    IECSET(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);
    splx(s);

    /*
     * Interface exists: make available by filling in network interface
     * record.  System will initialize the interface when it is ready
     * to accept packets.  We get the ethernet address here.
     */
    printf("en%d at interrupt %u, MAC address %s\n",
        unit, PIC32_IRQ_ETH, ether_sprintf(e->macaddr));

    e->phy_id = (phy_read(e->phy_addr, PHY_ID1, 1) << 16 |
          phy_read(e->phy_addr, PHY_ID2, 1)) & 0xfffffff0;
    printf("en%d: ", unit);
    switch (e->phy_id) {
    case PHY_ID_LAN8720A:
        printf("<SMSC LAN8720A>");
        break;
    case PHY_ID_LAN8740A:
        printf("<SMSC LAN8740A>");
        break;
    case PHY_ID_IP101G:
        printf("<IC+ IP101G>");
        phy_write(e->phy_addr, 20, 16, 100); // Select page 16
        break;
    case 0:
        /* Looks like a virtual PHY of LAN9303. */
        id = (phy_read(e->phy_addr + 1, PHY_ID1, 1) << 16 |
              phy_read(e->phy_addr + 1, PHY_ID2, 1)) & 0xfffffff0;
        if (id != PHY_ID_LAN9303)
            goto other;
        e->phy_id = id;
        printf("<SMSC LAN9303>");
        break;
    default:
other:  printf("PHY id=%08x", e->phy_id);
        break;
    }
    printf(" at address %u\n", e->phy_addr);

    ifp->if_unit = unit;
    ifp->if_name = "en";
    ifp->if_mtu = ETHERMTU;
    ifp->if_flags = IFF_BROADCAST | IFF_NOTRAILERS;
    ifp->if_init = en_init;
    ifp->if_output = ether_output;
    ifp->if_start = en_start;
    ifp->if_ioctl = en_ioctl;
    ifp->if_reset = en_reset;
    ifp->if_watchdog = en_watchdog;
    ifp->if_timer = 1;
#if NBPFILTER > 0
    bpfattach(&ifp->if_bpf, ifp, DLT_EN10MB, sizeof(struct ether_header));
#endif
    if_attach(ifp);
    return 1;
}

struct driver endriver = {
    "en", en_probe,
};
#endif
