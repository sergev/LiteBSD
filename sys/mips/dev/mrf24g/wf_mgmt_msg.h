/*
 * MRF24WG Universal Driver Management Messages
 *
 * This module contains management message defintions
 */
#ifndef __WF_MGMT_MSG_H
#define __WF_MGMT_MSG_H

#define WF_MAX_TX_MGMT_MSG_SIZE         128

/*
 * Management Message Request/Response Subtypes
 */
enum {
    /* Misc subtypes */
    WF_SUBTYPE_SCAN                             = 1,
    WF_SUBTYPE_JOIN                             = 2,
    WF_SUBTYPE_AUTH                             = 3,
    WF_SUBTYPE_ASSOC                            = 4,
    WF_SUBTYPE_DISCONNECT                       = 5,
    WF_SUBTYPE_DISASOCC                         = 6,
    WF_SUBTYPE_SET_POWER_MODE                   = 7,
    WF_SUBTYPE_SET_PM_KEY                       = 8,
    WF_SUBTYPE_SET_WEP_MAP                      = 9,
    WF_SUBTYPE_SET_WEP_KEY                      = 10,
    WF_SUBTYPE_SET_TEMP_KEY                     = 11,
    WF_SUBTYPE_CALC_PSK_KEY                     = 12,
    WF_SUBTYPE_SET_WEP_KEY_ID                   = 13,
    WF_SUBTYPE_CONFIG_KEY_SPACE                 = 14,
    WF_SUBTYPE_SET_PARAM                        = 15,
    WF_SUBTYPE_GET_PARAM                        = 16,
    WF_SUBTYPE_ADHOC_CONNECT                    = 17,
    WF_SUBTYPE_ADHOC_START                      = 18,

    /* Connection Profile Message Subtypes */
    WF_SUBTYPE_CP_CREATE_PROFILE                = 21,
    WF_SUBTYPE_CP_DELETE_PROFILE                = 22,
    WF_SUBTYPE_CP_GET_ID_LIST                   = 23,
    WF_SUBTYPE_CP_SET_ELEMENT                   = 24,
    WF_SUBTYPE_CP_GET_ELEMENT                   = 25,

    /* Connection Algorithm Message Subtypes */
    WF_SUBTYPE_CA_SET_ELEMENT                   = 26,
    WF_SUBTYPE_CA_GET_ELEMENT                   = 27,

    /* Connnection Manager Message Subtypes */
    WF_SUBTYPE_CM_CONNECT                       = 28,
    WF_SUBTYPE_CM_DISCONNECT                    = 29,
    WF_SUBTYPE_CM_GET_CONNECTION_STATUS         = 30,

    WF_SUBTYPE_SCAN_START                       = 31,
    WF_SUBTYPE_SCAN_GET_RESULTS                 = 32,

    WF_SUBTYPE_CM_INFO                          = 33,

    WF_SUBTYPE_SCAN_FOR_IE                      = 34,  /* not yet supported */
    WF_SUBTYPE_SCAN_IE_GET_RESULTS              = 35,  /* not yet supported */

    WF_SUBTYPE_CM_GET_CONNECTION_STATISTICS     = 36,  /* not yet supported so moved here for now */
    WF_SUBTYPE_NUM_REQUESTS
};

/*
 * Parameter message subtypes
 */
enum {
    PARAM_MAC_ADDRESS               = 1,    /* the device MAC address (6 bytes) */
    PARAM_REGIONAL_DOMAIN           = 2,    /* the device Regional Domain (1 byte) */
    PARAM_RTS_THRESHOLD             = 3,    /* the RTS byte threshold 256 - 2347 (2 bytes) */
    PARAM_LONG_FRAME_RETRY_LIMIT    = 4,    /* the long Frame Retry limit  (1 byte) */
    PARAM_SHORT_FRAME_RETRY_LIMIT   = 5,    /* the short Frame Retry limit (1 byte) */
    PARAM_TX_LIFETIME_TU            = 6,    /* the Tx Request lifetime in TU's 0 - 4194303 (4 bytes) */
    PARAM_RX_LIFETIME_TU            = 7,    /* the Rx Frame lifetime in TU's 0 - 4194303 (4 bytes) */
    PARAM_SUPPLICANT_ON_OFF         = 8,    /* boolean 1 = on 0 = off (1 byte) */
    PARAM_CONFIRM_DATA_TX_REQ       = 9,    /* boolean 1 = on 0 = off (1 byte) */
    PARAM_MASTER_STATE              = 10,   /* master state of the MAC using enumerated values (1 byte) */
    PARAM_HOST_ALERT_BITS           = 11,   /* a bit field which enables/disables various asynchronous */
                                            /*   indications from the MAC to the host (2 bytes) */
    PARAM_NUM_MISSED_BEACONS        = 12,   /* number of consecutive beacons MAC can miss before it */
                                            /*   considers the network lost (1 byte) */
    PARAM_DIFS_AND_EIFS             = 13,   /* delay intervals in usec DIFS and EIFS ( 2 * 2 bytes) */
    PARAM_TX_POWER                  = 14,   /* max and min boundaries for Tx power (2 * 2 bytes) */
    PARAM_DEFAULT_DEST_MAC_ADDR     = 15,   /* stores a persistant destination MAC address for small */
                                            /*   Tx Requests (6 bytes) */
    PARAM_WPA_INFO_ELEMENT          = 16,   /* stores a WPA info element (IE) in 802.11 IE format.  Used */
                                            /*   in Assoc Request and Supplicant exchange (3 - 258 bytes) */
    PARAM_RSN_INFO_ELEMENT          = 17,   /* stores a RSN info element (IE) in 802.11 IE format.  Used */
                                            /*   in Assoc Request and Supplicant exchange (3 - 258 bytes) */
    PARAM_ON_OFF_RADIO              = 18,   /* bool to force a radio state change 1 = on 0 = off (1 byte) */
    PARAM_COMPARE_ADDRESS           = 19,   /* A MAC address used to filter received frames */
                                            /*   (sizeof(tAddressFilterInput) = 8 bytes) */
    PARAM_SUBTYPE_FILTER            = 20,   /* Bitfield used to filter received frames based on type and */
                                            /* sub-type (sizeof(tAddressFilterInput) = 4 bytes) */
    PARAM_ACK_CONTROL               = 21,   /* Bitfield used to control the type of frames that cause ACK */
                                            /*   responses (sizeof(tAckControlInput) = 4 bytes) */
    PARAM_STAT_COUNTERS             = 22,   /* Complete set of statistics counters that are maintained by */
                                            /*   the MAC */
    PARAM_TX_THROTTLE_TABLE         = 23,   /* Custom Tx Rate throttle table to be used to control tx Rate */
    PARAM_TX_THROTTLE_TABLE_ON_OFF  = 24,   /* A boolean to enable/disable use of the throttle Table and a */
                                            /*   tx rate to use if the throttle table is disabled */
    PARAM_TX_CONTENTION_ARRAY       = 25,   /* Custom Retry contention ladder used for backoff calculation */
                                            /*   prior to a Tx attempt */
    PARAM_SYSTEM_VERSION            = 26,   /* 2 byte representation of a version number for the ROM and */
                                            /*  Patch */
    PARAM_STATUE_INFO               = 27,   /* MAC State information */
    PARAM_SECURITY_CONTROL          = 28,   /* 2 byte data structure to enable/disable encryption */
    PARAM_FACTORY_TX_POWER          = 29,   /* gets the factory-set tx max/min power level */
    PARAM_MRF24WB0M                 = 30,   /* a set enables MRF24WB0M Mode, a get gets the version */
    PARAM_CONNECT_CONTEXT           = 31,   /* gets current connection status */
    PARAM_WICOM_MODE                = 32,   /* a set enables WiCom mode */
    PARAM_WPA_TIMEOUT               = 33,   /* configure 4 way handshaking transaction timeout */
    PARAM_TX_MODE                   = 34,   /* choose tx mode */
    PARAM_ENABLE_DEBUG_PRINT        = 35,   /* enable WPS / P2P debug prints */
    PARAM_BROADCAST_PROBE_RESPONSE  = 36,   /* enable broadcast probe response */
    PARAM_LINK_DOWN_THRESHOLD       = 37,   /* sets link down threshold */
    PARAM_STACK_VERSION             = 38,   /* notifies stack version to RF FW */
    PARAM_SET_PSK                   = 39,   /* set psk */
    PARAM_YIELD_PASSPHRASE_TOHOST   = 40,   /* forces module FW to send passphrase to host in WPS */
    PARAM_USE_SW_MULTICAST_FILTER   = 41    /* force to use sw multicast filter instead of hw filter */
};

/*
 * Connection Algorithm Element IDs.
 * Used in conjunction with the WF_CA_SET_ELEMENT_SUBTYPE and
 * WF_CA_GET_ELEMENT_SUBTYPE message subtypes.
 */
enum {
    WF_CA_ELEMENT_ALL                       = 0,
    WF_CA_ELEMENT_SCANTYPE                  = 1,
    WF_CA_ELEMENT_RSSI                      = 2,
    WF_CA_ELEMENT_CP_LIST                   = 3,
    WF_CA_ELEMENT_LIST_RETRY_COUNT          = 4,
    WF_CA_ELEMENT_EVENT_NOTIFICATION_ACTION = 5,
    WF_CA_ELEMENT_BEACON_TIMEOUT_ACTION     = 6,
    WF_CA_ELEMENT_DEAUTH_ACTION             = 7,
    WF_CA_ELEMENT_CHANNEL_LIST              = 8,
    WF_CA_ELEMENT_LISTEN_INTERVAL           = 9,
    WF_CA_ELEMENT_BEACON_TIMEOUT            = 10,
    WF_CA_ELEMENT_SCAN_COUNT                = 11,
    WF_CA_ELEMENT_MIN_CHANNEL_TIME          = 12,
    WF_CA_ELEMENT_MAX_CHANNEL_TIME          = 13,
    WF_CA_ELEMENT_PROBE_DELAY               = 14,
    WF_CA_ELEMENT_DTIM_INTERVAL             = 15,
    WF_CA_ELEMENT_BEACON_PERIOD             = 16
};

/*-------------------------------------------*/
/* Connection Manager Event Message Subtypes */
/* (Used in Mgmt Indicate messages) */
/*-------------------------------------------*/
enum {
    WF_EVENT_SUBTYPE_CONNECTION_ATTEMPT_STATUS  = 6,
    WF_EVENT_SUBTYPE_CONNECTION_LOST            = 7,
    WF_EVENT_SUBTYPE_CONNECTION_REESTABLISHED   = 8,
    WF_EVENT_SUBTYPE_KEY_CALCULATION_REQUEST    = 9,
    WF_EVENT_SUBTYPE_SCAN_RESULTS_READY         = 11,
    WF_EVENT_SUBTYPE_SCAN_IE_RESULTS_READY      = 12,
};

/* event values for index 2 of WF_CONNECTION_ATTEMPT_STATUS_EVENT_SUBTYPE */
#define CONNECTION_ATTEMPT_SUCCESSFUL   1   /* if not 1 then failed to connect and info field is error code */
#define CONNECTION_ATTEMPT_FAILED       2

/* event values for index 2 of WF_EVENT_CONNECTION_LOST_SUBTYPE */
#define CONNECTION_TEMPORARILY_LOST     1
#define CONNECTION_PERMANENTLY_LOST     2
#define CONNECTION_REESTABLISHED        3

void mrf_mgmt_send(u_int8_t *header, unsigned header_len,
        u_int8_t *data, unsigned data_len, int free_response);
void mrf_mgmt_send_receive(u_int8_t *header, unsigned header_len,
        u_int8_t *reply, unsigned reply_len, unsigned offset);
int mrf_mgmt_receive_confirm(void);

#endif /* __WF_MGMT_MSG_H */
