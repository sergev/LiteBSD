/*
 * MRF24WG Connection profile functions
 *
 * Functions that connect, disconnect, get connection status,
 * set reconnection mode, and convert a WPA passphrase to a binary key.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * Connection Profile Element IDs.
 * Used in conjunction with the WF_CP_SET_ELEMENT_SUBTYPE and
 * WF_CP_GET_ELEMENT_SUBTYPE message subtypes
 */
typedef enum tCPElementIds {
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
} tCPElementIds;

/*
 * header format for response to CP Get Element message
 */
typedef struct cpElementResponseStruct {
    t_mgmtMsgRxHdr   mgmtHdr;                /* normal 4-byte hdr for all mgmt responses */
    u_int8_t         profileId;
    u_int8_t         elementId;
    u_int8_t         elementDataLength;
    /* element data follow */
} t_cPElementResponseHdr;

static u_int8_t g_cpid;

static t_wpaKeyInfo *g_p_wpaKeyInfo;

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
static void LowLevel_CPSetElement(unsigned element_id,
    u_int8_t *data, unsigned data_len)
{
    u_int8_t  header[5];

    /* Write out header portion of msg */
    header[0] = WF_MGMT_REQUEST_TYPE;       /* indicate this is a mgmt msg */
    header[1] = WF_CP_SET_ELEMENT_SUBTYPE;  /* mgmt request subtype */
    header[2] = g_cpid;                     /* Connection Profile ID */
    header[3] = element_id;                 /* Element ID */
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
static void LowLevel_CPGetElement(u_int8_t element_id,
                                  u_int8_t *reply,
                                  u_int8_t reply_len)
{
    u_int8_t  header[4];

    header[0] = WF_MGMT_REQUEST_TYPE;       /* indicate this is a mgmt msg */
    header[1] = WF_CP_GET_ELEMENT_SUBTYPE;  /* mgmt request subtype */
    header[2] = g_cpid;                     /* Connection Profile ID */
    header[3] = element_id;                 /* Element ID */

    if (reply) {
        mrf_mgmt_send_receive(header, sizeof(header),
            reply, reply_len,               /* where to write element data */
            sizeof(t_cPElementResponseHdr)); /* index of first byte of element data */
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
 *  securityType      - Value corresponding to the security type desired.
 *  wepKeyIndex       - 0 thru 3 (only used if security type is WF_SECURITY_WEP_40 or
 *                      WF_SECURITY_WEP_104)
 *  p_securityKey     - Binary key or passphrase (not used if security is
 *                      WF_SECURITY_OPEN)
 *  securityKeyLength - Number of bytes in p_securityKey (not used if security
 *                      is WF_SECURITY_OPEN)
 */
static void LowLevel_SetSecurity(unsigned securityType,
                                 unsigned wepKeyIndex,
                                 u_int8_t *p_securityKey,
                                 unsigned securityKeyLength)
{
    u_int8_t header[7], *p_key;

    /* Write out header portion of msg */
    header[0] = WF_MGMT_REQUEST_TYPE;           /* indicate this is a mgmt msg */
    header[1] = WF_CP_SET_ELEMENT_SUBTYPE;      /* mgmt request subtype */
    header[2] = GetCpid();                      /* Connection Profile ID */
    header[3] = WF_CP_ELEMENT_SECURITY;         /* Element ID */

    /* Next to header bytes are really part of data, but need to put them in header */
    /* bytes in order to prepend to security key */
    header[5] = securityType;
    header[6] = wepKeyIndex;

    /* if security is open (no key) or WPS push button method */
    if (securityType == WF_SECURITY_OPEN ||
        securityType == WF_SECURITY_WPS_PUSH_BUTTON)
    {
        header[4]         = 2;      /* Only data is security type and wep index */
        p_key             = 0;
        securityKeyLength = 0;
    }
    /* else security is selected, so need to send key */
    else {
        header[4] = 2 + securityKeyLength;  /* data is security type + wep index + key */
        p_key     = p_securityKey;
    }

    mrf_mgmt_send(header, sizeof(header), p_key, securityKeyLength, 1);
}

unsigned GetCpid()
{
    return g_cpid;
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
void WF_CPCreate()
{
    u_int8_t hdr[2];

    hdr[0] = WF_MGMT_REQUEST_TYPE;
    hdr[1] = WF_CP_CREATE_PROFILE_SUBTYPE;
    g_cpid = 0xff;

    mrf_mgmt_send_receive(hdr, sizeof(hdr),
        &g_cpid, 1,                         /* write data here */
        MGMT_RESP_1ST_DATA_BYTE_INDEX);     /* read starting at index 4 */
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
void WF_SsidSet(u_int8_t *p_ssid, u_int8_t ssidLength)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetSsid(p_ssid, ssidLength);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid SSID\n", __func__);
        return;
    }
#endif
    LowLevel_CPSetElement(WF_CP_ELEMENT_SSID, p_ssid, ssidLength);
}

/*
 * Gets the SSID and SSID Length elements in the Connection Profile.
 *
 * Parameters:
 *  p_ssid - Pointer to the SSID string
 *  ssidLength - Pumber of bytes in the SSID
 */
void WF_SsidGet(u_int8_t *p_ssid, u_int8_t *p_ssidLength)
{
    t_cPElementResponseHdr  mgmtHdr;

    /* Request SSID, but don't have this function read data or free response buffer. */
    LowLevel_CPGetElement(WF_CP_ELEMENT_SSID, 0, 0);

    /* At this point, management response is mounted and ready to be read.
     * Set raw index to 0, read normal 4 byte header plus the next 3 bytes, these will be:
     *   profile id             [4]
     *   element id             [5]
     *   element data length    [6] */
    mrf_raw_pread(RAW_ID_MGMT_RX, (u_int8_t*) &mgmtHdr, sizeof(t_cPElementResponseHdr), 0);

    /* extract SSID length and write to caller */
    *p_ssidLength = mgmtHdr.elementDataLength;

    /* copy SSID name to callers buffer */
    mrf_raw_pread(RAW_ID_MGMT_RX, p_ssid, *p_ssidLength, sizeof(t_cPElementResponseHdr));

    /* free management buffer */
    mrf_raw_move(RAW_ID_MGMT_RX, RAW_MGMT_POOL, 0, 0);
}

/*
 * Set the Network Type element a Connection Profile.
 * Allowable values are:
 *  WF_NETWORK_TYPE_INFRASTRUCTURE
 *  WF_NETWORK_TYPE_ADHOC
 *
 * Parameter:
 *  networkType - Type of network to create (infrastructure or adhoc)
 */
void WF_NetworkTypeSet(u_int8_t networkType)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetNetworkType(networkType);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: invalid network type=%u\n", __func__, networkType);
        return;
    }
#endif

    LowLevel_CPSetElement(WF_CP_ELEMENT_NETWORK_TYPE, &networkType, 1);
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
void WF_SsidTypeSet(bool hidden)
{
    LowLevel_CPSetElement(WF_CP_ELEMENT_SSID_TYPE, (u_int8_t*) &hidden, 1);
}

void WF_SecurityOpenSet()
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetSecurityOpen();
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: must be disconnected\n", __func__);
        return;
    }
#endif

    LowLevel_SetSecurity(WF_SECURITY_OPEN, 0, 0, 0);
}

void WF_SecurityWepSet(t_wepContext* p_context)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetSecurityWep(p_context);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot set WEP security\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    LowLevel_SetSecurity(p_context->wepSecurityType,
                         p_context->wepKeyIndex,
                         p_context->wepKey,
                         p_context->wepKeyLength);
}

void WF_SecurityWpaSet(t_wpaContext* p_context)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode;

    errorCode = UdSetSecurityWpa(p_context);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot set WPA security\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    LowLevel_SetSecurity(p_context->wpaSecurityType,
                         0, // not used
                         p_context->keyInfo.key,
                         p_context->keyInfo.keyLength);
}

void WF_SecurityWpsSet(t_wpsContext *p_context)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetSecurityWps(p_context);
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot set WPS security\n", __func__);
        return;
    }
#endif /* WF_ERROR_CHECKING */

    LowLevel_SetSecurity(p_context->wpsSecurityType,
                         0,
                         p_context->wpsPin,
                         p_context->wpsPinLength);

    // if host wants the host to calculate a binary key from a possible WPS-PSK passphrase
    if (p_context->getPassPhrase) {
        // tell MRF24WG to send wpa-psk passphrase back to host (if AP using WPA passphrase)
        YieldPassPhraseToHost();

        // save pointer to passphrase info block
        g_p_wpaKeyInfo = p_context->p_keyInfo;
    }
}

t_wpaKeyInfo *GetWpsPassPhraseInfo()
{
   return g_p_wpaKeyInfo;
}

void WF_WpsKeyGenerate()
{
    // create the binary key
    WF_WpaConvPassphraseToKey(g_p_wpaKeyInfo);

    // send it to MRF24WG
    SetPSK(g_p_wpaKeyInfo->key);
}

void WF_BssidSet(u_int8_t *p_bssid)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdSetBssid(p_bssid);
    if (errorCode != UD_SUCCESS) {
        return;
    }

#endif
    LowLevel_CPGetElement(WF_CP_ELEMENT_BSSID, p_bssid, WF_BSSID_LENGTH);
}

// called from SetAdhocContext().  Error checking performed there
void SetHiddenSsid(bool hiddenSsid)
{
    LowLevel_CPSetElement(WF_CP_ELEMENT_SSID_TYPE, (u_int8_t*) &hiddenSsid, 1);
}

// called from SetAdhocContext().  Error checking performed there
void SetAdHocMode(int mode)
{
    LowLevel_CPSetElement(WF_CP_ELEMENT_ADHOC_BEHAVIOR, (u_int8_t*) &mode, 1);
}

void WF_WpsCredentialsGet(t_wpsCredentials *p_cred)
{
#if defined(WF_ERROR_CHECKING)
    u_int32_t errorCode = UdGetWpsCredentials();
    if (errorCode != UD_SUCCESS) {
        printf("--- %s: cannot get WPS credentials\n", __func__);
        return;
    }
#endif
    LowLevel_CPGetElement(WF_CP_ELEMENT_READ_WPS_CRED, (u_int8_t*) p_cred, sizeof(*p_cred));
}
