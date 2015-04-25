/*
 * MRF24WG Connection algorithm functions
 *
 * Functions that connect, disconnect, get connection status,
 * set reconnection mode, and convert a WPA passphrase to a binary key.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * header format for response to CA Get Element message
 */
typedef struct caElementResponseStruct
{
    t_mgmtMsgRxHdr    mgmtHdr;                /* normal 4-byte hdr for all mgmt responses */
    u_int8_t          elementId;              /* index 4 */
    u_int8_t          elementDataLength;      /* index 5 */
    /* element data follows */
} tCAElementResponseHdr;

// did not want to create header file for one function
extern void pbkdf2_sha1(const char *passphrase, const char *ssid, u_int16_t ssid_len,
                        u_int16_t iterations, u_int8_t *buf, u_int16_t buflen);

/*
 * Set an element of the connection algorithm on the MRF24W.
 *
 * Low-level function to send the appropriate management message to the
 * MRF24W to set the Connection Algorithm element.
 *
 * MACInit must be called first.
 *
 * Parameters:
 *  elementId - Element that is being set
 *  p_elementData - Pointer to element data
 *  elementDataLength - Number of bytes pointed to by p_elementData
 *
 * All Connection Algorithm 'Set Element' functions call this function
 * to construct the management message.  The caller must fix up any endian
 * issues prior to calling this function.
 */
static void LowLevel_CASetElement(unsigned element_id,
    u_int8_t *data, unsigned data_len)
{
    u_int8_t header[4];

    header[0] = WF_TYPE_MGMT_REQUEST;       /* indicate this is a mgmt msg */
    header[1] = WF_SUBTYPE_CA_SET_ELEMENT;  /* mgmt request subtype */
    header[2] = element_id;                 /* Element ID */
    header[3] = data_len;                   /* number of bytes of element data */

    mrf_mgmt_send(header, sizeof(header), data, data_len, 1);
}

#if 0
/*
 * Get an element of the connection algorithm on the MRF24W.
 *
 * Low-level function to send the appropriate management message to the
 * MRF24W to get the Connection Algorithm element.
 *
 * MACInit must be called first.
 *
 * Parameters:
 *  elementId - Element that is being read
 *  p_elementData - Pointer to where element data will be written
 *  elementDataLength - Number of element data bytes that will be read
 *  dataReadAction - If TRUE then read data per paramters and free mgmt response buffer.
 *                   If FALSE then return after response received, do not read any data as the
 *                   caller will do that, and don't free buffer, as caller will do that as well.
 *
 * All Connection Algorithm 'Get Element' functions call this function to
 * construct the management message.  The caller must fix up any endian issues
 * after getting the data from this function.
 */
static void LowLevel_CAGetElement(unsigned element_id,
    u_int8_t *data, unsigned data_len)
{
    u_int8_t  header[4];

    header[0] = WF_TYPE_MGMT_REQUEST;       /* indicate this is a mgmt msg */
    header[1] = WF_SUBTYPE_CA_GET_ELEMENT;  /* mgmt request subtype */
    header[2] = element_id;                 /* Element ID */
    header[3] = 0;                          /* not used */

    if (data) {
        /* read desired data, and then free response buffer */
        mrf_mgmt_send_receive(header, sizeof(header),
            data, data_len,                 /* data bytes to read */
            sizeof(tCAElementResponseHdr)); /* index of first byte of element data */
    } else {
        /* don't read any data bytes, do not release mgmt buffer */
        mrf_mgmt_send(header, sizeof(header), 0, 0, 0);
    }
}
#endif

/*
 * Commands the MRF24WB0MA/B or MRF24WG0MA/B to start a connection.
 *
 * Directs the Connection Manager to scan for and connect to a WiFi network.
 * This function does not wait until the connection attempt is successful, but
 * returns immediately.
 *
 * Note that if the Connection Profile being used has WPA or WPA2 security
 * enabled and is using a passphrase, the connection manager will first
 * calculate the PSK key, and then start the connection process.  The key
 * calculation can take up to 30 seconds.
 *
 * MACInit must be called first.
 *
 * Parameters:
 *  CpId  - If this value is equal to an existing Connection Profile�s ID than
 *          only that Connection Profile will be used to attempt a connection to
 *          a WiFi network.
 *          If this value is set to WF_CM_CONNECT_USING_LIST then the
 *          connectionProfileList will be used to connect, starting with the
 *          first Connection Profile in the list.
 */
void WF_Connect()
{
    u_int8_t header[4];

#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdCheckConnectionConfig();
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid connection config\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    /* Write out header portion of msg (which is whole msg, there is no data) */
    header[0] = WF_TYPE_MGMT_REQUEST;    /* indicate this is a mgmt msg */
    header[1] = WF_SUBTYPE_CM_CONNECT;   /* mgmt request subtype */
    header[2] = GetCpid();
    header[3] = 0;

    mrf_mgmt_send(header, sizeof(header), 0, 0, 1);
}

/*
 * Controls how the MRF24WG handles reconnection in the event of a beacon timeout
 * or a deauthentication from the AP.
 *
 * The host application has two basic options with respect to controlling how the
 * MRF24WG handles a loss of WiFi connection.
 *  1) MRF24WG informs the host and automatically retries N times (or forever)
 *     to regain the connection
 *  2) MRF24WG simply informas the host application that the connection has
 *     been lost; it does not automatically try to regain the connection.
 *     Instead, it is up to the host to reestablish the connection.
 *
 * Parameters:
 *  retryCount - the number of times the MRF24WG should try to regain a connection:
 *               0     - Do not try to regain the connection (simply report event to host application)
 *               1:254 - number of times to try to regain the connection
 *               255   - Retry forever (WF_RETRY_FOREVER)
 *
 *  deauthAction - WF_ATTEMPT_TO_RECONNECT or WF_DO_NOT_ATTEMPT_TO_RECONNECT
 *
 *  beaconTimeOut - Number of missed beacons before MRF24WG designates the
 *                  connection as lost:
 *                  0 - MRF24WG will NOT monitor the beacon timeout condition
 *                      and will not indicate this condition to Host
 *                  1:255 - number of missed beacons before connection declared lost
 *
 *  beaconTimeoutAction - WF_ATTEMPT_TO_RECONNECT or WF_DO_NOT_ATTEMPT_TO_RECONNECT
 *
 * If this function is not called, the MRF2WG default is the equivalent of:
 *      WF_SetReconnectMode(3, WF_ATTEMPT_TO_RECONNECT, 0, WF_DO_NOT_ATTEMPT_TO_RECONNECT);
 */

/*
 * Examples of different scenarios are below.
 *
 * Example 1: MRF24WG should not do any connection retries and only report deauth events to host:
 *            WF_SetReconnectMode(0, WF_DO_NOT_ATTEMPT_TO_RECONNECT, 0, WF_DO_NOT_ATTEMPT_TO_RECONNECT);
 *
 * Example 2: MRF24WG should not do any connection retries, but report deauth and beacon timeout events to host.
 *            Beacon timeout should be 5 beacon periods:
 *            WF_SetReconnectMode(0, WF_DO_NOT_ATTEMPT_TO_RECONNECT, 5, WF_DO_NOT_ATTEMPT_TO_RECONNECT);
 *
 * Example 3: MRF24WG should ignore beacon timeouts, but attempt to reconnect 3 times if a deauth occurs:
 *            WF_SetReconnectMode(3, WF_ATTEMPT_TO_RECONNECT, 0, WF_DO_NOT_ATTEMPT_TO_RECONNECT);
 *
 * Example 4: MRF24WG should not do any connection retries if a deauth occcurs, but retry 3 times if a beacon
 *            timeout of 4 beacon periods occur:
 *            WF_SetReconnectMode(3, WF_DO_NOT_ATTEMPT_TO_RECONNECT, 4, WF_ATTEMPT_TO_RECONNECT);
 *
 * Example 5: MRF24WG should retry forever if either a deauth or beacon timeout occurs (beacon timeout is
 *            3 beacon periods):
 *            WF_SetReconnectMode(WF_RETRY_FOREVER, WF_ATTEMPT_TO_RECONNECT, 3, WF_ATTEMPT_TO_RECONNECT);
 */
void WF_ReconnectModeSet(u_int8_t retryCount, u_int8_t deauthAction,
    u_int8_t beaconTimeout, u_int8_t beaconTimeoutAction)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetReconnectMode(retryCount, deauthAction, beaconTimeout, beaconTimeoutAction);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid reconnect mode\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    LowLevel_CASetElement(WF_CA_ELEMENT_LIST_RETRY_COUNT,       // Element ID
                          &retryCount,                          // pointer to element data
                          sizeof(retryCount));                  // number of element data bytes

    LowLevel_CASetElement(WF_CA_ELEMENT_DEAUTH_ACTION,          // Element ID
                          &deauthAction,                        // pointer to element data
                          sizeof(deauthAction));                // number of element data bytes

    LowLevel_CASetElement(WF_CA_ELEMENT_BEACON_TIMEOUT,         // Element ID
                          &beaconTimeout,                       // pointer to element data
                          sizeof(beaconTimeout));               // number of element data bytes

    LowLevel_CASetElement(WF_CA_ELEMENT_BEACON_TIMEOUT_ACTION,  // Element ID
                          &beaconTimeoutAction,                 // pointer to element data
                          sizeof(beaconTimeoutAction));         // number of element data bytes

}

/*
 * Commands the MRF24WG to close any open connections and/or to cease attempting
 * to connect.
 *
 * Directs the Connection Manager to close any open connection or connection
 * attempt in progress.  No further attempts to connect are taken until
 * WF_Connect() is called.
 */
void WF_Disconnect()
{
    int connectionState;
    u_int8_t header[2];

    /* WARNING !!! :
     * Disconnect is allowed only in connected state.
     * If module FW is in the midst of connection (or reconnection) process, then
     * disconnect can hammer connection process, and furthermore it may cause
     * fatal failure in module FW operation. */

    // verify it is OK to issue a disconnect command
    connectionState = WF_ConnectionStateGet();
    if (connectionState != WF_CSTATE_CONNECTED_INFRASTRUCTURE &&
        connectionState != WF_CSTATE_CONNECTED_ADHOC) {
        printf("--- %s: disconnect not allowed\n", __func__);
        return;
    }

    header[0] = WF_TYPE_MGMT_REQUEST;
    header[1] = WF_SUBTYPE_CM_DISCONNECT;

    mrf_mgmt_send(header, sizeof(header), 0, 0, 1);

    UdSetConnectionState(CS_NOT_CONNECTED);
}


/*
 * Set the Channel List used by the Connection Algorithm.
 * MACInit must be called first.
 *
 * Parameters:
 *  p_channelList - Pointer to channel list.
 *  numChannels   - Number of channels in p_channelList.  If set to 0, the
 *                  MRF24W will use all valid channels for the current
 *                  regional domain.
 */
void WF_ChannelListSet(u_int8_t *p_channelList, u_int8_t numChannels)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetChannelList(p_channelList, numChannels);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid channel list\n", __func__);
        return;
    }
#endif

    LowLevel_CASetElement(WF_CA_ELEMENT_CHANNEL_LIST,  /* Element ID */
                          p_channelList,               /* pointer to element data */
                          numChannels);                /* number of element data bytes */
}

void WF_ScanContextSet(t_scanContext *p_context)
{
    u_int16_t tmp;

#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetScanContext(p_context);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid scan context\n", __func__);
        return;
    }
#endif

    LowLevel_CASetElement(WF_CA_ELEMENT_SCANTYPE,           // Element ID
                          &p_context->scanType,             // pointer to element data
                          sizeof(p_context->scanType));     // number of element data bytes

    LowLevel_CASetElement(WF_CA_ELEMENT_SCAN_COUNT,
                          &p_context->scanCount,
                          sizeof(p_context->scanCount));

    tmp = htons(p_context->minChannelTime);
    LowLevel_CASetElement(WF_CA_ELEMENT_MIN_CHANNEL_TIME,
                          (u_int8_t *)&tmp,
                          sizeof(tmp));

    tmp = htons(p_context->maxChannelTime);
    LowLevel_CASetElement(WF_CA_ELEMENT_MAX_CHANNEL_TIME,
                          (u_int8_t *)&tmp,
                          sizeof(tmp));

    tmp = htons(p_context->probeDelay);
    LowLevel_CASetElement(WF_CA_ELEMENT_PROBE_DELAY,
                          (u_int8_t *)&tmp,
                          sizeof(tmp));
}

void WF_AdhocContextSet(t_adHocNetworkContext *context)
{
    u_int16_t tmp;

#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetAdhocNetworkContext(context);
    if (errorCode != UD_SUCCESS) {
        return;
    }
#endif

    mrf_profile_set_hidden(context->hiddenSsid);

    tmp = htons(context->beaconPeriod);
    LowLevel_CASetElement(WF_CA_ELEMENT_BEACON_PERIOD,  // Element ID
                          (u_int8_t *)&tmp,              // pointer to element data
                          sizeof(tmp));                 // number of element data bytes

    mrf_profile_set_adhoc_mode(context->mode);
}

void WF_RssiSet(u_int8_t rssi)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetRssi(rssi);
    if (errorCode != UD_SUCCESS) {
        return;
    }
#endif
    LowLevel_CASetElement(WF_CA_ELEMENT_RSSI,   // Element ID
                          &rssi,                // pointer to element data
                          sizeof(rssi));        // number of element data bytes
}

/*
 * Return the current connection state.
 *
 * Parameters:
 *  p_state - Pointer to location where connection state will be written
 *
 * Response msg is actually two bytes, the second byte being the Connection Profile ID.
 * Since this is not being used, set msgData to a 1-byte array.
 */
int WF_ConnectionStateGet()
{
    u_int8_t header[2], reply[1];

    header[0] = WF_TYPE_MGMT_REQUEST;
    header[1] = WF_SUBTYPE_CM_GET_CONNECTION_STATUS;

    mrf_mgmt_send_receive(header, sizeof(header),
        reply, sizeof(reply), MGMT_RESP_1ST_DATA_BYTE_INDEX);

    return reply[0]; // connection state
}

/*
 * Convert the input WPA/WPA2 passphrase to a 32-byte binary key
 *
 * When this function completes the input structure field p_keyInfo->key[] will
 * have been overwritten with the 32-byte binary key.  In addtion, the input
 * structure field p_keyInfo->keyLength will be set to 32.
 *
 * Caller needs to be aware that two of the input fields will be overwritten
 * with the result of the conversion.
 *
 * Parameters:
 *  p_keyInfo -- structure containing the ASCII WPA passphrase
 */
void WF_WpaConvPassphraseToKey(t_wpaKeyInfo *p_keyInfo)
{
    u_int8_t binaryKey[WF_WPA_KEY_LENGTH];
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdConvWpaPassphrase(p_keyInfo);
    if (errorCode != UD_SUCCESS)
    {
        printf("--- %s: cannot convert passphrase\n", __func__);
        return;
    }
#endif
    p_keyInfo->key[p_keyInfo->keyLength] = '\0';   // make sure passphrase is terminated

    // generate the binary key
    pbkdf2_sha1((const char *)p_keyInfo->key,
                (const char *)p_keyInfo->ssid,
                p_keyInfo->ssidLen,
                4096,
                binaryKey, // binary key will be written to this field
                32);

    // overwrite the passphrase with binary key
    bcopy(binaryKey, p_keyInfo->key, WF_WPA_KEY_LENGTH);

    // overwrite the length with the length of the binary key (always 32)
    p_keyInfo->keyLength = WF_WPA_KEY_LENGTH;
}

void SetListenInterval(u_int16_t listenInterval)
{
    /* correct endianness before sending message */
    listenInterval = htons(listenInterval);
    LowLevel_CASetElement(WF_CA_ELEMENT_LISTEN_INTERVAL,    /* Element ID */
                         (u_int8_t *)&listenInterval,        /* pointer to element data */
                          sizeof(listenInterval));          /* number of element data bytes */
}

void SetDtimInterval(u_int16_t dtimInterval)
{
    /* correct endianness before sending message */
    dtimInterval = htons(dtimInterval);

    LowLevel_CASetElement(WF_CA_ELEMENT_DTIM_INTERVAL,    /* Element ID */
                          (u_int8_t *)&dtimInterval,       /* pointer to element data */
                          sizeof(dtimInterval));          /* number of element data bytes */
}
