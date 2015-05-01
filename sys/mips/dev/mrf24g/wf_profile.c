/*
 * MRF24WG Connection profile functions
 *
 * Functions that connect, disconnect, get connection status,
 * set reconnection mode, and convert a WPA passphrase to a binary key.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include "wf_universal_driver.h"
#include "wf_ud_state.h"

/*
 * Connection Profile Element IDs.
 * Used in conjunction with the WF_SUBTYPE_CP_SET_ELEMENT and
 * WF_SUBTYPE_CP_GET_ELEMENT message subtypes
 */
enum {
    WF_CP_ELEMENT_ALL               = 0,  /* sends all elements in CP struct (not used)*/
    WF_CP_ELEMENT_SSID              = 1,
    WF_CP_ELEMENT_BSSID             = 2,
    WF_CP_ELEMENT_SECURITY          = 3,
    WF_CP_ELEMENT_NETWORK_TYPE      = 4,
    WF_CP_ELEMENT_ADHOC_BEHAVIOR    = 5,
    WF_CP_ELEMENT_WEP_KEY_INDEX     = 6,
    WF_CP_ELEMENT_SSID_TYPE         = 7,
    WF_CP_ELEMENT_WEPKEY_TYPE       = 8,
    WF_CP_ELEMENT_UPDATE_PMK        = 9,
    WF_CP_ELEMENT_READ_WPS_CRED     = 10
};

/*
 * Set an element of the connection profile on the MRF24W.
 * MACInit must be called first.
 *
 * All Connection Profile 'Set Element' functions call this function to
 * construct the management message.  The caller must fix up any endian issues
 * prior to calling this function.
 *
 * Parameters:
 *  elementId - Element that is being set
 *  p_elementData - Pointer to element data
 *  elementDataLength - Number of bytes pointed to by p_elementData
 */
static void set_element(unsigned cpid, unsigned id, u_int8_t *data, unsigned data_len)
{
    u_int8_t  header[5];

    /* Write out header portion of msg */
    header[0] = WF_TYPE_MGMT_REQUEST;       /* indicate this is a mgmt msg */
    header[1] = WF_SUBTYPE_CP_SET_ELEMENT;  /* mgmt request subtype */
    header[2] = cpid;                       /* Connection Profile ID */
    header[3] = id;                         /* Element ID */
    header[4] = data_len;                   /* number of bytes of element data */

    mrf_mgmt_send(header, sizeof(header), data, data_len, 1);
}

/*
 * Get an element of the connection profile on the MRF24W.
 *
 * All Connection Profile 'Get Element' functions call this function to
 * construct the management message.  The caller must fix up any endian issues
 * prior to calling this function.
 * MACInit must be called first.
 *
 * Parameters:
 *  elementId - Element that is being read
 *  p_elementData - Pointer to where element data will be written
 *  elementDataLength - Number of element data bytes that will be read
 *  dataReadAction - If TRUE then read data per paramters and free mgmt
 *                   response buffer. If FALSE then return after response
 *                   received, do not read any data as the caller will do that,
 *                   and don't free buffer, as caller will do that as well.
 */
static void get_element(unsigned cpid, unsigned id, u_int8_t *reply, unsigned reply_len)
{
    u_int8_t header[4];

    header[0] = WF_TYPE_MGMT_REQUEST;       /* indicate this is a mgmt msg */
    header[1] = WF_SUBTYPE_CP_GET_ELEMENT;  /* mgmt request subtype */
    header[2] = cpid;                       /* Connection Profile ID */
    header[3] = id;                         /* Element ID */

    if (reply) {
        /* 7 = index of first byte of element data */
        mrf_mgmt_send_receive(header, sizeof(header), reply, reply_len, 7);
    } else {
        /* don't read any data bytes, do not release mgmt buffer */
        mrf_mgmt_send(header, sizeof(header), 0, 0, 0);
    }
}

/*
 * Configures security for a Connection Profile.
 * MACInit must be called first.
 *
 *   Security                                Key         Length
 *   --------                                ---         ------
 *   WF_SECURITY_OPEN                        N/A         N/A
 *   WF_SECURITY_WEP_40                      hex         4, 5 byte keys
 *   WF_SECURITY_WEP_104                     hex         4, 13 byte keys
 *   WF_SECURITY_WPA_WITH_KEY                hex         32 bytes
 *   WF_SECURITY_WPA_WITH_PASS_PHRASE        ascii       8-63 ascii characters
 *   WF_SECURITY_WPA2_WITH_KEY               hex         32 bytes
 *   WF_SECURITY_WPA2_WITH_PASS_PHRASE       ascii       8-63 ascii characters
 *   WF_SECURITY_WPA_AUTO_WITH_KEY           hex         32 bytes
 *   WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE   ascii       8-63 ascii characters
 *
 * Parameters:
 *  security_type - Value corresponding to the security type desired.
 *  wep_key_index - 0 thru 3 (only used if security type is WF_SECURITY_WEP_40 or
 *                  WF_SECURITY_WEP_104)
 *  key           - Binary key or passphrase (not used if security is
 *                  WF_SECURITY_OPEN)
 *  key_length    - Number of bytes in p_securityKey (not used if security
 *                  is WF_SECURITY_OPEN)
 */
static void set_security(unsigned cpid, unsigned security_type,
    unsigned wep_key_index, u_int8_t *key, unsigned key_length)
{
    u_int8_t header[7];

    /* Write out header portion of msg */
    header[0] = WF_TYPE_MGMT_REQUEST;           /* indicate this is a mgmt msg */
    header[1] = WF_SUBTYPE_CP_SET_ELEMENT;      /* mgmt request subtype */
    header[2] = cpid;                           /* Connection Profile ID */
    header[3] = WF_CP_ELEMENT_SECURITY;         /* Element ID */

    /* Next to header bytes are really part of data, but need to put
     * them in header bytes in order to prepend to security key. */
    header[5] = security_type;
    header[6] = wep_key_index;

    /* if security is open (no key) or WPS push button method */
    if (security_type == WF_SECURITY_OPEN ||
        security_type == WF_SECURITY_WPS_PUSH_BUTTON) {
        /* Only data is security type and wep index */
        header[4]  = 2;
        key        = 0;
        key_length = 0;
    } else {
        /* Security is selected, so need to send key */
        header[4] = 2 + key_length; /* data is security type + wep index + key */
    }

    mrf_mgmt_send(header, sizeof(header), key, key_length, 1);
}

/*
 * Create a Connection Profile on the MRF24W.
 *
 * Requests the MRF24W to create a Connection Profile (CP), assign it an ID,
 * and set all the elements to default values.  The ID returned by this function
 * is used in other connection profile functions.  A maximum of 2 Connection
 * Profiles can exist on the MRF24W.
 *
 * MACInit must be called first.
 *
 * Parameters:
 *  p_CpId - Pointer to where Connection Profile ID will be written.  If
 *           function fails, the CP ID will be set to 0xff.
 */
unsigned mrf_profile_create()
{
    u_int8_t hdr[2], cpid;

    hdr[0] = WF_TYPE_MGMT_REQUEST;
    hdr[1] = WF_SUBTYPE_CP_CREATE_PROFILE;
    cpid = 0xff;

    /* read 1 byte of data, starting at index 4 */
    mrf_mgmt_send_receive(hdr, sizeof(hdr), &cpid, 1, 4);
    return cpid;
}

/*
 * Sets the SSID and SSID Length elements in the Connection Profile.  Note that
 * an Access Point can have either a visible or hidden SSID.  If an Access Point
 * uses a hidden SSID then an active scan must be used (see scanType field in the
 * Connection Algorithm).
 *
 * MACInit must be called first.
 *
 * Parameters:
 *   p_ssid - Pointer to the SSID string
 *   ssidLength - Number of bytes in the SSID
 */
void mrf_profile_set_ssid(unsigned cpid, u_int8_t *ssid, unsigned ssid_len)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetSsid(ssid, ssid_len);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid SSID\n", __func__);
        return;
    }
#endif
    set_element(cpid, WF_CP_ELEMENT_SSID, ssid, ssid_len);
}

#if 0
/*
 * Get the SSID of the Connection Profile.
 * Return length of SSID string.
 *
 * Parameters:
 *  p_ssid - Pointer to the SSID string
 */
unsigned mrf_profile_get_ssid(unsigned cpid, u_int8_t *ssid)
{
    u_int8_t hdr[7];
    unsigned ssid_len;

    /* Request SSID, but don't have this function read data or free response buffer. */
    get_element(cpid, WF_CP_ELEMENT_SSID, 0, 0);

    /* At this point, management response is mounted and ready to be read.
     * Set raw index to 0, read normal 4 byte header plus the next 3 bytes, these will be:
     *   profile id             [4]
     *   element id             [5]
     *   element data length    [6] */
    mrf_raw_pread(RAW_ID_MGMT_RX, hdr, sizeof(hdr), 0);
    ssid_len = hdr[6];

    /* copy SSID name to callers buffer */
    mrf_raw_pread(RAW_ID_MGMT_RX, ssid, ssid_len, sizeof(hdr));

    /* free management buffer */
    mrf_raw_move(RAW_ID_MGMT_RX, RAW_MGMT_POOL, 0, 0);

    /* extract SSID length and write to caller */
    return ssid_len;
}
#endif

/*
 * Set the Network Type element a Connection Profile.
 * Allowable values are:
 *  WF_NETWORK_TYPE_INFRASTRUCTURE
 *  WF_NETWORK_TYPE_ADHOC
 *
 * Parameter:
 *  networkType - Type of network to create (infrastructure or adhoc)
 */
void mrf_profile_set_network_type(unsigned cpid, unsigned nettype)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetNetworkType(nettype);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid network type=%u\n", __func__, nettype);
        return;
    }
#endif

    set_element(cpid, WF_CP_ELEMENT_NETWORK_TYPE, (u_int8_t*) &nettype, 1);
}

/*
 * Sets the SSID type element in the Connection Profile.
 * !!! This is valid only when we create AdHoc network.
 *
 * MACInit must be called first.
 *
 * Parameters:
 *  CpId   - Connection Profile ID
 *  hidden - True or False
 */
void mrf_profile_set_hidden(unsigned cpid, int hidden)
{
    set_element(cpid, WF_CP_ELEMENT_SSID_TYPE, (u_int8_t*) &hidden, 1);
}

void mrf_profile_set_open(unsigned cpid)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetSecurityOpen();
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: must be disconnected\n", __func__);
        return;
    }
#endif

    set_security(cpid, WF_SECURITY_OPEN, 0, 0, 0);
}

/*
 * Set WEP security type.
 * Parameters:
 *  type      - WF_SECURITY_WEP_40 or WF_SECURITY_WEP_104
 *  key_index - 0 thru 3
 *  key       - binary (4 5-byte keys for WEP-40, 4 13-byte keys for WEP-104)
 *  key_len   - 20 bytes for WEP-40, 52 bytes for WEP-104
 */
void mrf_profile_set_wep(unsigned cpid, unsigned type,
    unsigned key_index, u_int8_t *key, unsigned key_len)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetSecurityWep(type, key_index, key, key_len);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot set WEP security\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    set_security(cpid, type, key_index, key, key_len);
}

/*
 * Set WPA security type.
 * Parameters:
 *  type     - WF_SECURITY_WPA_WITH_KEY, WF_SECURITY_WPA_WITH_PASS_PHRASE,
 *             WF_SECURITY_WPA2_WITH_KEY, WF_SECURITY_WPA2_WITH_PASS_PHRASE
 *             WF_SECURITY_WPA_AUTO_WITH_KEY, WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE
 *  key       - binary or text
 *  key_len   - 32 for binary, 8...63 for text
 */
void mrf_profile_set_wpa(unsigned cpid, unsigned type,
    u_int8_t *key, unsigned key_len)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetSecurityWpa(type, key, key_len);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot set WPA security\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    set_security(cpid, type, 0, key, key_len);
}

/*
 * Set WPA security type.
 * Parameters:
 *  type     - WF_SECURITY_WPS_PUSH_BUTTON or WF_SECURITY_WPS_PIN
 *  pin      - 8-digit pin for WPS_PIN mode
 *  pin_len  - 8 for WPS_PIN mode
 */
void mrf_profile_set_wps(unsigned cpid, unsigned type, u_int8_t *pin, unsigned pin_len)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetSecurityWps(type, pin, pin_len);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot set WPS security\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    set_security(cpid, type, 0, pin, pin_len);
}

void mrf_profile_set_bssid(unsigned cpid, u_int8_t *bssid)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetBssid(bssid);
    if (errorCode != UD_SUCCESS) {
        return;
    }

#endif
    set_element(cpid, WF_CP_ELEMENT_BSSID, bssid, WF_BSSID_LENGTH);
}

// called from SetAdhocContext().  Error checking performed there
void mrf_profile_set_adhoc_mode(unsigned cpid, int mode)
{
    set_element(cpid, WF_CP_ELEMENT_ADHOC_BEHAVIOR, (u_int8_t*) &mode, 1);
}

void mrf_profile_get_wps_credentials(unsigned cpid, wps_credentials_t *cred)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdGetWpsCredentials();
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot get WPS credentials\n", __func__);
        return;
    }
#endif
    get_element(cpid, WF_CP_ELEMENT_READ_WPS_CRED, (u_int8_t*) cred, sizeof(*cred));
}
