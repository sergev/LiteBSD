/*
 * MRF24WG management message processing.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

static void WFProcessMgmtIndicateMsg()
{
    t_mgmtIndicateHdr hdr;
    u_int8_t buf[6];

    /* read 2-byte header of management message */
    mrf_raw_pread(RAW_ID_MGMT_RX, (u_int8_t*) &hdr, sizeof(t_mgmtIndicateHdr), 0);

    /* Determine which event occurred and handle it */
    switch (hdr.subType) {
    case WF_EVENT_CONNECTION_ATTEMPT_STATUS_SUBTYPE:
        /* read first 2 bytes after header */
        mrf_raw_pread(RAW_ID_MGMT_RX, buf, 2, sizeof(t_mgmtIndicateHdr));

        if (buf[0] == CONNECTION_ATTEMPT_SUCCESSFUL) {
            // if connection attempt successful
            printf("--- %s: connection attempt successful\n", __func__);
            UdSetConnectionState(CS_CONNECTED);
        } else {
            /* else connection attempt failed */
            printf("--- %s: connection attempt failed, code=0x%x\n",
                __func__, (buf[0] << 8) | buf[1]);
            UdSetConnectionState(CS_NOT_CONNECTED);
        }
        break;

    case WF_EVENT_CONNECTION_LOST_SUBTYPE:
        /* read index 2 and 3 from message and store in buf[0] and buf[1]
           buf[0] -- 1: Connection temporarily lost  2: Connection permanently lost 3: Connection Reestablished
           buf[1] -- 0: Beacon Timeout  1: Deauth from AP  */
        mrf_raw_pread(RAW_ID_MGMT_RX, buf, 2, sizeof(t_mgmtIndicateHdr));

        switch (buf[0]) {
        case CONNECTION_TEMPORARILY_LOST:
            printf("--- %s: connection temporarily lost, code=0x%x\n", __func__, buf[1]);
            UdSetConnectionState(CS_CONNECTION_IN_PROGRESS);
            break;
        case CONNECTION_PERMANENTLY_LOST:
            printf("--- %s: connection lost, code=0x%x\n", __func__, buf[1]);
            UdSetConnectionState(CS_NOT_CONNECTED);
            break;
        case CONNECTION_REESTABLISHED:
            printf("--- %s: connection reestablished, code=0x%x\n", __func__, buf[1]);
            UdSetConnectionState(CS_CONNECTED);
            break;
        default:
            printf("--- %s: invalid parameter=%u in received mgmt indicate message\n", __func__, buf[0]);
            break;
        }
        break;

    case WF_EVENT_SCAN_RESULTS_READY_SUBTYPE:
        /* read index 2 of mgmt indicate to get the number of scan results */
        mrf_raw_pread(RAW_ID_MGMT_RX, buf, 1, sizeof(t_mgmtIndicateHdr));
        printf("--- %s: scan results ready, count=%u\n", __func__, buf[0]);
        break;

    case WF_EVENT_KEY_CALCULATION_REQUEST_SUBTYPE:
        printf("--- %s: key calculation finished\n", __func__);
        // read the passphrase data into the structure provided during WF_SetSecurityWps()
        mrf_raw_pread(RAW_ID_MGMT_RX, (u_int8_t *)GetWpsPassPhraseInfo(),
            sizeof(t_wpaKeyInfo), sizeof(t_mgmtIndicateHdr));
        break;

    default:
        printf("--- %s: unknown mgmt indicate message, subtype=%u\n", __func__, hdr.subType);
        break;
    }

    /* free mgmt buffer */
    mrf_raw_move(RAW_ID_MGMT_RX, RAW_MGMT_POOL, 0, 0);
}

/*
 * Receive mgmt Confirm message.
 */
int ReceiveMgmtConfirmMsg()
{
    unsigned len;
    u_int8_t msgType;

    // mount the mgmt pool rx data.
    len = mrf_raw_move(RAW_ID_MGMT_RX, RAW_MAC, 1, 0);
    if (len == 0) {
        printf("--- %s: failed\n", __func__);
        return 0;
    }

    // read first byte from this mgmt msg (msg type)
    mrf_raw_pread(RAW_ID_MGMT_RX, &msgType, 1, 0);

    switch (msgType) {
    case WF_TYPE_MGMT_CONFIRM:
        // if a mgmt confirm then the Universal Driver is waiting for it.
        //TODO: receive mgmt message
        return 1;
    case WF_TYPE_MGMT_INDICATE:
        // if a mgmt indicated occurred (asynchronous event),
        // then process it right now
        WFProcessMgmtIndicateMsg();
        return 0;
    default:
        // unknown mgmt msg type was received
        printf("--- %s: unknown mgmt message received, type=%u\n", __func__, msgType);
        return 0;
    }
}

/*
 * Send a management message, and wait for a response.
 * The caller can optionally request the the management
 * response be freed immediately (by this function) or not freed.
 * If not freed the caller is responsible to free the response buffer.
 *
 * Parameters:
 *  header        -- message header data
 *  header_len    -- number of bytes in the header
 *  data          -- message data
 *  data_len      -- number of data bytes
 *  free_response -- whether to deallocate response buffer immediately
 */
void mrf_mgmt_send(u_int8_t *header, unsigned header_len,
                   u_int8_t *data, unsigned data_len,
                   int free_response)
{
    unsigned start_time, elapsed_time, buf_avail, nbytes, intr;
    t_mgmtMsgRxHdr hdr;

    EnsureWFisAwake();
#if 0
    printf("--- %s: send %u+%u bytes: %02x", __func__, header_len, data_len, header[0]);
    int i;
    for (i=1; i<header_len; ++i)
        printf("-%02x", header[i]);
    for (i=0; i<data_len; ++i)
        printf("-%02x", data[i]);
    printf("\n");
#endif
    start_time = mrf_timer_read();
    for (;;) {
        /*
         * Allocate a Mgmt Tx buffer.
         */
        /* get total bytes available for MGMT tx memory pool */
        buf_avail = mrf_read(MRF24_REG_WFIFO_BCNT1) & FIFO_BCNT_MASK;

        /* if enough bytes available to allocate */
        if (buf_avail >= WF_MAX_TX_MGMT_MSG_SIZE) {
            /* allocate and create the new Mgmt Tx buffer */
            nbytes = mrf_raw_move(RAW_ID_MGMT_TX, RAW_MGMT_POOL, 1, WF_MAX_TX_MGMT_MSG_SIZE);
            if (nbytes > 0)
                break;
            /*printf("--- %s: cannot allocate %u bytes of %u free\n",
                __func__, WF_MAX_TX_MGMT_MSG_SIZE, buf_avail); */
        }

        elapsed_time = mrf_timer_elapsed(start_time);
        if (elapsed_time > 15) {
            // if timed out waiting for allocation of Mgmt Tx Buffer
            printf("--- %s: buffer allocation failed\n", __func__);
            return;
        }
        /* Don't poll too fast, give the chip some time for internal tasks */
        udelay(10);
    }

    /* write out management header */
    mrf_raw_write(RAW_ID_MGMT_TX, header, header_len);

    /* write out data (if any) */
    if (data_len > 0) {
        mrf_raw_write(RAW_ID_MGMT_TX, data, data_len);
    }

    /* signal MRF24W that mgmt message is ready to be processed */
    mrf_raw_move(RAW_ID_MGMT_TX, RAW_MAC, 0, header_len + data_len);

    /* Wait until mgmt response is received */
    start_time = mrf_timer_read();
    for (;;) {
        intr = mrf_read_byte(MRF24_REG_INTR);

        // if received a level 2 interrupt
        if (intr & INTR_INT2) {
            // Either a mgmt tx or mgmt rx Raw move complete occurred
            /* clear this interrupt */
            mrf_write(MRF24_REG_INTR2,
                INTR2_RAW2 | INTR2_RAW3 | INTR2_RAW4 | INTR2_RAW5);
            mrf_write_byte(MRF24_REG_INTR, INTR_INT2);
        }
        if (intr & INTR_FIFO1) {
            /* got a FIFO 1 Threshold interrupt (Management Fifo).  Mgmt Rx msg ready to proces. */
            /* clear this interrupt */
            mrf_write_byte(MRF24_REG_INTR, INTR_FIFO1);

            // signal that a mgmt msg, either confirm or indicate, has been received
            // and needs to be processed
            if (ReceiveMgmtConfirmMsg())
                break;
        }

        elapsed_time = mrf_timer_elapsed(start_time);
        if (elapsed_time > 50) {
            printf("--- %s: timeout waiting for response\n", __func__);
            return;
        }
        udelay(10);
    }

    /* if the caller wants to delete the response immediately (doesn't need any data from it */
    if (free_response) {
        /* read and verify result before freeing up buffer to ensure our message send was successful */
        mrf_raw_pread(RAW_ID_MGMT_RX, (u_int8_t*) &hdr, sizeof(t_mgmtMsgRxHdr), 0);

        if (hdr.result != MGMT_RESP_SUCCESS) {
            printf("--- %s: mgmt response failed, result=%u\n", __func__, hdr.result);
        }
        else if (hdr.subtype != header[1]) {
            printf("--- %s: invalid mgmt response subtype=%u, expected=%u\n",
                __func__, hdr.subtype, header[1]);
        }

        /* free mgmt buffer */
        mrf_raw_move(RAW_ID_MGMT_RX, RAW_MGMT_POOL, 0, 0);
    }
}

/*
 * Send a management message, and wait for a response.
 * Read the desired number of data bytes from the mgmt response,
 * copy them to p_data, and then free the mgmt buffer.
 *
 * Parameters:
 *  header     -- pointer to mgmt message header data
 *  header_len -- number of bytes in the header
 *  reply      -- pointer where any received management data will be written
 *  reply_len  -- number of data bytes to receive
 *  offset     -- skip this number of bytes in receive data
 */
void mrf_mgmt_send_receive(u_int8_t *message, unsigned message_len,
        u_int8_t *reply, unsigned reply_len, unsigned offset)
{
    t_mgmtMsgRxHdr hdr;     /* header of reply message */

    mrf_mgmt_send(message, message_len, 0, 0, 0);

    /* Read out header of the received response. */
    mrf_raw_pread(RAW_ID_MGMT_RX, (u_int8_t*) &hdr, sizeof(hdr), 0);

    if (hdr.result != MGMT_RESP_SUCCESS &&
        hdr.result != MGMT_RESP_ERROR_NO_STORED_BSS_DESCRIPTOR) {
        printf("--- %s: mgmt response failed, result=%u\n", __func__, hdr.result);
    }
    else if (hdr.subtype != message[1]) {
        printf("--- %s: invalid mgmt response subtype=%u, expected=%u\n",
            __func__, hdr.subtype, message[1]);
    }

    /* if caller wants to read data from this mgmt response */
    if (reply_len > 0) {
        mrf_raw_pread(RAW_ID_MGMT_RX, reply, reply_len, offset);
    }

    /* free the mgmt buffer */
    mrf_raw_move(RAW_ID_MGMT_RX, RAW_MGMT_POOL, 0, 0);
}
