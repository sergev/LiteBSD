/*
 * Network driver for the PIC32 internal Ethernet controller with 8720 PHY.
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
#define RX_BYTES_PER_DESC   256
#define RX_BYTES            (RX_PACKETS * ETHER_MAX_LEN)
#define RX_DESCRIPTORS      (RX_BYTES / RX_BYTES_PER_DESC)
#define TX_DESCRIPTORS      1

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
#define DESC_SOP(d)         ((d)->hdr & 0x80000000)
#define DESC_SET_SOP(d)     (d)->hdr |= 0x80000000

/* End of packet */
#define DESC_EOP(d)         ((d)->hdr & 0x40000000)
#define DESC_SET_EOP(d)     (d)->hdr |= 0x40000000

/* Number of data bytes */
#define DESC_BYTECNT(d)     ((d)->hdr >> 16 & 0x7ff)
#define DESC_SET_BYTECNT(d,n) ((d)->hdr |= (n) << 16)

/* Next descriptor pointer valid */
#define DESC_SET_NPV(d)     (d)->hdr &= 0x00000100
#define DESC_CLEAR_NPV(d)   (d)->hdr &= ~0x00000100

/* Eth controller owns this desc */
#define DESC_EOWN(d)        ((d)->hdr & 0x00000080)
#define DESC_SET_EOWN(d)    (d)->hdr &= 0x00000080
#define DESC_CLEAR_EOWN(d)  (d)->hdr &= ~0x00000080

/* Size of received packet */
#define DESC_FRAMESZ(d)     ((d)->status & 0xffff)

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

    int         phy_id;             /* PHY id */
    int         is_up;              /* whether the link is up */
    int         is_transmitting;    /* block re-entering en_start */

    char        rx_buffer[RX_BYTES];
    char        tx_buffer[ETHER_MAX_LEN];
    eth_desc_t  rx_desc[RX_DESCRIPTORS+1]; /* an additional terminating descriptor */
    eth_desc_t  tx_desc[TX_DESCRIPTORS+1];

    unsigned    receive_index;      /* next RX descriptor to look */
#define INCR_RX_INDEX(_i)   ((_i + 1) % RX_DESCRIPTORS)

} eth_port[NEN];

/*-------------------------------------------------------------
 * PHY routines for SMSC LAN8720A chip.
 */
#define PHY_CONTROL             0       /* Basic Control Register */
#define PHY_STATUS              1       /* Basic Status Register */
#define PHY_MODE                18      /* Special Modes */
#define PHY_SPECIAL             31      /* Special Control/Status Register */

#define PHY_CONTROL_RESET	0x8000	/* Soft reset, bit self cleared */

#define PHY_STATUS_ANEG_ACK     0x0020	/* Auto-negotiation acknowledge */
#define PHY_STATUS_CAP_ANEG     0x0008	/* Auto-negotiation available */
#define PHY_STATUS_LINK         0x0004	/* Link valid */

#define PHY_MODE_PHYAD          0x000f  /* PHY address mask */

#define PHY_SPECIAL_AUTODONE    0x1000  /* Auto-negotiation is done */
#define PHY_SPECIAL_FDX         0x0010  /* Full duplex */
#define PHY_SPECIAL_100         0x0008  /* Speed 100 Mbps */

/*
 * Read PHY register.
 * Return -1 when failed.
 */
static int phy_read(int phy_id, int reg_id, unsigned msec)
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

    EMAC1MADR = PIC32_EMAC1MADR(phy_id, reg_id);
    EMAC1MCMDSET = PIC32_EMAC1MCMD_READ;

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
static int phy_scan(int phy_id, int reg_id,
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
    EMAC1MADR = PIC32_EMAC1MADR(phy_id, reg_id);
    EMAC1MCMDSET = PIC32_EMAC1MCMD_SCAN;

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
static int phy_write(int phy_id, int reg_id, int value, unsigned msec)
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

    EMAC1MADR = PIC32_EMAC1MADR(phy_id, reg_id);
    EMAC1MWTD = value;

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
 * Determine whether the link is up.
 * When up, setup MAC controller for required speed and duplex..
 */
static int is_phy_linked(int phy_id, int was_up)
{
    int status = phy_read(phy_id, PHY_STATUS, 1);
    if (status < 0) {
        return 0;
    }

    int link_is_up =
        (status & PHY_STATUS_LINK) &&       /* link is up */
        (status & PHY_STATUS_CAP_ANEG) &&   /* able to auto-negotiate */
        (status & PHY_STATUS_ANEG_ACK);     /* auto-negotiation completed */

    /* Set our link speed. */
    if (link_is_up && ! was_up) {
        /* Must disable the RX while setting these parameters. */
        int rxen = ETHCON1 & PIC32_ETHCON1_RXEN;
        ETHCON1CLR = PIC32_ETHCON1_RXEN;

        /* Get the speed. */
        int special = phy_read(phy_id, PHY_SPECIAL, 1);
        int speed_100 = 0;
        int full_duplex = 0;

        if (special & PHY_SPECIAL_AUTODONE) {
            /* Auto-negotiation is done. */
            speed_100 = (special & PHY_SPECIAL_100);
            full_duplex = (special & PHY_SPECIAL_FDX);
        }
        log(LOG_ERR, "en0: link up, %s, %s duplex\n",
            speed_100 ? "100Mbps" : "10Mbps",
            full_duplex ? "full" : "half");

        /* Set speed. */
        if (speed_100)
            EMAC1SUPPSET = PIC32_EMAC1SUPP_SPEEDRMII;
        else
            EMAC1SUPPCLR = PIC32_EMAC1SUPP_SPEEDRMII;

        /* Set duplex and gap size. */
        if (full_duplex) {
            EMAC1CFG2SET = PIC32_EMAC1CFG2_FULLDPLX;
            EMAC1IPGT = 21;
        } else {
            EMAC1CFG2CLR = PIC32_EMAC1CFG2_FULLDPLX;
            EMAC1IPGT = 18;
        }

        /* Return the Rx Enable back to what it was. */
        if (rxen)
            ETHCON1SET = PIC32_ETHCON1_RXEN;
    }
    return link_is_up;
}

/*
 * Reset the PHY via MIIM interface.
 * Return -1 on failure.
 */
static int phy_reset(int phy_id)
{
    int mode;

    mode = phy_read(phy_id, PHY_MODE, 100);
    if (mode < 0)
        return -1;

    if ((mode & PHY_MODE_PHYAD) != phy_id) {
        printf("Wrong PHY id!\n");
    }

    /* Send a reset to the PHY. */
    if (phy_write(phy_id, PHY_CONTROL, PHY_CONTROL_RESET, 100) < 0)
        return -1;

    /* Wait for the reset pin to autoclear. */
    if (phy_scan(phy_id, PHY_CONTROL, PHY_CONTROL_RESET, 0, 500) < 0)
        return -1;

    return 0;
}

/*
 * Initialize the Ethernet Controller.
 */
static void en_setup()
{
    /* Disable the ethernet interrupt. */
    IECCLR(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);

    /* Turn the Ethernet cotroller off. */
    ETHCON1 = 0;

    /* Wait for abort to finish. */
    while (ETHSTAT & PIC32_ETHSTAT_ETHBUSY)
        continue;

    /* Clear the interrupt flag bit. */
    IFSCLR(PIC32_IRQ_ETH >> 5) = 1 << (PIC32_IRQ_ETH & 31);

    /* Clear interrupts. */
    ETHIEN = 0;
    ETHIRQ = 0;

    /* Clear discriptor pointers; for now. */
    ETHTXST = 0;
    ETHRXST = 0;

    /* Auto flow control is on. */
    ETHCON1SET = PIC32_ETHCON1_PTV(1);  /* the max number of pause timeouts */
    ETHCON1SET = PIC32_ETHCON1_AUTOFC;

    /* High and low watermarks. */
    int empty_watermark = ETHER_MAX_LEN / RX_BYTES_PER_DESC;
    int full_watermark  = RX_DESCRIPTORS - (ETHER_MAX_LEN * 2) / RX_BYTES_PER_DESC;
    ETHRXWM = PIC32_ETHRXWM_FWM(full_watermark) |
              PIC32_ETHRXWM_EWM(empty_watermark);

    /* Set RX buffer size, descriptor buffer size in bytes / 16. */
    ETHCON2 = RX_BYTES_PER_DESC >> 4;

    /* Set our Rx filters. */
    ETHRXFC = PIC32_ETHRXFC_CRCOKEN |   /* enable checksum filter */
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

    /* Turn on the ethernet controller. */
    ETHCON1SET = PIC32_ETHCON1_ON;
}

/*
 * Initialize the MAC.
 */
static void en_setup_mac()
{
    /* Reset the MAC. */
    EMAC1CFG1 = PIC32_EMAC1CFG1_SOFTRESET;

    /* Pull it out of reset. */
    EMAC1CFG1 = 0;
    EMAC1CFG1 = PIC32_EMAC1CFG1_TXPAUSE |   /* MAC TX flow control */
                PIC32_EMAC1CFG1_RXPAUSE |   /* MAC RX flow control */
                PIC32_EMAC1CFG1_RXENABLE;   /* Receive enable */

    EMAC1CFG2 = PIC32_EMAC1CFG2_EXCESSDFR | /* Defer to carrier indefinitely */
                PIC32_EMAC1CFG2_BPNOBKOFF | /* Backpressure/No Backoff */
                PIC32_EMAC1CFG2_AUTOPAD |   /* Automatic detect pad enable */
                PIC32_EMAC1CFG2_PADENABLE | /* Pad/CRC enable */
                PIC32_EMAC1CFG2_CRCENABLE | /* CRC enable */
                PIC32_EMAC1CFG2_LENGTHCK;   /* Frame length checking */

    /* These are all default. */
    EMAC1MAXF = 1518;                       /* max frame size in bytes */
    EMAC1IPGR = PIC32_EMAC1IPGR(12, 18);    /* non-back-to-back interpacket gap */
    EMAC1CLRT = PIC32_EMAC1CLRT(55, 15);    /* collision window/retry limit */
}

/*
 * Initialize RMII and MIIM.
 */
static void en_setup_mii()
{
    EMAC1SUPP = PIC32_EMAC1SUPP_RESETRMII;  /* reset RMII */
    EMAC1SUPP = 0;

    EMAC1MCFG = PIC32_EMAC1MCFG_RESETMGMT;  /* reset the management fuctions */
    EMAC1MCFG = 0;

    /* The IEEE 802.3 spec says no faster than 2.5MHz.
     * 80 / 40 = 2MHz
     */
    EMAC1MCFG = PIC32_EMAC1MCFG_CLKSEL_40;
}

/*
 * Set DMA descriptors.
 */
static void en_init_dma(struct eth_port *e)
{
    int i;

    /* Set Rx discriptor list.
     * All owned by the ethernet controller. */
    bzero(e->rx_desc, sizeof(e->rx_desc));
    for (i=0; i<RX_DESCRIPTORS; i++) {
        DESC_SET_EOWN(&e->rx_desc[i]);
        DESC_CLEAR_NPV(&e->rx_desc[i]);
        e->rx_desc[i].paddr = MACH_VIRT_TO_PHYS(e->rx_buffer + (i * RX_BYTES_PER_DESC));
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
    struct mbuf *m0, *m;
    char *buffer;
    int total,t;

    /* The previous transmit has not completed yet. */
    if (e->is_transmitting)
        return;

    /* Interface is administratively deactivated. */
    if ((e->netif.if_flags & IFF_RUNNING) == 0)
        return;

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
     * Copy the mbuf chain into the transmit buffer
     */
    e->is_transmitting = 1;             /* prevent entering nestart */
    buffer = e->tx_buffer;
    t = 0;
    for (m0 = m; m != 0; m = m->m_next)
        t += m->m_len;

    m = m0;
    total = t;
    for (m0 = m; m != 0; ) {
        if ((m->m_len & 1) && t > m->m_len) {
            bcopy(mtod(m, caddr_t), buffer, m->m_len - 1);
            t -= m->m_len - 1;
            buffer += m->m_len - 1;
            m->m_data += m->m_len - 1;
            m->m_len = 1;
            m = m_pullup(m, 2);
        } else {
            bcopy(mtod(m, caddr_t), buffer, m->m_len);
            buffer += m->m_len;
            t -= m->m_len;
            MFREE(m, m0);
            m = m0;
        }
    }

    /*
     * Init transmit descriptors, start transmittion.
     */
    bzero(e->tx_desc, sizeof(e->tx_desc));

    /* Use descriptor 0. */
    e->tx_desc[0].paddr = MACH_VIRT_TO_PHYS(e->tx_buffer);
    DESC_SET_BYTECNT(&e->tx_desc[0], total);
    DESC_SET_SOP(&e->tx_desc[0]);
    DESC_SET_EOP(&e->tx_desc[0]);
    DESC_SET_EOWN(&e->tx_desc[0]);

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
    if (ifp->if_flags & IFF_RUNNING)
        return;

    /* Enable receiver. */
    s = splimp();
    ETHCON1SET = PIC32_ETHCON1_RXEN;
    e->netif.if_flags |= IFF_RUNNING;
    en_start(ifp);
    splx(s);
}

/*
 * Reset of interface.
 */
static void
en_reset(unit, uban)
    int unit, uban;
{
    if (unit >= NEN)
        return;
    printf("en%d: reset\n", unit);
    eth_port[unit].is_transmitting = 0;
    en_init(unit);
}

/*
 * Pull read data off a interface.
 * When full cluster sized units are present
 * we copy into clusters.
 */
static struct mbuf *
en_get(buf, totlen, ifp)
    caddr_t buf;
    int totlen;
    struct ifnet *ifp;
{
    struct mbuf *top, **mp, *m;
    caddr_t cp = buf;
    char *epkt;
    int len;

    buf += sizeof(struct ether_header);
    totlen -= sizeof(struct ether_header);
    cp = buf;
    epkt = cp + totlen;

    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == 0)
        return 0;
    m->m_pkthdr.rcvif = ifp;
    m->m_pkthdr.len = totlen;
    m->m_len = MHLEN;

    top = 0;
    mp = &top;
    while (totlen > 0) {
        if (top) {
            MGET(m, M_DONTWAIT, MT_DATA);
            if (m == 0) {
                m_freem(top);
                return 0;
            }
            m->m_len = MLEN;
        }
        len = min(totlen, epkt - cp);
        if (len >= MINCLSIZE) {
            MCLGET(m, M_DONTWAIT);
            if (m->m_flags & M_EXT)
                m->m_len = len = min(len, MCLBYTES);
            else
                len = m->m_len;
        } else {
            /*
             * Place initial small packet/header at end of mbuf.
             */
            if (len < m->m_len) {
                if (top == 0 && len + max_linkhdr <= m->m_len)
                    m->m_data += max_linkhdr;
                m->m_len = len;
            } else
                len = m->m_len;
        }
        bcopy(cp, mtod(m, caddr_t), (unsigned)len);
        cp += len;
        *mp = m;
        mp = &m->m_next;
        totlen -= len;
        if (cp == epkt)
            cp = buf;
    }
    return top;
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
    unsigned frame_size = DESC_FRAMESZ(&e->rx_desc[e->receive_index]);
    if (frame_size <= 0) {
        /* Cannot happen. */
        printf("en_recv: bad frame size = %u\n", frame_size);
        return;
    }

    if (! DESC_SOP(&e->rx_desc[e->receive_index])) {
        /* Start of packet is expected. */
        printf("en_recv: no SOP flag (%x)\n", e->rx_desc[e->receive_index].hdr);
    }

    char buf[ETHER_MAX_LEN];
    unsigned read_nbytes = 0;

    while (read_nbytes < frame_size) {
        if (DESC_EOWN(&e->rx_desc[e->receive_index]))
            break;

        int end_of_packet = DESC_EOP(&e->rx_desc[e->receive_index]);
        unsigned nbytes = DESC_BYTECNT(&e->rx_desc[e->receive_index]);
        if (nbytes > frame_size - read_nbytes)
            nbytes = frame_size - read_nbytes;

        unsigned vaddr = MACH_PHYS_TO_UNCACHED(e->rx_desc[e->receive_index].paddr);
        bcopy((char*) vaddr, &buf[read_nbytes], nbytes);
        read_nbytes += nbytes;

        /* Free the receive descriptor. */
        DESC_SET_EOWN(&e->rx_desc[e->receive_index]);       /* give up ownership */
        ETHCON1SET = PIC32_ETHCON1_BUFCDEC;                 /* decrement the BUFCNT */
        e->receive_index = INCR_RX_INDEX(e->receive_index); /* check the next one */

        /* If we are done, get out. */
        if (end_of_packet || read_nbytes == frame_size)
            break;
    }

    /*
     * Pass a packet to the higher levels.
     */
    struct mbuf *m = en_get(buf, frame_size, &e->netif);
#if NBPFILTER > 0
    /*
     * Check if there's a bpf filter listening on this interface.
     * If so, hand off the raw packet to enet.
     */
    if (e->netif.if_bpf) {
        bpf_mtap(e->netif.if_bpf, m);
#if 0 // TODO
        /*
         * Keep the packet if it's a broadcast or has our
         * physical ethernet address (or if we support
         * multicast and it's one).
         */
        if (
#ifdef MULTICAST
            (flags & (M_BCAST | M_MCAST)) == 0 &&
#else
            (flags & M_BCAST) == 0 &&
#endif
            bcmp(et.ether_dhost, le->sc_addr, sizeof(et.ether_dhost)) != 0
        ) {
            m_freem(m);
            return;
        }
#endif
    }
#endif
    if (m != 0)
        ether_input(&e->netif, (struct ether_header*)buf, m);
}

/*
 * Timeout routine.
 */
static void
en_watchdog(unit)
    int unit;
{
    struct eth_port *e = &eth_port[unit];

    /* Poll whether the link is active. */
    e->is_up = is_phy_linked(e->phy_id, e->is_up);

    /* Call it every second. */
    e->netif.if_timer = 1;
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

    /* Receiver error. */
    if (irq & (PIC32_ETHIRQ_RXBUSE |
               PIC32_ETHIRQ_RXBUFNA |
               PIC32_ETHIRQ_RXOVFLW)) {
        e->netif.if_ierrors++;
        log(LOG_ERR, "en%d: receive error: irq %x\n", unit, irq);
    }

    /* We received something. */
    while (! DESC_EOWN(&e->rx_desc[e->receive_index])) {
        e->netif.if_ipackets++;
        en_recv(e);
    }

    /* Transmit error. */
    if (irq & (PIC32_ETHIRQ_TXBUSE | PIC32_ETHIRQ_TXABORT)) {
        log(LOG_ERR, "en%d: transmit error: irq %x\n", unit, irq);
        e->is_transmitting = 0;
        e->netif.if_oerrors++;
    }

    /* Packet Transmitted */
    if (irq & PIC32_ETHIRQ_TXDONE) {
        e->is_transmitting = 0;
        ++e->netif.if_opackets;
        e->netif.if_collisions += ETHSCOLFRM + ETHMCOLFRM;
    }

    /* Transmitter is idle. */
    if (! (ETHCON1 & PIC32_ETHCON1_TXRTS)) {
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
        if ((ifp->if_flags & IFF_UP) == 0 &&
            ifp->if_flags & IFF_RUNNING)
        {
            /* Disable receiver. */
            ETHCON1CLR = PIC32_ETHCON1_RXEN;
            ifp->if_flags &= ~IFF_RUNNING;
        }
        else if (ifp->if_flags & IFF_UP &&
            (ifp->if_flags & IFF_RUNNING) == 0)
        {
            en_init(ifp->if_unit);
        }
        break;

#ifdef notdef
    case SIOCGHWADDR:
        /* Change MAC address. */
        struct ifreq *ifr = (struct ifreq *)data;
        struct eth_port *e = &eth_port[ifp->if_unit];
        bcopy((caddr_t)e->macaddr, (caddr_t) &ifr->ifr_data,
            sizeof(e->macaddr));
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

static int
en_probe(config)
    struct scsi_device *config;
{
    int unit = config->sd_unit;
    struct eth_port *e = &eth_port[unit];
    struct ifnet *ifp = &e->netif;
    int s;

    /* Only one Ethernet device is supported by this driver. */
    if (unit != 0)
        return 0;

    s = splimp();

#ifdef MIBII
    /*
     * Setup for PIC32MZ EC Starter Kit board.
     */
    TRISHSET = 1 << 8;                  /* set RH8 as input for ERXD0 */
    TRISHSET = 1 << 5;                  /* set RH5 as input for ERXD1 */
    TRISHSET = 1 << 4;                  /* set RH4 as input for ERXERR */

    /* Default PHY address is 0 on LAN8720 PHY daughter board. */
    e->phy_id = 0;
#endif
    /* Link is down. */
    e->is_up = 0;

    /* As per section 35.4.10 of the Pic32 Family Ref Manual. */
    en_setup();
    en_setup_mac();
    en_setup_mii();
    if (phy_reset(e->phy_id) < 0) {
        printf("Ethernet PHY not detected at ID=%u\n", e->phy_id);
        return 0;
    }

    /* Set DMA descriptors. */
    en_init_dma(e);

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
