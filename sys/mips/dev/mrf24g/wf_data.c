/*
 * MRF24WG Data Tx/Rx
 *
 * Functions that allocate and deallocate Tx/Rx data messages
 * as well as functions that perform Tx/Rx processing.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

static bool g_HostRAWPacketRx;

void SignalPacketRx()
{
    g_HostRAWPacketRx = 1;
}

bool isPacketRx()
{
    return g_HostRAWPacketRx;
}

void ClearPacketRx()
{
    g_HostRAWPacketRx = 0;
}

/*
 * called from WiFi_Task
 */
void RxPacketCheck()
{
    if (isPacketRx()) {
        ClearPacketRx();
        WF_ProcessRxPacket();
    }
}

/*
 * Allocate a Data Tx buffer for use by the TCP/IP stack.
 * Returns True if data tx buffer successfully allocated, else False.
 *
 * Determines if WiFi chip has enough memory to allocate a tx data buffer,
 * and, if so, allocates it.
 *
 * Parameters:
 *  bytes_needed -- number of bytes needed for the data tx message
 */
int WF_TxPacketAllocate(unsigned bytes_needed)
{
    unsigned buf_avail, nbytes;
    u_int32_t startTime;

    EnsureWFisAwake();

    /* Allocate an extra 4 bytes for WiFi message preamble. */
    bytes_needed += 4;

    startTime = mrf_timer_read();
    while (mrf_timer_elapsed(startTime) < 20) {
        /* get total bytes available for DATA tx memory pool */
        buf_avail = mrf_read(MRF24_REG_WFIFO_BCNT0) & FIFO_BCNT_MASK;
        if (buf_avail < bytes_needed) {
            /* not enough bytes available at this time to satisfy request */
            udelay(10);
            continue;
        }

        /* allocate and create the new Tx buffer (mgmt or data) */
        nbytes = mrf_raw_move(RAW_ID_TRANSMIT, RAW_DATA_POOL, 1, bytes_needed);
        if (nbytes > 0) {
            // set the RAW index at 4 to leave room for internal 4 byte header.  Ethernet
            // packet data starts at index 4.
            mrf_raw_seek(RAW_ID_TRANSMIT, 4);
            return 1;
        }

        printf("--- %s: failed\n", __func__);
        udelay(10);
    }
    return 0;
}

/*
 * Copy Ethernet packet contents to the RAW data transmit buffer.
 */
void WF_TxPacketCopy(u_int8_t *buf, unsigned length)
{
    /* The RAW index has been previously set to 4
     * by WF_TxPacketAllocate function. */
    mrf_raw_write(RAW_ID_TRANSMIT, buf, length);
}

void WF_TxPacketTransmit(unsigned packet_size)
{
    /* create internal preamble */
    static const u_int8_t tx_preamble[4] =
        { WF_TYPE_DATA_REQUEST, WF_SUBTYPE_STD_DATA, 1, 0 };

    /* will be overwriting the ethernet header source address with the snap header */
    static const u_int8_t snap_hdr[6] =
        { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };

    EnsureWFisAwake();

    /* Write out internal preamble, starting at index 0 in the raw window. */
    mrf_raw_pwrite(RAW_ID_TRANSMIT, tx_preamble, sizeof(tx_preamble), 0);

    /* Overwrite the Ethernet source address in Tx packet with SNAP header.
     * Ethernet header starts at index 10 within packet, after 4 bytes
     * of preamble and 6 bytes of dest address). */
    mrf_raw_pwrite(RAW_ID_TRANSMIT, snap_hdr, sizeof(snap_hdr),
        sizeof(tx_preamble) + WF_MAC_ADDRESS_LENGTH);

    /* Notify WiFi device that there is a transmit frame to send.
     * The frame will be automatically deallocated after RF transmission
     * is completed. */
    mrf_raw_move(RAW_ID_TRANSMIT, RAW_MAC, 0, packet_size + sizeof(tx_preamble));
}

unsigned WF_RxPacketLengthGet()
{
    unsigned len;
    u_int8_t rx_preamble[2];

    /* Mount Read FIFO to RAW Rx window.
     * Allows use of RAW engine to read rx data packet.
     * Function call returns number of bytes in the data packet. */
    len = mrf_raw_move(RAW_ID_RECEIVE, RAW_MAC, 1, 0);
    if (len == 0) {
        printf("--- %s: failed\n", __func__);
        return 0;
    }

    /* Read the data frame internal preamble (type and subtype) to verify
     * that we did, in fact, mount an Rx data packet. */
    mrf_raw_read(RAW_ID_RECEIVE, rx_preamble, sizeof(rx_preamble));
    if (rx_preamble[0] != WF_TYPE_DATA_RX_INDICATE) {
        printf("--- %s: invalid preamble type=%u\n", __func__, rx_preamble[0]);
        return 0;
    }

    /* Set raw pointer to start of Ethernet packet. */
    mrf_raw_seek(RAW_ID_RECEIVE, 16);
    return len - 16;
}

/*
 * Deallocate a Data Rx buffer.
 */
void WF_RxPacketDeallocate()
{
    mrf_raw_move(RAW_ID_RECEIVE, RAW_DATA_POOL, 0, 0);
}

void WF_RxPacketCopy(u_int8_t *buf, unsigned length)
{
    mrf_raw_read(RAW_ID_RECEIVE, buf, length);
}
