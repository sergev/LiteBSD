/*
 * MRF24WG Universal Driver Error Checking
 *
 * This module contains error checking definitions
 *
 * This module is an extension of wf_events.h, but specific to
 * WiFi connection-related events
 */
#ifndef __WF_UD_STATE_H
#define __WF_UD_STATE_H

//==============================================================================
//                                  CONSTANTS
//==============================================================================

#define DEFAULT_RETRY_COUNT         (3)
#define DEFAULT_SECURITY_TYPE       WF_SECURITY_OPEN
#define DEFAULT_NETWORK_TYPE        WF_NETWORK_TYPE_INFRASTRUCTURE

// bit masks for errorState field
#define INIT_INVALID                ((u_int32_t)0x00000001)
#define CHANNEL_LIST_INVALID        ((u_int32_t)0x00000002)
#define DOMAIN_INVALID              ((u_int32_t)0x00000004)
#define SSID_INVALID                ((u_int32_t)0x00000008)
#define RETRY_COUNT_INVALID         ((u_int32_t)0x00000010)
#define SECURITY_TYPE_INVALID       ((u_int32_t)0x00000020)
#define NETWORK_TYPE_INVALID        ((u_int32_t)0x00000040)
#define NETWORK_CONFIG_INVALID      ((u_int32_t)0x00000080)
#define SCAN_CONTEXT_INVALID        ((u_int32_t)0x00000100)

typedef enum t_internalConnectionState {
    CS_NOT_CONNECTED           = 0,
    CS_CONNECTION_IN_PROGRESS  = 1,
    CS_CONNECTED               = 2,
} t_internalConnectionState;

//==============================================================================
//                                  DATA TYPES
//==============================================================================
typedef struct udStateStruct {
    u_int32_t       errorState;    // bit mask
    u_int8_t        connectionState;
    u_int8_t        retryCount;

#if defined(WF_ERROR_CHECKING)
    u_int8_t domain;
    u_int8_t networkType;
    u_int8_t securityType;
    u_int8_t ssid[WF_MAX_SSID_LENGTH];
    u_int8_t ssidLength;
    u_int8_t channelList[WF_MAX_CHANNEL_LIST_LENGTH];
    u_int8_t numChannels;
#endif

} t_udState;


//==============================================================================
//                                  GLOBALS
//==============================================================================
extern t_udState g_udState;  // only accessed via macros from external modules


//==============================================================================
//                                  MACROS
//==============================================================================

#define udGetDomain()                   g_udState.domain
#define UdGetNetworkType()              g_udState.networkType
#define UdSetRetryCount(c)              g_udState.retryCount = c
#define UdGetGetryCount()               g_udState.retryCount
#define UdSetConnectionState(s)         g_udState.connectionState = s
#define UdGetConnectionState()          g_udState.connectionState
#define UdGetSecurityType()             g_udState.securityType

//==============================================================================
//                                  ERROR BITMAP MACROS
//==============================================================================
#define UdSetInitInvalid()              g_udState.errorState |= INIT_INVALID
#define UdSetInitValid()                g_udState.errorState &= ~INIT_INVALID
#define UdIsInitValid()                 ((g_udState.errorState & INIT_INVALID) == 0)

#define UdSetDomainInvalid()            g_udState.errorState |= DOMAIN_INVALID
#define UdSetDomainValid()              g_udState.errorState &= ~DOMAIN_INVALID

#define UdSetChannelListInvalid()       g_udState.errorState |= CHANNEL_LIST_INVALID
#define UdSetChannelListValid()         g_udState.errorState &= ~CHANNEL_LIST_INVALID

#define UdSetSsidInvalid()              g_udState.errorState |= SSID_INVALID
#define UdSetSsidValid()                g_udState.errorState &= ~SSID_INVALID

#define UdSetSecurityInvalid()          g_udState.errorState |= SECURITY_TYPE_INVALID
#define UdSetSecurityValid()            g_udState.errorState &= ~SECURITY_TYPE_INVALID

#define UdSetNetworkTypeInvalid()       g_udState.errorState |= NETWORK_TYPE_INVALID
#define UdSetNetworkTypeValid()         g_udState.errorState &= ~NETWORK_TYPE_INVALID

#define UdSetNetworkConfigInvalid()     g_udState.errorState |= NETWORK_CONFIG_INVALID
#define UdSetNetworkConfigValid()       g_udState.errorState &= ~NETWORK_CONFIG_INVALID

#define UdSetScanContextInvalid()       g_udState.errorState |= SCAN_CONTEXT_INVALID
#define UdSetScanContextValid()         g_udState.errorState &= ~SCAN_CONTEXT_INVALID

//==============================================================================
//                                  FUNCTION PROTOTYPES
//==============================================================================

void  UdStateInit(void);

#if defined(WF_ERROR_CHECKING)
    u_int32_t UdSetDomain(u_int8_t domain);
    u_int32_t UdSetChannelList(u_int8_t *p_channelList, u_int8_t numChannels);
    u_int32_t UdSetSsid(u_int8_t *p_ssid, u_int8_t ssidLength);
    u_int32_t UdSetNetworkType(u_int8_t networkType);
    u_int32_t UdSetSecurityOpen(void);
    u_int32_t UdSetSecurityWep(unsigned wep_security_type, unsigned key_index, u_int8_t *key, unsigned key_len);
    u_int32_t UdSetSecurityWpa(unsigned wpa_security_type, u_int8_t *key, unsigned key_len);
    u_int32_t UdSetSecurityWps(unsigned wps_security_type, u_int8_t *pin, unsigned pin_len);
    u_int32_t UdCheckConnectionConfig(void);
    u_int32_t UdSetReconnectMode(unsigned retryCount, int deauthAction, int beaconTimeoutAction);
    u_int32_t UdSetScanContext(unsigned scan_type);
    u_int32_t UdSetAdhocNetworkContext(int hidden_ssid, unsigned mode);
    u_int32_t UdSetTxMode(u_int8_t mode);
    u_int32_t UdSetBssid(u_int8_t *p_bssid);
    u_int32_t UdSetRssi(void);
    u_int32_t UdSetRtsThreshold(u_int16_t rtsThreshold);
    u_int32_t udSetTxPowerMax(u_int8_t maxTxPower);
    u_int32_t UdSetHwMulticastFilter(u_int8_t multicastFilterId, u_int8_t *p_multicastAddress);
    u_int32_t UdGetWpsCredentials(void);
#endif /* WF_ERROR_CHECKING */

#endif /* __WF_UD_STATE_H */
