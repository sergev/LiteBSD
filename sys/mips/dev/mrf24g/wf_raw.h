/*
 * MRF24WG Universal Driver RAW
 *
 * This module contains definitions for RAW window access
 */
#ifndef __WF_RAW_H
#define __WF_RAW_H

#define NUM_RAW_WINDOWS                 (6) /* only using raw windows 0 thru 4 */

/* Supported RAW Windows */
#define RAW_ID_0                        (0)
#define RAW_ID_1                        (1)
#define RAW_ID_2                        (2)
#define RAW_ID_3                        (3)
#define RAW_ID_4                        (4)
#define RAW_ID_5                        (5)

/* Usage of RAW Windows */
#define RAW_DATA_RX_ID                  RAW_ID_0
#define RAW_DATA_TX_ID                  RAW_ID_1
#define RAW_MGMT_RX_ID                  RAW_ID_2
#define RAW_MGMT_TX_ID                  RAW_ID_3
#define RAW_SCRATCH_ID                  RAW_ID_4
#define RAW_UNUSED_ID                   RAW_ID_5

// Source/Destination objects on the MRF24W
#define RAW_MAC                         (0x00)   /* Cmd processor (aka MRF24W MAC)                 */
#define RAW_MGMT_POOL                   (0x10)   /* For 802.11 Management packets                  */
#define RAW_DATA_POOL                   (0x20)   /* Data Memory pool used for tx and rx operations */
#define RAW_SCRATCH_POOL                (0x30)   /* Scratch object                                 */
#define RAW_STACK_MEM                   (0x40)   /* single level stack to save state of RAW        */
#define RAW_COPY                        (0x70)   /* RAW to RAW copy                                */

/*---------------------*/
/* 8-bit RAW registers */
/*---------------------*/
#define RAW_0_DATA_REG                  (0x20)   /* Data Rx       */
#define RAW_1_DATA_REG                  (0x21)   /* Data Tx       */
#define RAW_2_DATA_REG                  (0x06)   /* Mgmt Rx       */
#define RAW_3_DATA_REG                  (0x07)   /* Mgmt Tx       */
#define RAW_4_DATA_REG                  (0x08)   /* Scratch Tx/Rx */
#define RAW_5_DATA_REG                  (0x09)   /* not used      */

/*----------------------*/
/* 16 bit RAW registers */
/*----------------------*/
#define RAW_0_CTRL_0_REG                (0x25)      /* RAW 0 -- Data Rx       */
#define RAW_0_CTRL_1_REG                (0x26)
#define RAW_0_INDEX_REG                 (0x27)
#define RAW_0_STATUS_REG                (0x28)

#define RAW_1_CTRL_0_REG                (0x29)      /* RAW 1 -- Data Tx       */
#define RAW_1_CTRL_1_REG                (0x2a)
#define RAW_1_INDEX_REG                 (0x2b)
#define RAW_1_STATUS_REG                (0x2c)

#define RAW_2_CTRL_0_REG                (0x18)      /* RAW 2 -- Mgmt Rx       */
#define RAW_2_CTRL_1_REG                (0x19)
#define RAW_2_INDEX_REG                 (0x1a)
#define RAW_2_STATUS_REG                (0x1b)

#define RAW_3_CTRL_0_REG                (0x1c)      /* RAW 3 -- Mgmt Tx       */
#define RAW_3_CTRL_1_REG                (0x1d)
#define RAW_3_INDEX_REG                 (0x1e)
#define RAW_3_STATUS_REG                (0x1f)

#define RAW_4_CTRL_0_REG                (0x0a)      /* RAW 4 -- Scratch Tx/Rx */
#define RAW_4_CTRL_1_REG                (0x0b)
#define RAW_4_INDEX_REG                 (0x0c)
#define RAW_4_STATUS_REG                (0x0d)

#define RAW_5_CTRL_0_REG                (0x0e)      /* RAW 5 -- Not used       */
#define RAW_5_CTRL_1_REG                (0x0f)
#define RAW_5_INDEX_REG                 (0x22)
#define RAW_5_STATUS_REG                (0x23)

/* RAW Window states */
#define WF_RAW_UNMOUNTED                (0)
#define WF_SCRATCH_MOUNTED              (1)
#define WF_RAW_DATA_MOUNTED             (2)
#define WF_RAW_MGMT_MOUNTED             (3)

void        RawInit(void);
void        ScratchUnmount(u_int8_t rawId);
u_int16_t    ScratchMount(u_int8_t rawId);
bool        isWaitingForRawMoveCompleteInterrupt(void);
void        ClearWaitingForRawMoveCompleteInterrupt(void);
void        SignalRawInterruptEvent(u_int8_t rawIntMask);
bool        AllocateMgmtTxBuffer(u_int16_t bytesNeeded);
void        DeallocateMgmtRxBuffer(void);
void        RawSetByte(u_int16_t rawId, const u_int8_t *p_buffer, u_int16_t length);
void        RawGetByte(u_int16_t rawId, u_int8_t *pBuffer, u_int16_t length);
void        SendRAWManagementFrame(u_int16_t bufLen);
u_int16_t    RawMountRxBuffer(u_int8_t rawId);
void        RawSetIndex(u_int16_t rawId, u_int16_t index);
void        RawRead(u_int8_t rawId, u_int16_t startIndex, u_int16_t length, u_int8_t *p_dest);
void        RawWrite(u_int8_t rawId, u_int16_t startIndex, u_int16_t length, const u_int8_t *p_src);
bool        AllocateDataTxBuffer(u_int16_t bytesNeeded);
void        DeallocateDataRxBuffer(void);
u_int16_t    RawMove(u_int16_t rawId,
                    u_int16_t srcDest,
                    bool     rawIsDestination,
                    u_int16_t size);
void SetRawDataWindowState(u_int8_t rawId, u_int8_t state);
u_int8_t GetRawDataWindowState(u_int8_t rawId);

#endif /* __WF_RAW_H */
