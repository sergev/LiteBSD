/*
 * MRF24WG parameter message processing.
 *
 * Functions pertaining MRF24WG set/get parameter message processing.
 * Parameter messages are a subset of management messages.
 */
#include <stdio.h>
#include <string.h>
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * Send a SetParam Mgmt request to MRF24W and waits for response.
 * MACInit must be called first.
 *
 * Index Set Param Request
 * ----- -----------------
 *  0    type            (always 0x02 signifying a mgmt request)
 *  1    subtype         (always 0x10 signifying a Set Param Msg)
 *  2    param ID [msb]  (MS byte of parameter ID being requested, e.g.
 *                        PARAM_SYSTEM_VERSION)
 *  3    param ID [lsb]  (LS byte of parameter ID being requested. e.g.
 *                        PARAM_SYSTEM_VERSION)
 *  4    payload[0]      first byte of param data
 *  N    payload[n]      Nth byte of payload data
 *
 * Index  Set Param Response
 * ------ ------------------
 *  0     type           (always 0x02 signifying a mgmt response)
 *  1     subtype        (always 0x10 signifying a Param Response Msg
 *  2     result         (1 if successful -- any other value indicates failure
 *  3     mac state      (not used)
 *
 * Parameters:
 *  paramType - Parameter type associated with the SetParam msg.
 *  p_paramData - pointer to parameter data
 *  paramDataLength - Number of bytes pointed to by p_paramData
 */
static void SendSetParamMsg(uint8_t paramType,
                            uint8_t *p_paramData,
                            uint8_t paramDataLength)
{
    uint8_t hdr[4];

    hdr[0] = WF_MGMT_REQUEST_TYPE;
    hdr[1] = WF_SET_PARAM_SUBTYPE;
    hdr[2] = 0x00;                 /* MS 8 bits of param Id, always 0 */
    hdr[3] = paramType;            /* LS 8 bits of param ID           */

    SendMgmtMsg(hdr,               /* header            */
                sizeof(hdr),       /* size of header    */
                p_paramData,       /* param data        */
                paramDataLength);  /* param data length */

    /* wait for MRF24W management response; free response because not needed */
    WaitForMgmtResponse(WF_SET_PARAM_SUBTYPE, FREE_MGMT_BUFFER);
}

/*
 * Send a GetParam Mgmt request to MRF24W and waits for response.
 * After response is received the param data is read from message and written
 * to p_paramData.  It is up to the caller to fix up endianness.
 * MACInit must be called first.
 *
 * Index Get Param Request
 * ----- -----------------
 *  0    type            (always 0x02 signifying a mgmt request)
 *  1    subtype         (always 0x10 signifying a Get Param Msg)
 *  2    param ID [msb]  (MS byte of parameter ID being requested, e.g.
 *                        PARAM_SYSTEM_VERSION)
 *  3    param ID [lsb]  (LS byte of parameter ID being requested, e.g.
 *                        PARAM_SYSTEM_VERSION)
 *
 * Index  Get Param Response
 * ------ ------------------
 *  0     type           (always 0x02 signifying a mgmt response)
 *  1     subtype        (always 0x10 signifying a Param Response Msg
 *  2     result         (1 if successful -- any other value indicates failure
 *  3     mac state      (not used)
 *  4     data length    Length of response data starting at index 6 (in bytes)
 *  5     not used
 *  6     Data[0]        first byte of returned parameter data
 *  N     Data[N]        Nth byte of param data
 */
static void SendGetParamMsg(uint8_t paramType, uint8_t *p_paramData, uint8_t paramDataLength)
{
    uint8_t hdr[4];

    hdr[0] = WF_MGMT_REQUEST_TYPE;
    hdr[1] = WF_GET_PARAM_SUBTYPE;
    hdr[2] = 0x00;                      /* MS 8 bits of param Id, always 0 */
    hdr[3] = paramType;                 /* LS 8 bits of param ID           */

    SendMgmtMsg(hdr,             /* header           */
                sizeof(hdr),     /* size of header   */
                NULL,            /* no data          */
                0);              /* no data          */

    WaitForMgmtResponseAndReadData(WF_GET_PARAM_SUBTYPE,       /* expected subtype                           */
                                   paramDataLength,            /* num data bytes to read                     */
                                   MSG_PARAM_START_DATA_INDEX, /* data for GetParam always starts at index 6 */
                                   p_paramData);               /* write the response data here               */
}

/*
 * Must be called to configure the MRF24W for operations.
 * MACInit must be called first.
 */
void WFEnableMRF24WB0MMode()
{
    uint8_t buf[1] = {ENABLE_MRF24WB0M};

    SendSetParamMsg(PARAM_MRF24WB0M, buf, sizeof(buf));
}

/*
 * Retrieve WF device information.
 * MACInit must be called first.
 *
 * Parameters:
 *  p_deviceInfo - Pointer where device info will be written
 */
void WF_DeviceInfoGet(t_deviceInfo *p_Info)
{
    uint8_t msgData[2];

    SendGetParamMsg(PARAM_SYSTEM_VERSION, msgData, sizeof(msgData));

    p_Info->romVersion   = msgData[0];
    p_Info->patchVersion = msgData[1];

    if (p_Info->romVersion == 0x12) {
        p_Info->deviceType = WF_MRF24WB_DEVICE;
    } else if (p_Info->romVersion >= 0x30) {
        p_Info->deviceType = WF_MRF24WG_DEVICE;  /* need part number */
    } else {
        p_Info->deviceType = WF_UNKNOWN_DEVICE;
    }
}

/*
 * Set the regional domain on the MRF24W.  By default the MRF24W will use the
 * factory-set regional domain.  It is invalid to call this function while in a
 * connected state.
 *
 * Valid values for the regional domain are:
 *  WF_DOMAIN_FCC (default)
 *  WF_DOMAIN_ETSI
 *  WF_DOMAIN_JAPAN
 *  WF_DOMAIN_OTHER
 *
 * Parameter:
 *  regionalDomain - desired regional domain
 */
void WF_RegionalDomainSet(uint8_t regionalDomain)
{
#if defined(WF_ERROR_CHECKING)
    uint32_t errorCode;

    errorCode = UdSetDomain(regionalDomain);
    if (errorCode != UD_SUCCESS) {
       EventEnqueue(WF_EVENT_ERROR, errorCode);
       return;
    }
#endif /* WF_ERROR_CHECKING */

    SendSetParamMsg(PARAM_REGIONAL_DOMAIN, &regionalDomain, 1);
}

/*
 * Get the regional domain currently in use.
 *
 * Parameters:
 *  p_regionalDomain - pointer to where regional domain is written
 */
void WF_RegionalDomainGet(uint8_t *p_regionalDomain)
{
    SendGetParamMsg(PARAM_REGIONAL_DOMAIN, p_regionalDomain, 1);
}

/*
 * Direct the MRF24W to use the input MAC address instead of its
 * factory-default MAC address.  This function does not overwrite the factory
 * default, which is in FLASH memory ? it simply tells the MRF24W to use a
 * different MAC.
 *
 * MACInit must be called first.  Cannot be called when the MRF24W is in a
 * connected state.
 *
 * Parameter:
 *  p_mac - Pointer to 6-byte MAC that will be sent to MRF24W
 */
void WF_MacAddressSet(uint8_t *p_mac)
{
#if defined(WF_ERROR_CHECKING)
    // can't change MAC address unless not connected
    if (UdGetConnectionState() != CS_NOT_CONNECTED) {
        EventEnqueue(WF_EVENT_ERROR, UD_ERROR_ONLY_VALID_WHEN_NOT_CONNECTED);
        return;
    }
#endif
    SendSetParamMsg(PARAM_MAC_ADDRESS, p_mac, WF_MAC_ADDRESS_LENGTH);
}

void WF_MacAddressGet(uint8_t *p_macAddress)
{
    SendGetParamMsg(PARAM_MAC_ADDRESS, p_macAddress, WF_MAC_ADDRESS_LENGTH);
}

/*
 * Enables or disables the MRF24W Tx data confirm mgmt message.
 * Data confirms should always be disabled.
 * MACInit must be called first.
 *
 * Parameters:
 *  state - WF_DISABLED or WF_ENABLED
 */
void WF_SetTxDataConfirm(uint8_t state)
{
    SendSetParamMsg(PARAM_CONFIRM_DATA_TX_REQ, &state, 1);
}

void WF_TxModeSet(uint8_t mode)
{
#if defined(WF_ERROR_CHECKING)
    uint32_t errorCode = UdSetTxMode(mode);
    if (errorCode != UD_SUCCESS) {
        EventEnqueue(WF_EVENT_ERROR, errorCode);
    }
#endif
    SendSetParamMsg(PARAM_TX_MODE, &mode, 1);
}

void WF_RtsThresholdSet(uint16_t rtsThreshold)
{
    uint16_t tmp;

#if defined(WF_ERROR_CHECKING)
    uint32_t errorCode = UdSetRtsThreshold(rtsThreshold);
    if (errorCode != UD_SUCCESS) {
        EventEnqueue(WF_EVENT_ERROR, errorCode);
        return;
    }
#endif

    tmp = htons(rtsThreshold);
    SendSetParamMsg(PARAM_RTS_THRESHOLD, (uint8_t *)&tmp, sizeof(tmp));
}

/*
 * Allow host to convert pass phrase to key in WPS WPA-PSK.
 * Only applicable if WPSDirects the MRF24WG to send, if applicable,
 * the ASCII WPA-PSK passphrase to the
 * host so the host can calculate the binary key.
 *
 * MACInit must be called first.
 */
void YieldPassPhraseToHost()
{
    uint8_t yield = 1;

    SendSetParamMsg(PARAM_YIELD_PASSPHRASE_TOHOST, &yield, sizeof(yield));
}

void SetPSK(uint8_t *psk)
{
    SendSetParamMsg(PARAM_SET_PSK, psk, WF_WPA_KEY_LENGTH);
}

/*
 * Get MAC statistics.
 * MACInit must be called first.
 *
 * Parameters:
 *  p_macStats - Pointer to where MAC statistics are written
 */
void WF_MacStatsGet(t_macStats *p_macStats)
{
    uint32_t *p_value;
    uint8_t  numElements;
    uint8_t  i;

    SendGetParamMsg(PARAM_STAT_COUNTERS, (uint8_t *)p_macStats, sizeof(t_macStats));

    // calculate number of 32-bit counters in the stats structure and point to first element
    numElements = sizeof(t_macStats) / sizeof(uint32_t);
    p_value = (uint32_t *)p_macStats;

    /* correct endianness on all counters in structure */
    for (i = 0; i < numElements; ++i) {
        *p_value = htonl(*p_value);
        ++p_value;
    }
}

/*
 * Configures the number of WiFi transmission failures that can occur before
 * the MRF24WG reports the connection is lost.
 *
 * There are two primary ways the MRF24WG can determine that it has lost the
 * link with an AP:
 *  1) beacon timeout
 *  2) receive deauth message from AP
 *
 * The deauth message, however, is not ack'd by the MRF24WG, so it is not
 * guaranteed that is will be received.  In this case, the MRF24WG will only
 * know it has lost the link because none of its future messages will be ack'd by
 * the AP.  This function allows the host to configure a 'missed ack' count, where,
 * if the MRF24WG misses N consecutive ack's from the AP it generates an event that
 * the connection is lost.
 *
 * The default threshold is 0, meaning that the MRF24WG ignores missed ack's, and
 * will simply keep trying to communicate with the AP.  It is important to note that
 * in normal operations, ack's from the AP are frequently missed, and retries are
 * quite common, so, if using this function, do not set the threshold to low because
 * then erroneous connection lost events will be generated.  It is recommended
 * that a threshold of 40 be used if this function is to be utilized.
 *
 * Parameter:
 *  threshold -- 0:     disable this feature; MRF24WG does not track missed ack's (default)
 *               1-255: after this many missed ack's, signal connection lost event
 */
void WF_LinkDownThresholdSet(uint8_t threshold)
{
    SendSetParamMsg(PARAM_LINK_DOWN_THRESHOLD, &threshold, sizeof(threshold));
}

/*
 * Set a multicast address filter using one of the two multicast filters.
 *
 * This function allows the application to configure up to 16 software Multicast
 * Address Filters on the MRF24WB0M.  If two active multicast filters are set
 * up they are OR'd together and the MRF24WG will receive and pass to the Host
 * CPU received packets from either multicast address.
 * The allowable values for the multicast filter are:
 *  WF_MULTICAST_FILTER_1
 *  WF_MULTICAST_FILTER_2
 *
 * By default, both multicast filters are inactive.
 *
 * Parameters:
 *  multicastFilterId - WF_MULTICAST_FILTER_1 or WF_MULTICAST_FILTER_2
 *  multicastAddress  - 6-byte address (all 0xFF will inactivate the filter)
 */
void WF_SetHwMultiCastFilter(uint8_t multicastFilterId,
                             uint8_t multicastAddress[WF_MAC_ADDRESS_LENGTH])
{
    int i;
    bool    deactivateFlag = true;
    uint8_t msgData[8];

#if defined(WF_ERROR_CHECKING)
    uint32_t errorCode = UdSetHwMulticastFilter(multicastFilterId, multicastAddress);
    if (errorCode != UD_SUCCESS) {
        EventEnqueue(WF_EVENT_ERROR, errorCode);
    }
#endif

    /* check if all 6 bytes of the address are 0xff, implying that the caller wants to deactivate */
    /* the multicast filter.                                                                      */
    for (i = 0; i < 6; ++i) {
        /* if any byte is not 0xff then a presume a valid multicast address */
        if (multicastAddress[i] != 0xff)
        {
            deactivateFlag = false;
            break;
        }
    }

    msgData[0] = multicastFilterId;     /* Address Compare Register number to use   */
    if (deactivateFlag) {
        msgData[1] = ADDRESS_FILTER_DEACTIVATE;
    } else {
        msgData[1] = MULTICAST_ADDRESS;     /* type of address being used in the filter */
    }

    memcpy(&msgData[2], (void *)multicastAddress, WF_MAC_ADDRESS_LENGTH);
    SendSetParamMsg(PARAM_COMPARE_ADDRESS, msgData, sizeof(msgData) );
}

/*
 * Retrieve the factory-set max Tx power from the MRF24WB0MA/B and MRF24WG0MA/B.
 * MACInit must be called first.
 *
 * Parameter:
 *  p_factoryMaxTxPower - desired maxTxPower ( 0 to 18 dBm), in 1dB steps
 */
uint8_t GetFactoryMax()
{
    uint8_t msgData[2];

    /* read max and min factory-set power levels */
    SendGetParamMsg(PARAM_FACTORY_SET_TX_MAX_POWER, msgData, sizeof(msgData));

    /* msgData[0] = max power, msgData[1] = min power */
    return msgData[0];
}
