/*
 * MRF24WG Universal Driver RAW
 *
 * This module contains definitions for RAW window access
 */
#ifndef __WF_RAW_H
#define __WF_RAW_H

#define NUM_RAW_WINDOWS                 6   /* only using raw windows 0 thru 4 */

/*
 * Usage of RAW Windows
 */
#define RAW_DATA_RX_ID                  0
#define RAW_DATA_TX_ID                  1
#define RAW_MGMT_RX_ID                  2
#define RAW_MGMT_TX_ID                  3
#define RAW_SCRATCH_ID                  4
#define RAW_UNUSED_ID                   5

/*
 * Source/Destination objects on the MRF24W
 */
#define RAW_MAC                         0x00    /* Cmd processor (aka MRF24W MAC) */
#define RAW_MGMT_POOL                   0x10    /* For 802.11 Management packets */
#define RAW_DATA_POOL                   0x20    /* Data Memory pool used for tx and rx operations */
#define RAW_SCRATCH_POOL                0x30    /* Scratch object */
#define RAW_STACK_MEM                   0x40    /* single level stack to save state of RAW */
#define RAW_COPY                        0x70    /* RAW to RAW copy */

/*
 * RAW Window states
 */
#define WF_RAW_UNMOUNTED                0
#define WF_SCRATCH_MOUNTED              1
#define WF_RAW_DATA_MOUNTED             2
#define WF_RAW_MGMT_MOUNTED             3

void        RawInit(void);
void        ScratchUnmount(u_int8_t rawId);
u_int16_t   ScratchMount(u_int8_t rawId);
bool        isWaitingForRawMoveCompleteInterrupt(void);
void        ClearWaitingForRawMoveCompleteInterrupt(void);
void        SignalRawInterruptEvent(u_int8_t rawIntMask);
bool        AllocateMgmtTxBuffer(u_int16_t bytesNeeded);
void        DeallocateMgmtRxBuffer(void);
void        RawSetByte(u_int16_t rawId, const u_int8_t *p_buffer, u_int16_t length);
void        RawGetByte(u_int16_t rawId, u_int8_t *pBuffer, u_int16_t length);
void        SendRAWManagementFrame(u_int16_t bufLen);
u_int16_t   RawMountRxBuffer(u_int8_t rawId);
void        RawSetIndex(u_int16_t rawId, u_int16_t index);
void        RawRead(u_int8_t rawId, u_int16_t startIndex, u_int16_t length, u_int8_t *p_dest);
void        RawWrite(u_int8_t rawId, u_int16_t startIndex, u_int16_t length, const u_int8_t *p_src);
bool        AllocateDataTxBuffer(u_int16_t bytesNeeded);
void        DeallocateDataRxBuffer(void);
u_int16_t   RawMove(u_int16_t rawId,
                    u_int16_t srcDest,
                    bool      rawIsDestination,
                    u_int16_t size);
void SetRawDataWindowState(u_int8_t rawId, u_int8_t state);
u_int8_t GetRawDataWindowState(u_int8_t rawId);

#endif /* __WF_RAW_H */
