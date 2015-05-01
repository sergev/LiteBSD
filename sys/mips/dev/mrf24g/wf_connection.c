/*
 * MRF24WG Connection algorithm functions
 *
 * Functions that connect, disconnect, get connection status,
 * set reconnection mode, and convert a WPA passphrase to a binary key.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include "wf_universal_driver.h"
#include "wf_ud_state.h"

/*
 * Set an element of the connection algorithm on the MRF24W.
 *
 * Low-level function to send the appropriate management message to the
 * MRF24W to set the Connection Algorithm element.
 *
 * MACInit must be called first.
 *
 * Parameters:
 *  element_id - Element that is being set
 *  data       - Pointer to element data
 *  data_len   - Number of bytes
 *
 * All Connection Algorithm 'Set Element' functions call this function
 * to construct the management message.  The caller must fix up any endian
 * issues prior to calling this function.
 */
static void set_element(unsigned element_id, u_int8_t *data, unsigned data_len)
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
 * Parameters:
 *  element_id - Element that is being read
 *  data       - Pointer to where element data will be written, or NULL
 *  data_len   - Number of element data bytes that will be read
 *
 * If data is non-NULL, then read data per parameters and
 * free mgmt response buffer.
 * If data is NULL, then return after response received,
 * do not read any data as the caller will do that, and don't
 * free buffer, as caller will do that as well.
 *
 * All Connection Algorithm 'Get Element' functions call this function to
 * construct the management message.  The caller must fix up any endian issues
 * after getting the data from this function.
 */
static void get_element(unsigned element_id, u_int8_t *data, unsigned data_len)
{
    u_int8_t  header[4];

    header[0] = WF_TYPE_MGMT_REQUEST;       /* indicate this is a mgmt msg */
    header[1] = WF_SUBTYPE_CA_GET_ELEMENT;  /* mgmt request subtype */
    header[2] = element_id;                 /* Element ID */
    header[3] = 0;                          /* not used */

    if (data) {
        /* Read desired data, and then free response buffer.
         * 6 = index of first byte of element data. */
        mrf_mgmt_send_receive(header, sizeof(header), data, data_len, 6);
    } else {
        /* Don't read any data bytes, do not release mgmt buffer. */
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
 *  cpid  - this Connection Profile will be used to attempt a connection to
 *          a WiFi network.
 */
void mrf_connect(unsigned cpid)
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
    header[2] = cpid;
    header[3] = 0;

    mrf_mgmt_send(header, sizeof(header), 0, 0, 1);
}

/*
 * Commands the MRF24WG to close any open connections and/or to cease attempting
 * to connect.
 *
 * Directs the Connection Manager to close any open connection or connection
 * attempt in progress.  No further attempts to connect are taken until
 * mrf_connect() is called.
 */
void mrf_disconnect()
{
    int connectionState;
    u_int8_t header[2];

    /* WARNING !!! :
     * Disconnect is allowed only in connected state.
     * If module FW is in the midst of connection (or reconnection) process, then
     * disconnect can hammer connection process, and furthermore it may cause
     * fatal failure in module FW operation. */

    /* verify it is OK to issue a disconnect command */
    connectionState = mrf_conn_get_state();
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
 *  retry_count - the number of times the MRF24WG should try to regain a connection:
 *                0     - Do not try to regain the connection (simply report event to host application)
 *                1:254 - number of times to try to regain the connection
 *                255   - Retry forever
 *
 *  reconnect_on_deauth - True or False
 *
 *  beacon_timeOut - Number of missed beacons before MRF24WG designates the
 *                   connection as lost:
 *                   0 - MRF24WG will NOT monitor the beacon timeout condition
 *                       and will not indicate this condition to Host
 *                   1:255 - number of missed beacons before connection declared lost
 *
 *  reconnect_on_loss - True or False
 *
 * If this function is not called, the MRF2WG default is the equivalent of:
 *      mrf_conn_set_mode(3, True, 0, False);
 */

/*
 * Examples of different scenarios are below.
 *
 * Example 1: MRF24WG should not do any connection retries and only
 * report deauth events to host:
 *      mrf_conn_set_mode(0, False, 0, False);
 *
 * Example 2: MRF24WG should not do any connection retries, but report
 * deauth and beacon timeout events to host.
 * Beacon timeout should be 5 beacon periods:
 *      mrf_conn_set_mode(0, False, 5, False);
 *
 * Example 3: MRF24WG should ignore beacon timeouts, but attempt to reconnect
 * 3 times if a deauth occurs:
 *      mrf_conn_set_mode(3, True, 0, False);
 *
 * Example 4: MRF24WG should not do any connection retries if a deauth occcurs,
 * but retry 3 times if a beacon timeout of 4 beacon periods occur:
 *      mrf_conn_set_mode(3, False, 4, True);
 *
 * Example 5: MRF24WG should retry forever if either a deauth or beacon
 * timeout occurs (beacon timeout is 3 beacon periods):
 *      mrf_conn_set_mode(255, True, 3, True);
 */
void mrf_conn_set_mode(unsigned retry_count, int reconnect_on_deauth,
    unsigned beacon_timeout, int reconnect_on_loss)
{
#if defined(WF_ERROR_CHECKING)
    unsigned error_code;

    error_code = UdSetReconnectMode(retry_count, reconnect_on_deauth,
        reconnect_on_loss);
    if (error_code != UD_SUCCESS) {
        printf("--- %s: invalid reconnect mode\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    set_element(WF_CA_ELEMENT_LIST_RETRY_COUNT,
        (u_int8_t*) &retry_count, 1);
    set_element(WF_CA_ELEMENT_DEAUTH_ACTION,
        (u_int8_t*) &reconnect_on_deauth, 1);
    set_element(WF_CA_ELEMENT_BEACON_TIMEOUT,
        (u_int8_t*) &beacon_timeout, 1);
    set_element(WF_CA_ELEMENT_BEACON_TIMEOUT_ACTION,
        (u_int8_t*) &reconnect_on_loss, 1);
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
void mrf_conn_set_channels(u_int8_t *channel_list, unsigned num_channels)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetChannelList(channel_list, num_channels);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid channel list\n", __func__);
        return;
    }
#endif

    set_element(WF_CA_ELEMENT_CHANNEL_LIST, channel_list, num_channels);
}

void mrf_conn_set_scan(unsigned scan_type,      // see WF_SCAN_*
    unsigned scan_count, unsigned min_channel_msec, unsigned max_channel_msec,
    unsigned probe_delay_usec)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetScanContext(scan_type);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid scan context\n", __func__);
        return;
    }
#endif

    /* Convert to network byte order. */
    min_channel_msec = htons(min_channel_msec);
    max_channel_msec = htons(max_channel_msec);
    probe_delay_usec = htons(probe_delay_usec);

    set_element(WF_CA_ELEMENT_SCANTYPE,
                (u_int8_t*) &scan_type, 1);
    set_element(WF_CA_ELEMENT_SCAN_COUNT,
                (u_int8_t*) &scan_count, 1);
    set_element(WF_CA_ELEMENT_MIN_CHANNEL_TIME,
                (u_int8_t*) &min_channel_msec, 2);
    set_element(WF_CA_ELEMENT_MAX_CHANNEL_TIME,
                (u_int8_t*) &max_channel_msec, 2);
    set_element(WF_CA_ELEMENT_PROBE_DELAY,
                (u_int8_t*) &probe_delay_usec, 2);
}

/*
 * Parameters:
 *  hidden_ssid - True if SSID should be hidden, else False (normally False)
 *  beacon_msec - beacon period, in ms
 *  mode        - WF_ADHOC_CONNECT_THEN_START, or WF_ADHOC_CONNECT_ONLY,
 *                or WF_ADHOC_START_ONLY
 */
void mrf_conn_set_adhoc(unsigned cpid, int hidden_ssid, unsigned beacon_msec, unsigned mode)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetAdhocNetworkContext(hidden_ssid, mode);
    if (errorCode != UD_SUCCESS) {
        return;
    }
#endif

    mrf_profile_set_hidden(cpid, hidden_ssid);

    beacon_msec = htons(beacon_msec);
    set_element(WF_CA_ELEMENT_BEACON_PERIOD, (u_int8_t*) &beacon_msec, 2);

    mrf_profile_set_adhoc_mode(cpid, mode);
}

void mrf_conn_set_rssi(unsigned rssi)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetRssi();
    if (errorCode != UD_SUCCESS) {
        return;
    }
#endif
    set_element(WF_CA_ELEMENT_RSSI, (u_int8_t*) &rssi, 1);
}

void mrf_conn_set_listen_interval(unsigned listen_interval)
{
    /* correct endianness before sending message */
    listen_interval = htons(listen_interval);
    set_element(WF_CA_ELEMENT_LISTEN_INTERVAL,
                (u_int8_t*) &listen_interval, 2);
}

void mrf_conn_set_dtim_interval(unsigned dtim_interval)
{
    /* correct endianness before sending message */
    dtim_interval = htons(dtim_interval);
    set_element(WF_CA_ELEMENT_DTIM_INTERVAL,
                (u_int8_t*) &dtim_interval, 2);
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
int mrf_conn_get_state()
{
    u_int8_t header[2], reply[1];

    header[0] = WF_TYPE_MGMT_REQUEST;
    header[1] = WF_SUBTYPE_CM_GET_CONNECTION_STATUS;

    mrf_mgmt_send_receive(header, sizeof(header), reply, sizeof(reply), 4);

    /* Return connection state WF_CSTATE_xxx. */
    return reply[0];
}
