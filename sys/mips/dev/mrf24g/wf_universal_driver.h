/*
 * MRF24WG primary header file
 *
 * Function prototypes and defines for Universal Driver API.
 */
#ifndef __MRF24WG_UNIVERSAL_DRIVER_API_H
#define __MRF24WG_UNIVERSAL_DRIVER_API_H

#include "wf_registers.h"
#include "wf_events.h"
#include "wf_connection_event_codes.h"
#include "wf_mgmt_msg.h"
#include "wf_raw.h"

//==============================================================================
//                                  DEFINES/CONSTANTS
//==============================================================================
#define WF_MAC_ADDRESS_LENGTH               6       // MAC addresses are always 6 bytes
#define WF_BSSID_LENGTH                     6       // BSSID is always 6 bytes
#define WF_MAX_SSID_LENGTH                  32      // does not include string terminator
#define WF_MAX_PASSPHRASE_LENGTH            64      // must include string terminator
#define WF_MAX_CHANNEL_LIST_LENGTH          14      // max channel list (for Japan) is 1-14
#define WF_WPS_PIN_LENGTH                   8       // 7 digits + checksum byte
#define WF_WPA_KEY_LENGTH                   32      // WPA binary key always 32 bytes
#define WF_WEP40_KEY_LENGTH                 20      // 4 keys of 5 bytes each
#define WF_WEP104_KEY_LENGTH                52      // 4 keys of 13 bytes each
#define WF_MAX_WEP_KEY_LENGTH               WF_WEP104_KEY_LENGTH
#define WF_NO_EVENT_DATA                    0xffffffffU
#define WF_MAX_NUM_RATES                    8

//==============================================================================
// DEFAULTS - After a reset, the MRF24WG has the following default settings.  These
//            can also be used in various API calls.
//==============================================================================
// see t_scanContext
#define WF_DEFAULT_SCAN_TYPE                WF_ACTIVE_SCAN
#define WF_DEFAULT_SCAN_COUNT               1
#define WF_DEFAULT_SCAN_MIN_CHANNEL_TIME    200     // ms
#define WF_DEFAULT_SCAN_MAX_CHANNEL_TIME    400     // ms
#define WF_DEFAULT_SCAN_PROBE_DELAY         20      // us

// see mrf_conn_set_adhoc()
#define WF_DEFAULT_ADHOC_HIDDEN_SSID        0
#define WF_DEFAULT_ADHOC_BEACON_PERIOD      100     // ms
#define WF_DEFAULT_ADHOC_MODE               WF_ADHOC_CONNECT_THEN_START

// see mrf_powersave_enable()
#define WF_DEFAULT_PS_LISTEN_INTERVAL       1       // 100ms multiplier, e.g. 1 * 100ms = 100ms
#define WF_DEFAULT_PS_DTIM_INTERVAL         2       // number of beacon periods
#define WF_DEFAULT_PS_DTIM_ENABLED          true    // DTIM wake-up enabled (normally the case)

// see t_txmode
#define WF_DEFAULT_TX_MODE                  WF_TXMODE_G_RATES   // full 802.11 g rates

// see mrf_conn_set_rssi()
#define WF_DEFAULT_RSSI                     255     // connect to highest RSSI found

// see mrf_set_rts_threshold()
#define WF_DEFAULT_RTS_THRESHOLD            2347

// see mrf_profile_set_wep()
#define WF_DEFAULT_WEP_KEY_INDEX            0
#define WF_DEFAULT_WEP_KEY_TYPE             WF_SECURITY_WEP_OPENKEY

//===================
// End DEFAULTS block
//===================

// See mrf_set_regional_domain()
enum {
    WF_DOMAIN_FCC           = 0,        // Available Channels: 1 - 11
    WF_DOMAIN_ETSI          = 2,        // Available Channels: 1 - 13
    WF_DOMAIN_JAPAN         = 7,        // Available Channels: 1 - 14
    WF_DOMAIN_OTHER         = 7,        // Available Channels: 1 - 14
};

// WiFi security modes
enum {
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

    WF_MAX_SECURITY_TYPE                     = 10,
};

// mrf_conn_set_scan()
enum {
    WF_SCAN_ACTIVE          = 1,
    WF_SCAN_PASSIVE         = 2,
};

// see mrf_profile_set_network_type()
enum {
    WF_MIN_NETWORK_TYPE            = 1,

    WF_NETWORK_TYPE_INFRASTRUCTURE = 1,
    WF_NETWORK_TYPE_ADHOC          = 2,
    WF_NETWORK_TYPE_P2P            = 3,

    WF_MAX_NETWORK_TYPE            = 3,
};

// see mrf_profile_set_wep()
enum {
    WF_SECURITY_WEP_SHAREDKEY = 0,
    WF_SECURITY_WEP_OPENKEY   = 1,
};

// see mrf_conn_set_adhoc()
enum {
    WF_ADHOC_CONNECT_THEN_START = 0,
    WF_ADHOC_CONNECT_ONLY       = 1,
    WF_ADHOC_START_ONLY         = 2,
};

// See mrf_set_tx_mode()
enum {
    WF_TXMODE_G_RATES       = 0,
    WF_TXMODE_B_RATES       = 1,
    WF_TXMODE_LEGACY_RATES  = 2,
};

// see mrf_conn_get_state()
enum {
    WF_CSTATE_NOT_CONNECTED               = 1,
    WF_CSTATE_CONNECTION_IN_PROGRESS      = 2,
    WF_CSTATE_CONNECTED_INFRASTRUCTURE    = 3,
    WF_CSTATE_CONNECTED_ADHOC             = 4,
    WF_CSTATE_RECONNECTION_IN_PROGRESS    = 5,
    WF_CSTATE_CONNECTION_PERMANENTLY_LOST = 6,
};

// Multicast Filter IDs
enum {
    WF_MULTICAST_FILTER_1   = 4,
    WF_MULTICAST_FILTER_2   = 5,
};

// see mrf_profile_get_wps_credentials()
enum {
    WF_WPS_AUTH_OPEN        = 0x01,
    WF_WPS_AUTH_WPA_PSK     = 0x02,
    WF_WPS_AUTH_SHARED      = 0x04,
    WF_WPS_AUTH_WPA         = 0x08,
    WF_WPS_AUTH_WPA2        = 0x10,
    WF_WPS_AUTH_WPA2_PSK    = 0x20,
};

// see mrf_profile_get_wps_credentials()
enum {
    WF_WPS_ENC_NONE         = 0x01,
    WF_WPS_ENC_WEP          = 0x02,
    WF_WPS_ENC_TKIP         = 0x04,
    WF_WPS_ENC_AES          = 0x08,
};

// Event Types - see mrf_event()
typedef enum {
    // Connection events
    WF_EVENT_CONNECTION_SUCCESSFUL          = 1,
    WF_EVENT_CONNECTION_FAILED              = 2,
    WF_EVENT_CONNECTION_TEMPORARILY_LOST    = 3,
    WF_EVENT_CONNECTION_PERMANENTLY_LOST    = 4,
    WF_EVENT_CONNECTION_REESTABLISHED       = 5,

    // WiFi scan event
    WF_EVENT_SCAN_RESULTS_READY             = 6,

    // WPS WPA-PSK key calculation needed
    WF_WPS_EVENT_KEY_CALCULATION_REQUEST    = 8,

    // Error events
    WF_EVENT_MRF24WG_MODULE_ASSERT          = 52,
} event_t;

//==============================================================================
//                                  DATA TYPES
//==============================================================================

typedef struct {
    char pass[WF_MAX_PASSPHRASE_LENGTH];    /* passphrase */
    u_int8_t pass_len;                      /* number of bytes in passphrase */
    u_int8_t ssid[WF_MAX_SSID_LENGTH];      /* ssid */
    u_int8_t ssid_len;                      /* number of bytes in SSID */
} key_info_t;

// See mrf_scan_get_result()
typedef struct {
    u_int8_t    bssid[WF_BSSID_LENGTH];     // Network BSSID value
    u_int8_t    ssid[WF_MAX_SSID_LENGTH];   // Network SSID value

    /*
     * Access point configuration
     *
     *   Bit 7  Bit 6  Bit 5     Bit 4    Bit 3     Bit 2     Bit 1     Bit 0
     *   -----  -----  -----     -----    -----     -----     -----     -----
     *   WPA2   WPA    Preamble  Privacy  Reserved  Reserved  Reserved  IE
     *
     * IE         1 if AP broadcasting one or more Information Elements, else 0
     * Privacy    0: AP is open (no security)
     *            1: AP using security,  if neither WPA and WPA2 set then security is WEP.
     * Preamble   0: AP transmitting with short preamble
     *            1: AP transmitting with long preamble
     * WPA        Only valid if Privacy is 1.
     *            0: AP does not support WPA
     *            1: AP supports WPA
     * WPA2       Only valid if Privacy is 1.
     *            0: AP does not support WPA2
     *            1: AP supports WPA2
     */
    u_int8_t    apConfig;
    u_int8_t    reserved;
    u_int16_t   beaconPeriod;   // Network beacon interval
    u_int16_t   atimWindow;     // Only valid if bssType = WF_INFRASTRUCTURE

    /*
     * List of Network basic rates.  Each rate has the following format:
     *
     * Bit 7
     *   0: rate is not part of the basic rates set
     *   1: rate is part of the basic rates set
     *
     * Bits 6:0
     *   Multiple of 500kbps giving the supported rate.  For example, a value of 2
     *   (2 * 500kbps) indicates that 1mbps is a supported rate.  A value of 4 in
     *   this field indicates a 2mbps rate (4 * 500kbps).
     */
    u_int8_t    basicRateSet[WF_MAX_NUM_RATES];
    u_int8_t    rssi;                           // Signal strength of received frame beacon or probe response
    u_int8_t    numRates;                       // Number of valid rates in basicRates
    u_int8_t    dtimPeriod;                     // Part of TIM element
    u_int8_t    bssType;                        // WF_INFRASTRUCTURE or WF_ADHOC
    u_int8_t    channel;                        // Channel number
    u_int8_t    ssidLen;                        // Number of valid characters in ssid
} scan_result_t;

typedef struct {
    u_int8_t  ssid[WF_MAX_SSID_LENGTH];         // network SSID
    u_int8_t  netKey[WF_MAX_PASSPHRASE_LENGTH]; // binary security key
    u_int16_t authType;                         // see WF_WPS_AUTH_*
    u_int16_t encType;                          // see WF_WPS_ENC_*
    u_int8_t  netIdx;                           // not used
    u_int8_t  ssidLen;                          // SSID length
    u_int8_t  keyIdx;                           // WEP key index (only valid if encType = WF_ENC_WEP)
    u_int8_t  keyLen;                           // key length, in bytes
    u_int8_t  bssid[WF_MAC_ADDRESS_LENGTH];     // BSSID
} wps_credentials_t;

// MAC Statistics
typedef struct {
    u_int32_t wep_exclude;      // Number of frames received with the Protected Frame subfield of the Frame
                                //  Control field set to zero and the value of dot11ExcludeUnencrypted causes
                                //  that frame to be discarded
    u_int32_t tx_bytes;         // Total number of Tx bytes that have been transmitted
    u_int32_t tx_multicast;     // Number of frames successfully transmitted that had the multicast bit set
                                //  in the destination MAC address.
    u_int32_t tx_failed;        // Number of Tx frames that failed due to the number of transmits exceeding the retry count
    u_int32_t tx_rtry;          // Number of times a transmitted frame needed to be retried
    u_int32_t tx_mult_rtry;     // Number of times a frame was successfully transmitted after more than one retransmission.
    u_int32_t tx_success;       // Number of Tx frames successfully transmitted.
    u_int32_t rx_dup;           // Number of frames received where the Sequence Control field indicates a duplicate.
    u_int32_t rx_cts_succ;      // Number of CTS frames received in response to an RTS frame.
    u_int32_t rx_cts_fail;      // Number of times an RTS frame was not received in response to a CTS frame.
    u_int32_t rx_ack_fail;      // Number of times an Ack was not received in response to a Tx frame.
    u_int32_t rx_bytes;         // Total number of Rx bytes received.
    u_int32_t rx_frag;          // Number of successful received frames (management or data)
    u_int32_t rx_mult;          // Number of frames received with the multicast bit set in the destination MAC address.
    u_int32_t rx_fcs_err;       // Number of frames received with an invalid Frame Checksum (FCS).
    u_int32_t rx_wep_undecrypt; // Number of frames received where the Protected Frame subfield of the Frame Control Field is set to
                                //  one and the WEPOn value for the key mapped to the transmitter's MAC address indicates the frame
                                //  should not have been encrypted.
    u_int32_t rx_frag_aged;     // Number of times that fragments 'aged out', or were not received in the allowable time.
    u_int32_t rx_mic_failure;   // Number of MIC failures that have occurred.
} mac_stats_t;

//==============================================================================
//                                  MRF24WG API
//==============================================================================

// WiFi init functions
//-----------------------
unsigned mrf_setup(void);                       // must be called first
void mrf_intr_init(void);
void mrf_event(event_t event, void *data);

// Profile configuration functions
//----------------------------------------------------------
unsigned mrf_profile_create(void);
void mrf_profile_set_ssid(unsigned cpid, u_int8_t *ssid, unsigned ssid_len);
void mrf_profile_set_network_type(unsigned cpid, unsigned nettype);
void mrf_profile_set_hidden(unsigned cpid, int hidden);
void mrf_profile_set_adhoc_mode(unsigned cpid, int mode);
void mrf_profile_set_bssid(unsigned cpid, u_int8_t *bssid);
void mrf_profile_get_wps_credentials(unsigned cpid, wps_credentials_t *cred);

// WiFi security functions
//------------------------
void mrf_profile_set_open(unsigned cpid);
void mrf_profile_set_wep(unsigned cpid, unsigned wep_security_type,
    unsigned key_index, u_int8_t *key, unsigned key_len);
void mrf_profile_set_wpa(unsigned cpid, unsigned wpa_security_type,
    u_int8_t *key, unsigned key_len);
void mrf_profile_set_wps(unsigned cpid, unsigned wps_security_type,
    u_int8_t *pin, unsigned pin_len);
void mrf_passphrase_to_key(const char *passphrase,
    const u_int8_t *ssid, unsigned ssid_len, u_int8_t *key);

// WiFi Connection functions
//--------------------------
void mrf_connect(unsigned cpid);
void mrf_disconnect(void);
int mrf_conn_get_state(void);
void mrf_conn_set_channels(u_int8_t *channel_list, unsigned num_channels);
void mrf_conn_set_mode(unsigned retry_count, int reconnect_on_deauth,
    unsigned beacon_timeout, int reconnect_on_loss);
void mrf_conn_set_listen_interval(unsigned value);
void mrf_conn_set_dtim_interval(unsigned value);

// WiFi scanning functions
//------------------------
void mrf_scan_start(unsigned cpid);
void mrf_scan_get_result(unsigned list_index, scan_result_t *scan_result);

// Power-save mode (PS-Poll)
//--------------------------
void mrf_powersave_enable(unsigned listenInterval, unsigned dtimInterval, int useDtim);
void mrf_powersave_disable(void);
void mrf_powersave_activate(int enable);
void mrf_awake(void);

// Maximum Tx power functions
//---------------------------
unsigned mrf_get_max_power(void);

// WiFi status functions
//----------------------
unsigned mrf_get_system_version();
void mrf_get_stats(mac_stats_t *stats);

// Multicast filter functions
void mrf_set_multicast_filter(unsigned filter_id, u_int8_t *address);

// Data tx/rx functions
//------------------
int mrf_tx_allocate(unsigned bytes_needed);
void mrf_tx_copy(u_int8_t *buf, unsigned length);
void mrf_tx_start(unsigned packet_size);
unsigned mrf_rx_get_length(void);

// Miscellaneous functions
//------------------------
void mrf_set_regional_domain(unsigned regional_domain);
void mrf_set_mac_address(u_int8_t *mac_address);
void mrf_get_mac_address(u_int8_t *mac_address);
void mrf_set_tx_confirm(unsigned state);

// Advanced functions.  These are not typically needed because the MRF24WG
// defaults suffice for most applications.
//------------------------------------------------------------------------
void mrf_conn_set_scan(unsigned scan_type,      // see WF_SCAN_*
    unsigned scan_count, unsigned min_channel_msec, unsigned max_channel_msec,
    unsigned probe_delay_usec);
void mrf_conn_set_adhoc(unsigned cpid, int hidden_ssid, unsigned beacon_msec,
    unsigned mode);                             // see WF_ADHOC_*
void mrf_set_tx_mode(unsigned mode);
void mrf_conn_set_rssi(unsigned rssi);
void mrf_set_rts_threshold(unsigned level);
void mrf_set_link_down_threshold(unsigned level);

/*
 * Parameter message definitions
 */
void mrf_enable_module_operation(void);
void mrf_yield_passphrase_to_host(void);
void mrf_set_psk(u_int8_t *psk);

/*
 * 1ms Timer Function
 */
unsigned mrf_timer_read(void);
int mrf_timer_elapsed(unsigned start_time);

#endif /* __MRF24WG_UNIVERSAL_DRIVER_API_H */
