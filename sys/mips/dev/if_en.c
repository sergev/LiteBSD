/*
 * PIC32 Ethernet driver
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

#ifdef INET
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#endif

#define ETHER_MIN_LEN       64
#define ETHER_MAX_LEN       1536

#define RX_PACKETS          4
#define RX_BYTES_PER_DESC   128                             // This is tuned as a balance between the number of discriptors and size wasting empty space at the end of a discriptor
#define RX_BYTES            (RX_PACKETS * ETHER_MAX_LEN)    // how much buffer space we have for incoming frames
#define RX_DESCRIPTORS      (RX_BYTES / RX_BYTES_PER_DESC)  // check that this works out to be even ie with a modulo of zero
#define TX_DESCRIPTORS      5

#if 1
#define inb(a)      0
#define insw(a,b,l) /*empty*/
#define outb(a,v)   /*empty*/
#define outsw(a,b,l) /*empty*/

#define PKTSZ   3*512
#define TBUF    (16*1024)       /* Starting location of Transmit Buffer */
#define RBUF    (16*1024+PKTSZ) /* Starting location of Receive Buffer */
#define RBUFEND (32*1024)       /* Ending location of Transmit Buffer */

#define DS_PGSIZE   256         /* Size of RAM pages in bytes */

#define DSIS_RX     1
#define DSIS_RXE    2
#define DSIS_ROVRN  4
#define DSIS_TX     8
#define DSIS_TXE    16
#endif

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
    int         flags;
#define DSF_LOCK    1               /* block re-entering en_start */
    int         oactive;
    int         phy_id;             /* PHY id */
    int         is_up;              /* whether the link is up */
    struct mbuf *tx_packet;         /* Current packet under transmit */

    char        rx_buffer[RX_BYTES];
    eth_desc_t  rx_desc[RX_DESCRIPTORS+1]; /* an additional terminating descriptor */
    eth_desc_t  tx_desc[TX_DESCRIPTORS];

    unsigned    receive_index;      /* next RX descriptor to look */

    /* Pointers into the current frame to read out the received packet. */
    unsigned    read_index;
    unsigned    desc_offset;
    unsigned    frame_size;
    unsigned    read_nbytes;

    int         mask;
    int         ba;                 /* byte addr in buffer ram of inc pkt */
    int         cur;                /* current page being filled */
    u_char      tx_buffer[2048];    /* ETHERMTU + sizeof(long) */
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
static int eth_reset_phy(int phy_id)
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
 * Reset the Ethernet Controller.
 */
static void eth_reset()
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
 * Reset the MAC.
 */
static void eth_reset_mac()
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
 * RMII and MIIM reset.
 */
static void eth_reset_mii()
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
static void eth_init_dma(struct eth_port *e)
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

    /* Init our frame reading values,
     * used by read_packet. */
    e->read_index = 0;
    e->desc_offset = 0;
    e->frame_size = 0;
    e->read_nbytes = 0;
}

/*
 * Put to onboard RAM
 */
static void
en_put(up, ad, len)
    caddr_t up;
{
#if 0
    u_char cmd;

    cmd = inb(ds_cmd);
    outb(ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_START);

    /* Setup for remote dma */
    outb(ds0_isr, DSIS_RDC);
    if (len&1)
        len++;        /* roundup to words */
    outb(ds0_rbcr0, len);
    outb(ds0_rbcr1, len>>8);
    outb(ds0_rsar0, ad);
    outb(ds0_rsar1, ad>>8);

    /* Execute & stuff to card */
    outb(ds_cmd, DSCM_RWRITE|DSCM_PG0|DSCM_START);
    outsw(0x10, up, len/2);

    /* Wait till done, then shutdown feature */
    while ((inb(ds0_isr) & DSIS_RDC) == 0)
        ;
    outb(ds0_isr, DSIS_RDC);
    outb(ds_cmd, cmd);
#endif
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
    int buffer;
    int len = 0, i, total,t;

    /*
     * The DS8390 has only one transmit buffer, if it is busy we
     * must wait until the transmit interrupt completes.
     */
    outb(ds_cmd, DSCM_NODMA|DSCM_START);

    if (e->flags & DSF_LOCK)
        return;
#if 0
    if (inb(ds_cmd) & DSCM_TRANS)
        return;
#endif
    if ((e->netif.if_flags & IFF_RUNNING) == 0)
        return;

    IF_DEQUEUE(&e->netif.if_snd, m);

    if (m == 0)
        return;

    /*
     * Copy the mbuf chain into the transmit buffer
     */

    e->flags |= DSF_LOCK;       /* prevent entering nestart */
    buffer = TBUF; len = i = 0;
    t = 0;
    for (m0 = m; m != 0; m = m->m_next)
        t += m->m_len;

    m = m0;
    total = t;
    for (m0 = m; m != 0; ) {

        if (m->m_len&1 && t > m->m_len) {
            en_put(mtod(m, caddr_t), buffer, m->m_len - 1);
            t -= m->m_len - 1;
            buffer += m->m_len - 1;
            m->m_data += m->m_len - 1;
            m->m_len = 1;
            m = m_pullup(m, 2);
        } else {
            en_put(mtod(m, caddr_t), buffer, m->m_len);
            buffer += m->m_len;
            t -= m->m_len;
            MFREE(m, m0);
            m = m0;
        }
    }

    /*
     * Init transmit length registers, and set transmit start flag.
     */

    len = total;
    if (len < ETHER_MIN_LEN)
        len = ETHER_MIN_LEN;
    outb(ds0_tbcr0, len & 0xff);
    outb(ds0_tbcr1, (len >> 8) & 0xff);
    outb(ds0_tpsr, TBUF/DS_PGSIZE);
    outb(ds_cmd, DSCM_TRANS|DSCM_NODMA|DSCM_START);
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
    int s, i;

    if (ifp->if_addrlist == (struct ifaddr *)0)
        return;
    if (ifp->if_flags & IFF_RUNNING)
        return;

    s = splimp();

    /* set physical address on ethernet */
    outb(ds_cmd, DSCM_NODMA|DSCM_PG1|DSCM_STOP);
    for (i=0; i < 6; i++)
        outb(ds1_par0+i, e->macaddr[i]);

    /* clr logical address hash filter for now */
    for (i=0; i < 8; i++)
        outb(ds1_mar0+i, 0xff);
#if 0
    /* init regs */
    outb(ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_STOP);
    outb(ds0_rbcr0, 0);
    outb(ds0_rbcr1, 0);
    outb(ds0_imr, 0);
    outb(ds0_isr, 0xff);

    /* Word Transfers, Burst Mode Select, Fifo at 8 bytes */
    outb(ds0_dcr, DSDC_WTS|DSDC_BMS|DSDC_FT1);
    outb(ds0_tcr, 0);
    outb(ds0_rcr, DSRC_MON);
    outb(ds0_tpsr, 0);
    outb(ds0_pstart, RBUF/DS_PGSIZE);
    outb(ds0_pstop, RBUFEND/DS_PGSIZE);
    outb(ds0_bnry, RBUF/DS_PGSIZE);
    outb(ds_cmd, DSCM_NODMA|DSCM_PG1|DSCM_STOP);
    outb(ds1_curr, RBUF/DS_PGSIZE);
    e->cur = RBUF/DS_PGSIZE;
    outb(ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_START);
    outb(ds0_rcr, DSRC_AB);
    outb(ds0_dcr, DSDC_WTS|DSDC_BMS|DSDC_FT1);
    outb(ds0_imr, 0xff);
#endif

    e->netif.if_flags |= IFF_RUNNING;
    e->oactive = 0;
    e->mask = ~0;
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
    eth_port[unit].flags &= ~DSF_LOCK;
    en_init(unit);
}

/*
 * Pull read data off a interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We copy the trailer information and then all the normal
 * data into mbufs.  When full cluster sized units are present
 * we copy into clusters.
 */
static struct mbuf *
en_get(buf, totlen, off0, ifp)
    caddr_t buf;
    int totlen, off0;
    struct ifnet *ifp;
{
    struct mbuf *top, **mp, *m;
    int off = off0, len;
    caddr_t cp = buf;
    char *epkt;

    buf += sizeof(struct ether_header);
    cp = buf;
    epkt = cp + totlen;

    if (off) {
        cp += off + 2 * sizeof(u_short);
        totlen -= 2 * sizeof(u_short);
    }

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
 * Pass a packet to the higher levels.
 * We deal with the trailer protocol here.
 */
static void
en_read(e, buf, len)
    struct eth_port *e;
    char *buf;
    int len;
{
    struct ether_header *eh;
    struct mbuf *m;
    int off, resid;

    /*
     * Deal with trailer protocol: if type is trailer type
     * get true type from first 16-bit word past data.
     * Remember that type was trailer by setting off.
     */
    eh = (struct ether_header *)buf;
    eh->ether_type = ntohs((u_short)eh->ether_type);
#define en_dataaddr(eh, off, type) ((type)(((caddr_t)((eh)+1)+(off))))
    if (eh->ether_type >= ETHERTYPE_TRAIL &&
        eh->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
        off = (eh->ether_type - ETHERTYPE_TRAIL) * 512;
        if (off >= ETHERMTU)
            return;        /* sanity */
        eh->ether_type = ntohs(*en_dataaddr(eh, off, u_short *));
        resid = ntohs(*(en_dataaddr(eh, off+2, u_short *)));
        if (off + resid > len)
            return;      /* sanity */
        len = off + resid;
    } else  off = 0;

    if (len == 0)
        return;

    /*
     * Pull packet off interface.  Off is nonzero if packet
     * has trailing header; en_get will then force this header
     * information to be at the front, but we still have to drop
     * the type and length which are at the front of any trailer data.
     */
    m = en_get(buf, len, off, &e->netif);
    if (m == 0)
        return;

    ether_input(&e->netif, eh, m);
}

/*
 * Ethernet interface receiver interface.
 * If input error just drop packet.
 * Otherwise examine packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Othewise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
static void
en_recv(e, len)
    struct eth_port *e;
{
    e->netif.if_ipackets++;
#if 0
    if (len < ETHER_MIN_LEN || len > ETHER_MAX_LEN)
        return;

    /* this need not be so torturous - one/two bcopys at most into mbufs */
    en_fetch(e->tx_buffer, e->ba, min(len, DS_PGSIZE - 4));
    if (len > DS_PGSIZE - 4) {
        int l = len - (DS_PGSIZE - 4), b;
        u_char *p = e->tx_buffer + (DS_PGSIZE - 4);

        if(++e->cur > 0x7f) e->cur = 0x46;
        b = e->cur*DS_PGSIZE;

        while (l >= DS_PGSIZE) {
            en_fetch(p, b, DS_PGSIZE);
            p += DS_PGSIZE; l -= DS_PGSIZE;
            if(++e->cur > 0x7f) e->cur = 0x46;
            b = e->cur*DS_PGSIZE;
        }
        if (l > 0)
            en_fetch(p, b, l);
    }
    /* don't forget checksum! */
    len -= sizeof(struct ether_header) + sizeof(long);
#endif
    en_read(e, (caddr_t)e->tx_buffer, len);
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

#if 0
    if (e->sc_iflags & IFF_OACTIVE) {
        e->netif.if_flags &= ~IFF_RUNNING;
        einit(unit);
    } else if (e->sc_txcnt > 0)
        e->sc_iflags |= IFF_OACTIVE;
#endif

    /* Call it every second. */
    e->netif.if_timer = 1;
}

/*
 * Controller interrupt.
 */
void
enintr(unit)
{
    struct eth_port *e = &eth_port[unit];
    int isr;
    int cmd __attribute__((unused));

    /* Save cmd, clear interrupt */
    cmd = inb(ds_cmd);
loop:
    isr = inb(ds0_isr);
    outb(ds_cmd,DSCM_NODMA|DSCM_START);
    outb(ds0_isr, isr);

    /* Receiver error */
    if (isr & DSIS_RXE) {
        /* need to read these registers to clear status */
        (void) inb(ds0_rsr);
        (void) inb(0xD);
        (void) inb(0xE);
        (void) inb(0xF);
        e->netif.if_ierrors++;
    }

    /* We received something; rummage thru tiny ring buffer */
    if (isr & (DSIS_RX|DSIS_RXE|DSIS_ROVRN)) {
        u_char pend,lastfree;

        outb(ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG1);
        pend = inb(ds1_curr);
        outb(ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG0);
        lastfree = inb(ds0_bnry);

        /* Have we wrapped? */
        if (lastfree >= RBUFEND/DS_PGSIZE)
            lastfree = RBUF/DS_PGSIZE;
        if (pend < lastfree && e->cur < pend)
            lastfree = e->cur;
        else if (e->cur > lastfree)
            lastfree = e->cur;

        /* Something in the buffer? */
        while (pend != lastfree) {
            int len = 0;
            //struct prhdr ph;       /* hardware header of incoming packet */

            /* Extract header from microcephalic board */
            //en_fetch(&ph, lastfree*DS_PGSIZE, 4);
            e->ba = lastfree*DS_PGSIZE + 4;

            //len = ph.pr_sz0 + (ph.pr_sz1<<8);
            en_recv(e, len);
#if 0
/* buffer successor/predecessor in ring? */
#define succ(n) (((n)+1 >= RBUFEND/DS_PGSIZE) ? RBUF/DS_PGSIZE : (n)+1)
#define pred(n) (((n)-1 < RBUF/DS_PGSIZE) ? RBUFEND/DS_PGSIZE-1 : (n)-1)

            int nxt = ph.pr_nxtpg;

            /* Sanity check */
            if (nxt >= RBUF/DS_PGSIZE && nxt <= RBUFEND/DS_PGSIZE
                && nxt <= pend)
                e->cur = nxt;
            else
                e->cur = nxt = pend;

            /* Set the boundaries */
            lastfree = nxt;
            outb(ds0_bnry, pred(nxt));
            outb(ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG1);
            pend = inb(ds1_curr);
            outb(ds_cmd, DSCM_START|DSCM_NODMA|DSCM_PG0);
#endif
        }
        outb(ds_cmd, DSCM_START|DSCM_NODMA);
    }

    /* Transmit error */
    if (isr & DSIS_TXE) {
        e->flags &= ~DSF_LOCK;
        /* Need to read these registers to clear status */
        e->netif.if_collisions += inb(ds0_tbcr0);
        e->netif.if_oerrors++;
    }

    /* Packet Transmitted */
    if (isr & DSIS_TX) {
        e->flags &= ~DSF_LOCK;
        ++e->netif.if_opackets;
        e->netif.if_collisions += inb(ds0_tbcr0);
    }

    /* Receiver ovverun? */
    if (isr & DSIS_ROVRN) {
        log(LOG_ERR, "en%d: error: isr %x\n", e-eth_port, isr);
        outb(ds0_rbcr0, 0);
        outb(ds0_rbcr1, 0);
        outb(ds0_tcr, DSTC_LB0);
        outb(ds0_rcr, DSRC_MON);
        outb(ds_cmd, DSCM_START|DSCM_NODMA);
        outb(ds0_rcr, DSRC_AB);
        outb(ds0_tcr, 0);
    }

    /* Any more to send? */
    outb(ds_cmd, DSCM_NODMA|DSCM_PG0|DSCM_START);
    en_start(&e->netif);
    outb(ds_cmd, cmd);
    outb(ds0_imr, 0xff);

    /* Still more to do? */
    isr = inb(ds0_isr);
    if (isr)
        goto loop;
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
        ifp->if_flags |= IFF_UP;

        switch (ifa->ifa_addr->sa_family) {
#ifdef INET
        case AF_INET:
            en_init(ifp->if_unit);   /* before arpwhohas */
            ((struct arpcom *)ifp)->ac_ipaddr =
                IA_SIN(ifa)->sin_addr;
            arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
            break;
#endif
        default:
            en_init(ifp->if_unit);
            break;
        }
        break;

    case SIOCSIFFLAGS:
        if ((ifp->if_flags & IFF_UP) == 0 &&
            ifp->if_flags & IFF_RUNNING) {
            ifp->if_flags &= ~IFF_RUNNING;
            outb(ds_cmd,DSCM_STOP|DSCM_NODMA);
        } else if (ifp->if_flags & IFF_UP &&
            (ifp->if_flags & IFF_RUNNING) == 0)
            en_init(ifp->if_unit);
        break;

#ifdef notdef
    case SIOCGHWADDR:
        struct ifreq *ifr = (struct ifreq *)data;
        struct eth_port *e = &eth_port[ifp->if_unit];
        bcopy((caddr_t)e->macaddr, (caddr_t) &ifr->ifr_data,
            sizeof(e->macaddr));
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

    /* Clear packet queues. */
    e->tx_packet = 0;

    /* As per section 35.4.10 of the Pic32 Family Ref Manual. */
    eth_reset();
    eth_reset_mac();
    eth_reset_mii();
    if (eth_reset_phy(e->phy_id) < 0) {
        printf("Ethernet PHY not detected at ID=%u\n", e->phy_id);
        return 0;
    }

    /* Set DMA descriptors. */
    eth_init_dma(e);

    /* Extract our MAC address */
    e->macaddr[0] = EMAC1SA2;
    e->macaddr[1] = EMAC1SA2 >> 8;
    e->macaddr[2] = EMAC1SA1;
    e->macaddr[3] = EMAC1SA1 >> 8;
    e->macaddr[4] = EMAC1SA0;
    e->macaddr[5] = EMAC1SA0 >> 8;
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
    if_attach(ifp);
    return 1;
}

struct driver endriver = {
    en_probe,
};
#endif
