/*
 * MRF24WG management message processing.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

static volatile bool gMgmtConfirmMsgReceived = 0;

void ClearMgmtConfirmMsg()
{
    gMgmtConfirmMsgReceived = 0;
}

static void SignalMgmtConfirmMsg()
{
    gMgmtConfirmMsgReceived = 1;
}

static bool isMgmtConfirmMsg()
{
    return gMgmtConfirmMsgReceived;
}

static void WFProcessMgmtIndicateMsg()
{
    t_mgmtIndicateHdr hdr;
    u_int8_t buf[6];

    /* read 2-byte header of management message */
    RawRead(RAW_MGMT_RX_ID, 0, sizeof(t_mgmtIndicateHdr), (u_int8_t *)&hdr);

    /* Determine which event occurred and handle it */
    switch (hdr.subType) {
    case WF_EVENT_CONNECTION_ATTEMPT_STATUS_SUBTYPE:
        RawRead(RAW_MGMT_RX_ID, sizeof(t_mgmtIndicateHdr), 2, buf); /* read first 2 bytes after header */

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
        RawRead(RAW_MGMT_RX_ID, sizeof(t_mgmtIndicateHdr), 2, buf);

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
        RawRead(RAW_MGMT_RX_ID, sizeof(t_mgmtIndicateHdr), 1, buf);
        printf("--- %s: scan results ready, count=%u\n", __func__, buf[0]);
        break;

    case WF_EVENT_KEY_CALCULATION_REQUEST_SUBTYPE:
        printf("--- %s: key calculation finished\n", __func__);
        // read the passphrase data into the structure provided during WF_SetSecurityWps()
        RawRead(RAW_MGMT_RX_ID, sizeof(t_mgmtIndicateHdr),
            sizeof(t_wpaKeyInfo), (u_int8_t *)GetWpsPassPhraseInfo());
        break;

    default:
        printf("--- %s: unknown mgmt indicate message, subtype=%u\n", __func__, hdr.subType);
        break;
    }

    /* free mgmt buffer */
    DeallocateMgmtRxBuffer();
}

void SignalMgmtMsgRx()
{
    u_int8_t msgType;

    // mount the mgmt pool rx data.  Read index is set to 0.
    RawMountRxBuffer(RAW_MGMT_RX_ID);

    // read first byte from this mgmt msg (msg type)
    RawRead(RAW_MGMT_RX_ID, 0, 1, &msgType);

    switch (msgType) {
    case WF_MGMT_CONFIRM_TYPE:
        // if a mgmt confirm then the Universal Driver is waiting for it.  Don't
        // process it here, but signal the driver that it happened.
        SignalMgmtConfirmMsg();
        break;
    case WF_MGMT_INDICATE_TYPE:
        // if a mgmt indicated occurred (asynchronous event),
        // then process it right now
        WFProcessMgmtIndicateMsg();
        break;
    default:
        // unknown mgmt msg type was received
        printf("--- %s: unknown mgmt message received, type=%u\n", __func__, msgType);
        break;
    }
}

/*
 * Send a management message
 *
 * Parameters:
 *  p_header      -- pointer to mgmt message header data
 *  headerLength  -- number of bytes in the header
 *                   will be written
 *  p_data        -- pointer to mgmt message data
 *  dataLength    -- number of byte of data
 */
void SendMgmtMsg(u_int8_t *p_header, u_int8_t headerLength,
    u_int8_t *p_data, u_int8_t dataLength)
{
    u_int32_t startTime;

    EnsureWFisAwake();

    startTime = WF_TimerRead();
    while (AllocateMgmtTxBuffer(WF_MAX_TX_MGMT_MSG_SIZE) == 0) {
         // if timed out waiting for allocation of Mgmt Tx Buffer
         if (WF_TimerElapsed(startTime) > 15) {
            printf("--- %s: buffer allocation failed\n", __func__);
            return;
         }
    }

    /* write out management header */
    RawSetByte(RAW_MGMT_TX_ID, p_header, headerLength);

    /* write out data (if any) */
    if (dataLength > 0) {
        RawSetByte(RAW_MGMT_TX_ID, p_data, dataLength);
    }

    ClearMgmtConfirmMsg();  // haven't received the confirm message yet

    /* signal MRF24W that mgmt message is ready to be processed */
    SendRAWManagementFrame(headerLength + dataLength);
}

/*
 * Called after sending a mgmt request.  This function waits for a mgmt
 * response.  The caller can optionally request the the management
 * response be freed immediately (by this function) or not freed.  If not
 * freed the caller is responsible to free the response buffer.
 *
 * Parameters:
 *  expectedSubtype -- The expected subtype of the mgmt response
 *  freeAction      -- FREE_MGMT_BUFFER or DO_NOT_FREE_MGMT_BUFFER
 */
void WaitForMgmtResponse(u_int8_t expectedSubtype, u_int8_t freeAction)
{
    unsigned startTime, intr;
    t_mgmtMsgRxHdr  hdr;

    /* Wait until mgmt response is received */
    startTime = WF_TimerRead();
    for (;;) {
        intr = WF_ReadByte(WF_HOST_INTR_REG);

        // if received a level 2 interrupt
        if (intr & WF_HOST_INT_MASK_INT2) {
            // Either a mgmt tx or mgmt rx Raw move complete occurred
            /* clear this interrupt */
            WF_Write(WF_HOST_INTR2_REG, WF_HOST_INT2_MASK_RAW_2 |
                                        WF_HOST_INT2_MASK_RAW_3 |
                                        WF_HOST_INT2_MASK_RAW_4 |
                                        WF_HOST_INT2_MASK_RAW_5);
            WF_WriteByte(WF_HOST_INTR_REG, WF_HOST_INT_MASK_INT2);
        }
        if (intr & WF_HOST_INT_MASK_FIFO_1) {
            /* got a FIFO 1 Threshold interrupt (Management Fifo).  Mgmt Rx msg ready to proces. */
            /* clear this interrupt */
            WF_WriteByte(WF_HOST_INTR_REG, WF_HOST_INT_MASK_FIFO_1);

            // signal that a mgmt msg, either confirm or indicate, has been received
            // and needs to be processed
            SignalMgmtMsgRx();
            if (isMgmtConfirmMsg())
                break;
        }

        if (WF_TimerElapsed(startTime) > 50) {
            printf("--- %s: timeout waiting for response\n", __func__);
            return;
        }
    }
    ClearMgmtConfirmMsg();

    /* if the caller wants to delete the response immediately (doesn't need any data from it */
    if (freeAction == FREE_MGMT_BUFFER) {
        /* read and verify result before freeing up buffer to ensure our message send was successful */
        RawRead(RAW_MGMT_RX_ID, 0, (u_int16_t)(sizeof(t_mgmtMsgRxHdr)), (u_int8_t *)&hdr);

        if (hdr.result != MGMT_RESP_SUCCESS) {
            printf("--- %s: mgmt response failed, result=%u\n", __func__, hdr.result);
        }
        else if (hdr.subtype != expectedSubtype) {
            printf("--- %s: invalid mgmt response subtype=%u, expected=%u\n",
                __func__, hdr.subtype, expectedSubtype);
        }

        /* free mgmt buffer */
        DeallocateMgmtRxBuffer();
    }
}

/*
 * Wait for the mgmt response message and validate it by:
 *  1) checking the result field
 *  2) verifying that the received subtype matches the execpted subtype
 *
 * In addition, this function reads the desired number of data bytes from
 * the mgmt response, copies them to p_data, and then frees the mgmt buffer.
 *
 * Parameters:
 *  expectedSubtype -- management message subtype that we are expecting
 *  p_data          -- pointer where any desired management data bytes
 *                     will be written
 *  numDataBytes    -- Number of data bytes from mgmt response to write to
 *                     p_data.  Data always starts at index 4 of mgmt response.
 *  skipDataRead    -- if TRUE, then no data will be read and the mgmt buffer will not
 *                     be freed.  If FALSE, the data will be read and the mgmt buffer
 *                     will be freed.
 */
void WaitForMgmtResponseAndReadData(u_int8_t expectedSubtype,
                                    u_int8_t numDataBytes,
                                    u_int8_t startIndex,
                                    u_int8_t *p_data)
{
    t_mgmtMsgRxHdr  hdr;  /* management msg header struct */

    WaitForMgmtResponse(expectedSubtype, DO_NOT_FREE_MGMT_BUFFER);

    // if made it here then received a management message; read out header
    RawRead(RAW_MGMT_RX_ID, 0, (u_int16_t)(sizeof(t_mgmtMsgRxHdr)), (u_int8_t *)&hdr);

    if (hdr.result != MGMT_RESP_SUCCESS &&
        hdr.result != MGMT_RESP_ERROR_NO_STORED_BSS_DESCRIPTOR) {
        printf("--- %s: mgmt response failed, result=%u\n", __func__, hdr.result);
    }
    else if (hdr.subtype != expectedSubtype) {
        printf("--- %s: invalid mgmt response subtype=%u, expected=%u\n",
            __func__, hdr.subtype, expectedSubtype);
    }

    /* if caller wants to read data from this mgmt response */
    if (numDataBytes > 0) {
        RawRead(RAW_MGMT_RX_ID, startIndex, numDataBytes, p_data);
    }

    /* free the mgmt buffer */
    DeallocateMgmtRxBuffer();
}
