/*
 * MRF24WG management message processing.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

static volatile bool gMgmtConfirmMsgReceived = false;

void ClearMgmtConfirmMsg()
{
    gMgmtConfirmMsgReceived = false;
}

static void SignalMgmtConfirmMsg()
{
    gMgmtConfirmMsgReceived = true;
}

static bool isMgmtConfirmMsg()
{
    return gMgmtConfirmMsgReceived;
}

static void WFProcessMgmtIndicateMsg()
{
    t_mgmtIndicateHdr hdr;
    uint8_t buf[6];
    uint8_t eventType = 0xff;
    uint32_t eventData = WF_NO_EVENT_DATA;

    /* read 2-byte header of management message */
    RawRead(RAW_MGMT_RX_ID, 0, sizeof(t_mgmtIndicateHdr), (uint8_t *)&hdr);

    /* Determine which event occurred and handle it */
    switch (hdr.subType)
    {
        //----------------------------------------------
        case WF_EVENT_CONNECTION_ATTEMPT_STATUS_SUBTYPE:
        //----------------------------------------------
            RawRead(RAW_MGMT_RX_ID, sizeof(t_mgmtIndicateHdr), 2, buf); /* read first 2 bytes after header */
            // if connection attempt successful
            if (buf[0] == CONNECTION_ATTEMPT_SUCCESSFUL)
            {
                eventType = WF_EVENT_CONNECTION_SUCCESSFUL;
                eventData = WF_NO_EVENT_DATA;
                UdSetConnectionState(CS_CONNECTED);
            }
            /* else connection attempt failed */
            else
            {
                eventType = WF_EVENT_CONNECTION_FAILED;
                eventData = ((uint32_t)buf[0] << 8) | (uint32_t)buf[1]; /* contains connection failure code */
                UdSetConnectionState(CS_NOT_CONNECTED);
            }
        break;

        //------------------------------------
        case WF_EVENT_CONNECTION_LOST_SUBTYPE:
        //------------------------------------
            /* read index 2 and 3 from message and store in buf[0] and buf[1]
               buf[0] -- 1: Connection temporarily lost  2: Connection permanently lost 3: Connection Reestablished
               buf[1] -- 0: Beacon Timeout  1: Deauth from AP  */
            RawRead(RAW_MGMT_RX_ID, sizeof(t_mgmtIndicateHdr), 2, buf);

            if (buf[0] == CONNECTION_TEMPORARILY_LOST)
            {
                eventType     = WF_EVENT_CONNECTION_TEMPORARILY_LOST;
                eventData = (uint32_t)buf[1];    /* lost due to beacon timeout or deauth */
                UdSetConnectionState(CS_CONNECTION_IN_PROGRESS);
            }
            else if (buf[0] == CONNECTION_PERMANENTLY_LOST)
            {
                eventType     = WF_EVENT_CONNECTION_PERMANENTLY_LOST;
                eventData = (uint32_t)buf[1];   /* lost due to beacon timeout or deauth */
                UdSetConnectionState(CS_NOT_CONNECTED);
            }
            else if (buf[0] == CONNECTION_REESTABLISHED)
            {
                eventType     = WF_EVENT_CONNECTION_REESTABLISHED;
                eventData = (uint32_t)buf[1];    /* originally lost due to beacon timeout or deauth */
                UdSetConnectionState(CS_CONNECTED);
            }
            else
            {
                /* invalid parameter in received mgmt indicate message */
                EventEnqueue(WF_EVENT_ERROR, UD_ERROR_BAD_PARAM_IN_CONN_LOST_EVENT);
            }
            break;

        //---------------------------------------
        case WF_EVENT_SCAN_RESULTS_READY_SUBTYPE:
        //---------------------------------------
            /* read index 2 of mgmt indicate to get the number of scan results */
            RawRead(RAW_MGMT_RX_ID, sizeof(t_mgmtIndicateHdr), 1, buf);
            eventType = WF_EVENT_SCAN_RESULTS_READY;
            eventData = (uint32_t)buf[0];          /* number of scan results */
            break;

#if 0
        /*-----------------------------------------------------------------*/
        case WF_EVENT_SCAN_IE_RESULTS_READY_SUBTYPE:
        /*-----------------------------------------------------------------*/
            eventType = WF_EVENT_IE_RESULTS_READY;
            /* read indexes 2 and 3 containing the 16-bit value of IE bytes */
            RawRead(RAW_MGMT_RX_ID, sizeof(tMgmtIndicateHdr), 2, (UINT8 *)&eventInfo);
            eventData = WFSTOHS(eventInfo);     /* fix endianess of 16-bit value */
            break;
#endif

        //--------------------------------------------
        case WF_EVENT_KEY_CALCULATION_REQUEST_SUBTYPE:
        //--------------------------------------------
            eventType = WF_WPS_EVENT_KEY_CALCULATION_REQUEST;
            eventData = WF_NO_EVENT_DATA;
            // read the passphrase data into the structure provided during WF_SetSecurityWps()
            RawRead(RAW_MGMT_RX_ID,
                    sizeof(t_mgmtIndicateHdr),
                    sizeof(t_wpaKeyInfo),
                    (uint8_t *)GetWpsPassPhraseInfo());
            break;

        //------
        default:
        //------
            eventType = WF_EVENT_ERROR;
            eventData = UD_ERROR_UNKNOWN_EVENT_TYPE;
            break;
    }

    /* free mgmt buffer */
    DeallocateMgmtRxBuffer();

    EventEnqueue(eventType, eventData);
}

void SignalMgmtMsgRx()
{
    uint8_t msgType;

    // mount the mgmt pool rx data.  Read index is set to 0.
    RawMountRxBuffer(RAW_MGMT_RX_ID);

    // read first byte from this mgmt msg (msg type)
    RawRead(RAW_MGMT_RX_ID, 0, 1, &msgType);

    // if a mgmt confirm then the Universal Driver is waiting for it.  Don't
    // process it here, but signal the driver that it happened.
    if (msgType == WF_MGMT_CONFIRM_TYPE)
    {
        SignalMgmtConfirmMsg();
    }
    // if a mgmt indicated occurred (asynchronous event), then process it right
    // now
    else if (msgType == WF_MGMT_INDICATE_TYPE)
    {
        WFProcessMgmtIndicateMsg();
    }
    // else an unknown mgmt msg type was received
    else
    {
        EventEnqueue(WF_EVENT_ERROR, UD_ERROR_INVALID_MGMT_TYPE);
    }

    WF_EintEnable();
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
void SendMgmtMsg(uint8_t *p_header, uint8_t headerLength,
    uint8_t *p_data, uint8_t dataLength)
{
    uint32_t elapsedTime;
    uint32_t startTime;

    EnsureWFisAwake();

    startTime = WF_TimerRead();
    while (AllocateMgmtTxBuffer(WF_MAX_TX_MGMT_MSG_SIZE) == false)
    {
         // if timed out waiting for allocation of Mgmt Tx Buffer
         elapsedTime = GetElapsedTime(startTime, WF_TimerRead());
         if (elapsedTime > 15)
         {
            EventEnqueue(WF_EVENT_ERROR, UD_ERROR_MGMT_BUFFER_ALLOCATION_FAILED);
            return;
         }
    }

    /* write out management header */
    RawSetByte(RAW_MGMT_TX_ID, p_header, headerLength);

    /* write out data (if any) */
    if (dataLength > 0)
    {
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
void WaitForMgmtResponse(uint8_t expectedSubtype, uint8_t freeAction)
{
    uint32_t startTime, elapsedTime;
#if 1
    #if defined(__18CXX)
        static tMgmtMsgRxHdr  hdr;
    #else
        t_mgmtMsgRxHdr  hdr;
    #endif


    /* Wait until mgmt response is received */
    startTime = WF_TimerRead();
    while ( !isMgmtConfirmMsg() )
    {
        InterruptCheck();   // check if an interrupt has occurred (and process it)

        /* if received a data packet while waiting for mgmt response packet */
        if ( isPacketRx() )
        {
            // We can't let the StackTask processs data messages that come in while waiting for mgmt
            // response because the application might send another mgmt message, which is illegal until the response
            // is received for the first mgmt msg.  And, we can't prevent the race condition where a data message
            // comes in before a mgmt response is received.  Thus, the only solution is to throw away a data message
            // that comes in while waiting for a mgmt response.  This should happen very infrequently.  If using TCP then the
            // stack takes care of retries.  If using UDP, the application has to deal with occasional data messages not being
            // received.  Also, applications typically do not send a lot of management messages after connected.

            // TODO: may be a away to avoid doing this when the data tx/rx, with their
            // buffers, are available.  Can just copy the data message to buffer.
            // throw away the data rx
            RawMountRxBuffer(RAW_DATA_RX_ID);
            DeallocateDataRxBuffer();
            ClearPacketRx();

            /* ensure interrupts enabled */
            WF_EintEnable();
        }

        elapsedTime = GetElapsedTime(startTime, WF_TimerRead());
        if (elapsedTime > 50)
        {
            EventEnqueue(WF_EVENT_ERROR, UD_ERROR_MGMT_RESPONSE_TIMEOUT);
            return;
        }
    }
    ClearMgmtConfirmMsg();


    /* if the caller wants to delete the response immediately (doesn't need any data from it */
    if (freeAction == FREE_MGMT_BUFFER)
    {
        /* read and verify result before freeing up buffer to ensure our message send was successful */
        RawRead(RAW_MGMT_RX_ID, 0, (uint16_t)(sizeof(t_mgmtMsgRxHdr)), (uint8_t *)&hdr);

        if (hdr.subtype != expectedSubtype)
        {
            EventEnqueue(WF_EVENT_ERROR, UD_ERROR_INVALID_MGMT_SUBTYPE);
        }

        if (hdr.result != MGMT_RESP_SUCCESS)
        {
            EventEnqueue(WF_EVENT_ERROR, hdr.result);
        }

        /* free mgmt buffer */
        DeallocateMgmtRxBuffer();
    }
#endif
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
void WaitForMgmtResponseAndReadData(uint8_t expectedSubtype,
                                    uint8_t numDataBytes,
                                    uint8_t startIndex,
                                    uint8_t *p_data)
{
    t_mgmtMsgRxHdr  hdr;  /* management msg header struct */

    WaitForMgmtResponse(expectedSubtype, DO_NOT_FREE_MGMT_BUFFER);

    // if made it here then received a management message; read out header
    RawRead(RAW_MGMT_RX_ID, 0, (uint16_t)(sizeof(t_mgmtMsgRxHdr)), (uint8_t *)&hdr);

    if ((hdr.result != MGMT_RESP_SUCCESS) && (hdr.result != MGMT_RESP_ERROR_NO_STORED_BSS_DESCRIPTOR))
    {
        EventEnqueue(WF_EVENT_ERROR, hdr.result);
    }

    if (hdr.subtype != expectedSubtype)
    {
        EventEnqueue(WF_EVENT_ERROR, UD_ERROR_INVALID_MGMT_SUBTYPE);
    }

    /* if caller wants to read data from this mgmt response */
    if (numDataBytes > 0)
    {
        RawRead(RAW_MGMT_RX_ID, startIndex, numDataBytes, p_data);
    }

    /* free the mgmt buffer */
    DeallocateMgmtRxBuffer();
}
