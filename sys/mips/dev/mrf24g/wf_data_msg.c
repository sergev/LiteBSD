/*
 * MRF24WG Data Tx/Rx
 *
 * Functions that allocate and deallocate Tx/Rx data messages
 * as well as functions that perform Tx/Rx processing.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

#define WF_TX_PREAMBLE_SIZE 4
#define ETHER_HEADER_SIZE   16  // 6 bytes dest address, 6 bytes src address, 2 bytes for type field
#define ENC_PREAMBLE_OFFSET 14

typedef struct {
    u_int8_t type;
    u_int8_t subType;
} t_rxPreamble;

// will be overwriting the ethernet header source address with the snap header
static const u_int8_t snapHdr[6] = {
    SNAP_VAL, SNAP_VAL, SNAP_CTRL_VAL, SNAP_TYPE_VAL, SNAP_TYPE_VAL, SNAP_TYPE_VAL
};

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
bool WF_TxPacketAllocate(u_int16_t bytes_needed)
{
    unsigned buf_avail, nbytes;
    u_int32_t startTime;

    EnsureWFisAwake();

    // allocate an extra 4 bytes for WiFi message preamble
    bytes_needed += WF_TX_PREAMBLE_SIZE;

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
        nbytes = mrf_raw_move(RAW_DATA_TX_ID, RAW_DATA_POOL, 1, bytes_needed);
        if (nbytes > 0) {
            // set the RAW index at 4 to leave room for internal 4 byte header.  Ethernet
            // packet data starts at index 4.
            mrf_raw_seek(RAW_DATA_TX_ID, 4);
            return 1;
        }

        printf("--- %s: failed\n", __func__);
        udelay(10);
    }
    return 0;
}

void WF_TxPacketCopy(u_int8_t *buf, u_int16_t length)
{
    mrf_raw_write(RAW_DATA_TX_ID, buf, length);
}

void WF_TxPacketTransmit(u_int16_t packetSize)
{
    /* create internal preamble */
    u_int8_t txDataPreamble[4] = {WF_DATA_REQUEST_TYPE, WF_STD_DATA_MSG_SUBTYPE, 1, 0};

    EnsureWFisAwake();

    // write out internal preamble, starting at index 0 in the raw window
    mrf_raw_pwrite(RAW_DATA_TX_ID, txDataPreamble, sizeof(txDataPreamble), 0);

    // overwrite the Ethernet source address in Tx packet with SNAP header
    // (starts at index 10 within packet, after 4 bytes of preamble and 6 bytes of dest address)
    mrf_raw_pwrite(RAW_DATA_TX_ID, snapHdr, SNAP_SIZE,
        sizeof(txDataPreamble) + WF_MAC_ADDRESS_LENGTH);

    // Notify WiFi device that there is a transmit frame to send.  The frame will
    // be automatically deallocated after RF transmission is completed.
    mrf_raw_move(RAW_DATA_TX_ID, RAW_MAC, 0, packetSize + sizeof(txDataPreamble));

    // this raw window is logically no longer mounted.  The WiFi chip will
    // automatically deallocate after RF transmission.
    //SetRawDataWindowState(RAW_DATA_TX_ID, WF_RAW_UNMOUNTED);
}

u_int16_t WF_RxPacketLengthGet()
{
    u_int16_t len;
    t_rxPreamble rxPreamble;

    /* Mount Read FIFO to RAW Rx window.  Allows use of RAW engine to read rx data packet. */
    /* Function call returns number of bytes in the data packet.                           */
    len = mrf_raw_move(RAW_DATA_RX_ID, RAW_MAC, 1, 0);
    if (len == 0) {
        printf("--- %s: failed\n", __func__);
        return 0;
    }

    // read the data frame internal preamble (type and subtype) to verify that we did, in
    // fact, mount an Rx data packet.  This read auto-increments the raw index to the first
    // actual data byte in the frame.
    mrf_raw_read(RAW_DATA_RX_ID, (u_int8_t*) &rxPreamble, sizeof(t_rxPreamble));
    if (rxPreamble.type != WF_DATA_RX_INDICATE_TYPE) {
        printf("--- %s: invalid preamble type=%u\n", __func__, rxPreamble.type);
        return 0;
    }

    // set raw pointer to start of 802.11 payload (start of Ethernet packet)
    mrf_raw_seek(RAW_DATA_RX_ID, sizeof(t_rxPreamble) + ENC_PREAMBLE_OFFSET);

    return len - sizeof(t_rxPreamble) - ENC_PREAMBLE_OFFSET;
}

/*
 * Deallocate a Data Rx buffer.
 */
void WF_RxPacketDeallocate()
{
    mrf_raw_move(RAW_DATA_RX_ID, RAW_DATA_POOL, 0, 0);
}

void WF_RxPacketCopy(u_int8_t *buf, u_int16_t length)
{
    mrf_raw_read(RAW_DATA_RX_ID, buf, length);
}
