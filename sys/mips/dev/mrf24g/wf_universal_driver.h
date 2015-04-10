/*
 * MRF24WG primary header file
 *
 * Function prototypes and defines for Universal Driver API.
 */
#ifndef __MRF24WG_UNIVERSAL_DRIVER_API_H
#define __MRF24WG_UNIVERSAL_DRIVER_API_H

//==============================================================================
//                                  INCLUDES
//==============================================================================
#include <sys/types.h>

typedef int bool;

#include "wf_events.h"
#include "wf_connection_event_codes.h"
#include "wf_stubs.h"

#define UD_VERSION  "v1.2"

//==============================================================================
//                                  DEFINES/CONSTANTS
//==============================================================================
#define WF_MAC_ADDRESS_LENGTH               (6)         // MAC addresses are always 6 bytes
#define WF_BSSID_LENGTH                     (6)         // BSSID is always 6 bytes
#define WF_MAX_SSID_LENGTH                  (32)        // does not include string terminator
#define WF_MAX_PASSPHRASE_LENGTH            (64)        // must include string terminator
#define WF_MAX_CHANNEL_LIST_LENGTH          (14)        // max channel list (for Japan) is 1-14
#define WF_WPS_PIN_LENGTH                   (8)         // 7 digits + checksum byte
#define WF_WPA_KEY_LENGTH                   (32)        // WPA binary key always 32 bytes
#define WF_WEP40_KEY_LENGTH                 (20)        // 4 keys of 5 bytes each
#define WF_WEP104_KEY_LENGTH                (52)        // 4 keys of 13 bytes each
#define WF_MAX_WEP_KEY_LENGTH               (WF_WEP104_KEY_LENGTH)
#define WF_NO_EVENT_DATA                    ((u_int32_t)0xffffffff)
#define WF_RETRY_FOREVER                    (0xff)
#define WF_MAX_NUM_RATES                    (8)

//==============================================================================
// DEFAULTS - After a reset, the MRF24WG has the following default settings.  These
//            can also be used in various API calls.
//==============================================================================
// see t_scanContext
#define WF_DEFAULT_SCAN_TYPE                WF_ACTIVE_SCAN
#define WF_DEFAULT_SCAN_COUNT               (1)
#define WF_DEFAULT_SCAN_MIN_CHANNEL_TIME    (200)   // ms
#define WF_DEFAULT_SCAN_MAX_CHANNEL_TIME    (400)   // ms
#define WF_DEFAULT_SCAN_PROBE_DELAY         (20)    // us

// see t_adHocNetworkContext
#define WF_DEFAULT_ADHOC_HIDDEN_SSID        false
#define WF_DEFAULT_ADHOC_BEACON_PERIOD      (100)   // ms
#define WF_DEFAULT_ADHOC_MODE               WF_ADHOC_CONNECT_THEN_START

// see t_psPollContext
#define WF_DEFAULT_PS_LISTEN_INTERVAL       ((u_int16_t)1)      // 100ms multiplier, e.g. 1 * 100ms = 100ms
#define WF_DEFAULT_PS_DTIM_INTERVAL         ((u_int16_t)2)      // number of beacon periods
#define WF_DEFAULT_PS_DTIM_ENABLED          true                // DTIM wake-up enabled (normally the case)

// see t_txmode
#define WF_DEFAULT_TX_MODE                  WF_TXMODE_G_RATES   // full 802.11 g rates

// see WF_RssiSet()
#define WF_DEFAULT_RSSI                     (255)               // connect to highest RSSI found

// see WF_RtsThresholdSet()
#define WF_DEFAULT_RTS_THRESHOLD            ((u_int16_t)2347)

// see WF_SetSecurityWep() and t_WepContext
#define WF_DEFAULT_WEP_KEY_INDEX            (0)
#define WF_DEFAULT_WEP_KEY_TYPE             WF_SECURITY_WEP_OPENKEY

//===================
// End DEFAULTS block
//===================

enum
{
    WF_LOW  = 0,
    WF_HIGH = 1
};

enum
{
    WF_DISABLED = 0,
    WF_ENABLED  = 1
};

// See WF_DeviceInfoGet() and t_mrf24wgDeviceInfo
typedef enum t_deviceType
{
    WF_UNKNOWN_DEVICE = 0,
    WF_MRF24WB_DEVICE = 1,
    WF_MRF24WG_DEVICE = 2
} t_deviceType;

// See WF_SetRegionalDomain()
typedef enum t_regionalDomain
{
    WF_DOMAIN_FCC   = 0,    // Available Channels: 1 - 11
    WF_DOMAIN_ETSI  = 2,    // Available Channels: 1 - 13
    WF_DOMAIN_JAPAN = 7,    // Available Channels: 1 - 14
    WF_DOMAIN_OTHER = 7    // Available Channels: 1 - 14
} t_regionalDomain;

// WiFi security modes
typedef enum t_securityMode
{
    WF_SECURITY_OPEN                         = 0,
    WF_SECURITY_WEP_40                       = 1,
    WF_SECURITY_WEP_104                      = 2,
    WF_SECURITY_WPA_WITH_KEY                 = 3,
    WF_SECURITY_WPA_WITH_PASS_PHRASE         = 4,
    WF_SECURITY_WPA2_WITH_KEY                = 5,
    WF_SECURITY_WPA2_WITH_PASS_PHRASE        = 6,
    WF_SECURITY_WPA_AUTO_WITH_KEY            = 7,
    WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE    = 8,
    WF_SECURITY_WPS_PUSH_BUTTON              = 9,
    WF_SECURITY_WPS_PIN                      = 10,

    WF_MAX_SECURITY_TYPE                     = 10
} t_securityMode;

// see WF_SetReconnectMode
typedef enum t_retry
{
    WF_DO_NOT_ATTEMPT_TO_RECONNECT = 0,
    WF_ATTEMPT_TO_RECONNECT        = 1
} t_retry;

// WF_ScanContextSet()
typedef enum t_scanType
{
    WF_ACTIVE_SCAN  = 1,
    WF_PASSIVE_SCAN = 2
} t_scanType;

// see WF_SetNetworkType()
typedef enum t_networkType
{
    WF_MIN_NETWORK_TYPE            = 1,

    WF_NETWORK_TYPE_INFRASTRUCTURE = 1,
    WF_NETWORK_TYPE_ADHOC          = 2,
    WF_NETWORK_TYPE_P2P            = 3,

    WF_MAX_NETWORK_TYPE            = 3
} t_networkType;

// see WF_SetSecurityWep()
typedef enum t_wepKeyType
{
    WF_SECURITY_WEP_SHAREDKEY = 0,
    WF_SECURITY_WEP_OPENKEY   = 1
} t_wepKeyType;

// see WF_AdhocContextSet()
typedef enum t_adhocMode
{
    WF_ADHOC_CONNECT_THEN_START = 0,
    WF_ADHOC_CONNECT_ONLY       = 1,
    WF_ADHOC_START_ONLY         = 2
} t_adhocMode;

// see WF_PowerStateGet()
typedef enum t_powerSaveStates
{
    WF_PS_HIBERNATE             = 1,
    WF_PS_PS_POLL_DTIM_ENABLED  = 2,
    WF_PS_PS_POLL_DTIM_DISABLED = 3,
    WF_PS_OFF                   = 4
} t_powerSaveStates;

// See WF_TxModeSet()
typedef enum t_txMode
{
    WF_TXMODE_G_RATES       = 0,
    WF_TXMODE_B_RATES       = 1,
    WF_TXMODE_LEGACY_RATES  = 2
} t_txMode;

// see WF_ConnectionStateGet()
typedef enum t_connectionState
{
    WF_CSTATE_NOT_CONNECTED               = 1,
    WF_CSTATE_CONNECTION_IN_PROGRESS      = 2,
    WF_CSTATE_CONNECTED_INFRASTRUCTURE    = 3,
    WF_CSTATE_CONNECTED_ADHOC             = 4,
    WF_CSTATE_RECONNECTION_IN_PROGRESS    = 5,
    WF_CSTATE_CONNECTION_PERMANENTLY_LOST = 6
} t_connectionState;

// see WF_Scan()
typedef enum t_scanMode
{
    WF_SCAN_FILTERED = 0,
    WF_SCAN_ALL      = 1
} t_scanMode;

//------------------------------------------------------------------------------
// Multicast Filter ID's (see t_swMulticastConfig() and WF_SwMulticastFilterSet().
// AdHoc can only use WF_MULTICAST_FILTER_1 or WF_MULTICAST_FILTER_2.
//------------------------------------------------------------------------------
typedef enum t_multicastFilterId
{
    WF_MULTICAST_FILTER_1       = 4,
    WF_MULTICAST_FILTER_2       = 5,
    WF_MULTICAST_FILTER_3       = 6,
    WF_MULTICAST_FILTER_4       = 7,
    WF_MULTICAST_FILTER_5       = 8,
    WF_MULTICAST_FILTER_6       = 9,
    WF_MULTICAST_FILTER_7       = 10,
    WF_MULTICAST_FILTER_8       = 11,
    WF_MULTICAST_FILTER_9       = 12,
    WF_MULTICAST_FILTER_10      = 13,
    WF_MULTICAST_FILTER_11      = 14,
    WF_MULTICAST_FILTER_12      = 15,
    WF_MULTICAST_FILTER_13      = 16,
    WF_MULTICAST_FILTER_14      = 17,
    WF_MULTICAST_FILTER_15      = 18,
    WF_MULTICAST_FILTER_16      = 19
} t_multicastFilterId;

// see t_swMulticastConfig() and WF_SwMulticastFilterSet()
typedef enum t_multicastAction
{
    WF_MULTICAST_DISABLE_ALL    = 0,
    WF_MULTICAST_ENABLE_ALL     = 1,
    WF_MULTICAST_USE_FILTERS    = 2
} t_multicastAction;

// see WF_WpsCredentialsGet()
typedef enum t_wps_auth
{
    WF_AUTH_OPEN       = 0x01,
    WF_AUTH_WPA_PSK    = 0x02,
    WF_AUTH_SHARED     = 0x04,
    WF_AUTH_WPA        = 0x08,
    WF_AUTH_WPA2       = 0x10,
    WF_AUTH_WPA2_PSK   = 0x20
} t_wps_auth;

// see WF_WpsCredentialsGet()
typedef enum t_wps_enc
{
    WF_ENC_NONE        = 0x01,
    WF_ENC_WEP         = 0x02,
    WF_ENC_TKIP        = 0x04,
    WF_ENC_AES         = 0x08
} t_wps_enc;

//==============================================================================
//                                  DATA TYPES
//==============================================================================
// See WF_ScanContextSet()
typedef struct scanContext
{
    u_int8_t  scanType;                 // see t_scanType
    u_int8_t  scanCount;
    u_int16_t minChannelTime;           // ms
    u_int16_t maxChannelTime;           // ms
    u_int16_t probeDelay;               // us
} t_scanContext;

// See SetAdhocContext()
typedef struct adHocNetworkContext
{
    bool     hiddenSsid;                // True if SSID should be hidden, else False (normally False)
    u_int16_t beaconPeriod;             // beacon period, in ms
    u_int8_t  mode;                     // see t_adhoc mode
} t_adHocNetworkContext;

// used in WF_DeviceInfoGet()
typedef struct mrf24wgDeviceInfo
{
    u_int8_t  deviceType;               // see t_deviceType
    u_int8_t  romVersion;               // ROM version number
    u_int8_t  patchVersion;             // Patch version number
} t_deviceInfo;

// See WF_SecurityWepSet()
typedef struct wepContext
{
    u_int8_t wepSecurityType;               // WF_SECURITY_WEP_40 or WF_SECURITY_WEP_104
    u_int8_t wepKeyIndex;                   // 0 thru 3
    u_int8_t wepKey[WF_MAX_WEP_KEY_LENGTH]; // array containing WEP binary security key (4 5-byte keys for WEP-40, 4 13-byte keys for WEP-104)
    u_int8_t wepKeyLength;                  // number of bytes pointed to by p_wepKey
    u_int8_t wepKeyType;                    // WF_SECURITY_WEP_OPENKEY (default) or WF_SECURITY_WEP_SHAREDKEY
} t_wepContext;

// used in t_wpaContext and t_wpsContext
typedef struct
{
    u_int8_t key[WF_MAX_PASSPHRASE_LENGTH]; // binary key or passphrase
    u_int8_t keyLength;                     // number of bytes in binary key (always 32) or passphrase
    u_int8_t ssid[WF_MAX_SSID_LENGTH];      // ssid
    u_int8_t ssidLen;                       // number of bytes in SSID
} t_wpaKeyInfo;

// See WF_SetSecurityWpa()
typedef struct wpaContext
{
    u_int8_t wpaSecurityType;               // WF_SECURITY_WPA_WITH_KEY, WF_SECURITY_WPA_WITH_PASS_PHRASE,
                                            //  WF_SECURITY_WPA2_WITH_KEY, WF_SECURITY_WPA2_WITH_PASS_PHRASE
                                            //  WF_SECURITY_WPA_AUTO_WITH_KEY, WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE
    t_wpaKeyInfo keyInfo;
} t_wpaContext;

// See WF_SetSecurityWps()
typedef struct wpsContext
{
    u_int8_t wpsSecurityType;               // WF_SECURITY_WPS_PUSH_BUTTON or WF_SECURITY_WPS_PIN
    u_int8_t wpsPin[WF_WPS_PIN_LENGTH];     // if using WF_SECURITY_WPS_PIN then pointer to 8-digit pin
    u_int8_t wpsPinLength;                  // should always be 8
    bool    getPassPhrase;                  // True if ASCII passphrase should be sent back to host
                                            //  so host can (more quickly) calculate binary key.  False
                                            //  if the MRF24WG should calculate the binary key
    t_wpaKeyInfo *p_keyInfo;                // pointer to where the Universal driver will
                                            // store passphrase info (must be global memory)
} t_wpsContext;

// structure/union can be used in functions WF_SecurityWepSet, WF_SecurityWpaSet,
// and WF_SetSecurityWps
#if 0
typedef struct
{
    union
    {
        t_wepContext wepContext;
        t_wpaContext wpaContext;
        t_wpsContext wpsContext;
    } t_context;
} t_securityContext;
#endif

typedef union
{
    t_wepContext wepContext;
    t_wpaContext wpaContext;
    t_wpsContext wpsContext;
} t_securityContext;


// See WF_PsPollEnable()
typedef struct psPollContext
{
    u_int16_t listenInterval; // Number of 100ms intervals between instances when
                              // the MRF24W wakes up to receive buffered messages
                              // from the network (1 = 100ms, 2 = 200ms, etc.)
    u_int16_t dtimInterval;   // Number of DTIM intervals between instances when
                              // the MRF24W wakes up to receive buffered messages
                              // from the network.
    bool     useDtim;         // true if dtimInterval is being used, else false
} t_psPollContext;

// See WF_MacStatsGet()
typedef struct WFMacStatsStruct
{
    u_int32_t MibWEPExcludeCtr;     // Number of frames received with the Protected Frame subfield of the Frame
                                    //  Control field set to zero and the value of dot11ExcludeUnencrypted causes
                                    //  that frame to be discarded
    u_int32_t MibTxBytesCtr;        // Total number of Tx bytes that have been transmitted
    u_int32_t MibTxMulticastCtr;    // Number of frames successfully transmitted that had the multicast bit set
                                    //  in the destination MAC address.
    u_int32_t MibTxFailedCtr;       // Number of Tx frames that failed due to the number of transmits exceeding the retry count
    u_int32_t MibTxRtryCtr;         // Number of times a transmitted frame needed to be retried
    u_int32_t MibTxMultRtryCtr;     // Number of times a frame was successfully transmitted after more than one retransmission.
    u_int32_t MibTxSuccessCtr;      // Number of Tx frames successfully transmitted.
    u_int32_t MibRxDupCtr;          // Number of frames received where the Sequence Control field indicates a duplicate.
    u_int32_t MibRxCtsSuccCtr;      // Number of CTS frames received in response to an RTS frame.
    u_int32_t MibRxCtsFailCtr;      // Number of times an RTS frame was not received in response to a CTS frame.
    u_int32_t MibRxAckFailCtr;      // Number of times an Ack was not received in response to a Tx frame.
    u_int32_t MibRxBytesCtr;        // Total number of Rx bytes received.
    u_int32_t MibRxFragCtr;         // Number of successful received frames (management or data)
    u_int32_t MibRxMultCtr;         // Number of frames received with the multicast bit set in the destination MAC address.
    u_int32_t MibRxFCSErrCtr;       // Number of frames received with an invalid Frame Checksum (FCS).
    u_int32_t MibRxWEPUndecryptCtr; // Number of frames received where the Protected Frame subfield of the Frame Control Field is set to
                                    //  one and the WEPOn value for the key mapped to the transmitter's MAC address indicates the frame
                                    //  should not have been encrypted.
    u_int32_t MibRxFragAgedCtr;     // Number of times that fragments 'aged out', or were not received in the allowable time.
    u_int32_t MibRxMICFailureCtr;   // Number of MIC failures that have occurred.
} t_macStats;

// See WF_ScanResultGet()
typedef struct
{
    u_int8_t    bssid[WF_BSSID_LENGTH];     // Network BSSID value
    u_int8_t    ssid[WF_MAX_SSID_LENGTH];   // Network SSID value

    /**
      Access point configuration
      <table>
        Bit 7       Bit 6       Bit 5       Bit 4       Bit 3       Bit 2       Bit 1       Bit 0
        -----       -----       -----       -----       -----       -----       -----       -----
        WPA2        WPA         Preamble    Privacy     Reserved    Reserved    Reserved    IE
      </table>

      <table>
      IE         1 if AP broadcasting one or more Information Elements, else 0
      Privacy    0 : AP is open (no security)
                 1: AP using security,  if neither WPA and WPA2 set then security is WEP.
      Preamble   0: AP transmitting with short preamble
                 1: AP transmitting with long preamble
      WPA        Only valid if Privacy is 1.
                   0: AP does not support WPA
                   1: AP supports WPA
      WPA2       Only valid if Privacy is 1.
                   0: AP does not support WPA2
                   1: AP supports WPA2
      </table>
      */
    u_int8_t    apConfig;
    u_int8_t    reserved;
    u_int16_t   beaconPeriod;   // Network beacon interval
    u_int16_t   atimWindow;     // Only valid if bssType = WF_INFRASTRUCTURE

    /*
      List of Network basic rates.  Each rate has the following format:

      Bit 7
        * 0: rate is not part of the basic rates set
        * 1: rate is part of the basic rates set

      Bits 6:0
        Multiple of 500kbps giving the supported rate.  For example, a value of 2
        (2 * 500kbps) indicates that 1mbps is a supported rate.  A value of 4 in
        this field indicates a 2mbps rate (4 * 500kbps).
      */
    u_int8_t    basicRateSet[WF_MAX_NUM_RATES];
    u_int8_t    rssi;                                   // Signal strength of received frame beacon or probe response
    u_int8_t    numRates;                               // Number of valid rates in basicRates
    u_int8_t    dtimPeriod;                             // Part of TIM element
    u_int8_t    bssType;                                // WF_INFRASTRUCTURE or WF_ADHOC
    u_int8_t    channel;                                // Channel number
    u_int8_t    ssidLen;                                // Number of valid characters in ssid

} t_scanResult;

// see WF_SwMulticastFilterSet
typedef struct swMulticastConfigStruct
{
    u_int8_t filterId;
    u_int8_t action;
    u_int8_t macAddress[WF_MAC_ADDRESS_LENGTH];
    u_int8_t macBitMask;
} t_swMulticastConfig;

typedef struct wpsCredentialsStruct
{
    u_int8_t  ssid[WF_MAX_SSID_LENGTH];         // network SSID
    u_int8_t  netKey[WF_MAX_PASSPHRASE_LENGTH]; // binary security key
    u_int16_t authType;                         // see t_wps_auth
    u_int16_t encType;                          // see t_wps_enc
    u_int8_t  netIdx;                           // not used
    u_int8_t  ssidLen;                          // SSID length
    u_int8_t  keyIdx;                           // WEP key index (only valid if encType = WF_ENC_WEP)
    u_int8_t  keyLen;                           // key length, in bytes
    u_int8_t  bssid[WF_MAC_ADDRESS_LENGTH];     // BSSID
} t_wpsCredentials;

//==============================================================================
//                                  MRF24WG API
//==============================================================================

// WiFi init and task functions
//-----------------------------
void WF_Init(void);     // must be called first
#if defined(WF_RTOS)
    void WF_Task(void *arg);
#else
    void WF_Task(void);
#endif

// Core WiFi configuration functions (must always be called)
//----------------------------------------------------------
void WF_RegionalDomainSet(u_int8_t regionalDomain);
void WF_SsidSet(u_int8_t *p_ssid,  u_int8_t ssidLength);
void WF_NetworkTypeSet(u_int8_t networkType);
void WF_ChannelListSet(u_int8_t *p_channelList, u_int8_t numChannels);
void WF_ReconnectModeSet(u_int8_t retryCount, u_int8_t deauthAction, u_int8_t beaconTimeout, u_int8_t beaconTimeoutAction);

// WiFi security functions
//------------------------
void WF_SecurityOpenSet(void);
void WF_SecurityWepSet(t_wepContext *p_context);
void WF_SecurityWpaSet(t_wpaContext *p_context);
void WF_SecurityWpsSet(t_wpsContext *p_context);
void WF_WpsCredentialsGet(t_wpsCredentials *p_cred);
void WF_WpaConvPassphraseToKey(t_wpaKeyInfo *p_keyInfo);
void WF_WpsKeyGenerate(void);

// WiFi Connection functions
//--------------------------
void WF_Connect(void);
void WF_Disconnect(void);
void WF_ConnectionStateGet(u_int8_t *p_state);

// WiFi scanning functions
//------------------------
void WF_Scan(u_int8_t scanMode);
void WF_ScanResultGet(u_int8_t listIndex, t_scanResult *p_scanResult);

// Power-save mode (PS-Poll)
//--------------------------
void WF_PsPollEnable(t_psPollContext *p_context);
void WF_PsPollDisable(void);
void WF_Hibernate(void);
void WF_PowerStateGet(u_int8_t *p_powerState);

// Maximum Tx power functions
//---------------------------
void WF_TxPowerFactoryMaxGet(u_int8_t *p_maxTPower);
void WF_TxPowerMaxSet(u_int8_t maxTxPower);

// WiFi status functions
//----------------------
void WF_DeviceInfoGet(t_deviceInfo *p_deviceInfo);
void WF_MacStatsGet(t_macStats *p_stats);

// multicast filter functions
void WF_SetHwMultiCastFilter(u_int8_t multicastFilterId, u_int8_t multicastAddress[WF_MAC_ADDRESS_LENGTH]);

// data tx functions
//------------------
bool WF_TxPacketAllocate(u_int16_t bytesNeeded);
void WF_TxPacketCopy(u_int8_t *p_buf, u_int16_t length);
void WF_TxPacketTransmit(u_int16_t packetSize);

// data rx functions
//------------------
void WF_ProcessRxPacket(void);
u_int16_t WF_RxPacketLengthGet(void);
void WF_RxPacketCopy(u_int8_t *p_buf, u_int16_t len);
void WF_RxPacketDeallocate(void);

// Miscellaneous functions
//------------------------
void WF_MacAddressSet(u_int8_t *p_macAddress);
void WF_MacAddressGet(u_int8_t *p_macAddress);

// Advanced functions.  These are not typically needed because the MRF24WG
// defaults suffice for most applications.
//------------------------------------------------------------------------
void WF_ScanContextSet(t_scanContext *p_context);
void WF_AdhocContextSet(t_adHocNetworkContext *p_context);
void WF_TxModeSet(u_int8_t mode);
void WF_BssidSet(u_int8_t *p_bssid);
void WF_RssiSet(u_int8_t rssi);
void WF_RtsThresholdSet(u_int16_t rstThreshold);
void WF_LinkDownThresholdSet(u_int8_t threshold);

#endif /* __MRF24WG_UNIVERSAL_DRIVER_API_H */
