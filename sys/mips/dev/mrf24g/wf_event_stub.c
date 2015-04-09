#include "wf_universal_driver.h"

//TODO: extern WFMRFD wfmrf24;

/*
 * Called by Universal Driver to inform application of MRF24WG events.
 *
 * Various events are reported to the application via this function callback.
 * The application should take appropriate action based on the event.
 *
 * Example 1) After WF_Init() is called, no other WiFI functions can be called,
 *            other than calling WF_Task(), until the WF_EVENT_INITIALIZATION
 *            occurs with eventData = WF_INIT_SUCCESSFUL.
 *
 * Example 2) If the WiFi connection is temporarily lost the application should
 *            not be sending messages until the WiFi connection is restored.
 *            If DHCP is being used, then, after the connection is restored, a
 *            new DHCP request should be initiated.
 *
 * Parameters:
 *   eventType -- type of event
 *   eventData -- data associated with event; not always used.
 */
void WF_ProcessEvent(uint8_t eventType, uint32_t eventData)
{
    //TODO
#if 0
    wfmrf24.priv.fMRFBusy = FALSE;
    wfmrf24.priv.lastEventType = eventType;
    wfmrf24.priv.lastEventData = eventData;

    switch (eventType) {
    case WF_EVENT_INITIALIZATION:
        if (eventData == WF_INIT_SUCCESSFUL)
        {
            wfmrf24.priv.initStatus = ForceIPStatus((InitMask | eventData));
        }
        else
        {
            wfmrf24.priv.initStatus = ForceIPError((InitMask | eventData));
        }
        break;

    case WF_EVENT_CONNECTION_SUCCESSFUL:
        wfmrf24.priv.connectionStatus = ipsSuccess;
        break;

    case WF_EVENT_CONNECTION_TEMPORARILY_LOST:
        wfmrf24.priv.connectionStatus = ForceIPStatus((CLMask | eventData));
        wfmrf24.priv.fMRFBusy = true;   // don't do anything during the reconnect!
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
        WF_WpsKeyGenerate(); // can be called here or later, but must be called
                             // to complete WPS connection
        wfmrf24.priv.fMRFBusy = TRUE;   // wait for connection status or error.
        break;

    case WF_EVENT_MRF24WG_MODULE_ASSERT:
        //TODO: OutputMrf24wgAssertInfo(eventData);
        break;

    // if we get an event error, it will be in
    // the last eventData
    case WF_EVENT_ERROR:
    default:
        break;
    }
#endif
}

/*
 * Called by Universal Driver to notify application of incoming packet.
 */
void WF_ProcessRxPacket()
{
    //TODO
#if 0
    uint16_t cbPkt = WF_RxPacketLengthGet();

    if (cbPkt > 0) {
        IPSTACK * pIpStack = RRHPAlloc(wfmrf24.adpMRF24G.hAdpHeap, cbPkt + sizeof(IPSTACK));
        if (pIpStack != NULL) {
            // fill in info about the frame data
            pIpStack->fFrameIsParsed    = FALSE;
            pIpStack->fFreeIpStackToAdp = TRUE;
            pIpStack->headerOrder       = NETWORK_ORDER;
            pIpStack->pPayload          = ((uint8_t *) pIpStack) + sizeof(IPSTACK);
            pIpStack->cbPayload         = cbPkt;

            WF_RxPacketCopy(pIpStack->pPayload, pIpStack->cbPayload);
            WF_RxPacketDeallocate();

            pIpStack->fOwnedByAdp = true;
            FFInPacket(&wfmrf24.priv.ffptRead, pIpStack);
        }
        // if we know we can never allocate this packet, then just drop it
        else {
            WF_RxPacketDeallocate();
        }
    }
#endif
}
