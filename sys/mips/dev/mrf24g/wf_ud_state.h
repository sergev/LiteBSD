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
#define INIT_INVALID                ((uint32_t)0x00000001)
#define CHANNEL_LIST_INVALID        ((uint32_t)0x00000002)
#define DOMAIN_INVALID              ((uint32_t)0x00000004)
#define SSID_INVALID                ((uint32_t)0x00000008)
#define RETRY_COUNT_INVALID         ((uint32_t)0x00000010)
#define SECURITY_TYPE_INVALID       ((uint32_t)0x00000020)
#define NETWORK_TYPE_INVALID        ((uint32_t)0x00000040)
#define NETWORK_CONFIG_INVALID      ((uint32_t)0x00000080)
#define SCAN_CONTEXT_INVALID        ((uint32_t)0x00000100)

typedef enum t_internalConnectionState
{
    CS_NOT_CONNECTED           = 0,
    CS_CONNECTION_IN_PROGRESS  = 1,
    CS_CONNECTED               = 2,
} t_internalConnectionState;

//==============================================================================
//                                  DATA TYPES
//==============================================================================
typedef struct udStateStruct
{
    uint32_t        errorState;    // bit mask
    uint8_t         connectionState;
    uint8_t         retryCount;
    bool            psPollEnabled;
    t_psPollContext psPollContext;

#if defined(WF_ERROR_CHECKING)
    uint8_t domain;
    uint8_t networkType;
    uint8_t securityType;
    uint8_t ssid[WF_MAX_SSID_LENGTH];
    uint8_t ssidLength;
    uint8_t channelList[WF_MAX_CHANNEL_LIST_LENGTH];
    uint8_t numChannels;
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
void  UdEnablePsPoll(t_psPollContext *p_context);
void  UdDisablePsPoll(void);
bool  UdisPsPollEnabled(void);

#if defined(WF_ERROR_CHECKING)
    uint32_t UdSetDomain(uint8_t domain);
    uint32_t UdSetChannelList(uint8_t *p_channelList, uint8_t numChannels);
    uint32_t UdSetSsid(uint8_t *p_ssid, uint8_t ssidLength);
    uint32_t UdSetNetworkType(uint8_t networkType);
    uint32_t UdSetSecurityOpen(void);
    uint32_t UdSetSecurityWep(t_wepContext *p_context);
    uint32_t UdSetSecurityWpa(t_wpaContext *p_context);
    uint32_t UdSetSecurityWps(t_wpsContext *p_context);
    uint32_t UdCheckConnectionConfig(void);
    uint32_t UdSetReconnectMode(uint8_t retryCount, uint8_t deauthAction, uint8_t beaconTimeout, uint8_t beaconTimeoutAction);
    uint32_t UdSetScanContext(t_scanContext *p_context);
    uint32_t UdSetAdhocNetworkContext(t_adHocNetworkContext *p_context);
    uint32_t UdSetTxMode(uint8_t mode);
    uint32_t UdSetBssid(uint8_t *p_bssid);
    uint32_t UdSetRssi(uint8_t rssi);
    uint32_t UdSetRtsThreshold(uint16_t rtsThreshold);
    uint32_t UdScan(uint8_t mode);
    uint32_t udSetTxPowerMax(uint8_t maxTxPower);
    uint32_t UdSetHwMulticastFilter(uint8_t multicastFilterId, uint8_t *p_multicastAddress);
    uint32_t UdConvWpaPassphrase(t_wpaKeyInfo *p_passPhrase);
    uint32_t UdGetWpsCredentials(void);
#endif /* WF_ERROR_CHECKING */

#endif /* __WF_UD_STATE_H */
