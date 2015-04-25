/*
 * MRF24WG Universal Driver RAW
 *
 * This module contains definitions for RAW window access
 */
#ifndef __WF_RAW_H
#define __WF_RAW_H

#define NUM_RAW_WINDOWS         6   /* Total number of raw windows */

/*
 * Usage of RAW Windows
 */
#define RAW_ID_RECEIVE          0   /* Network data receive */
#define RAW_ID_TRANSMIT         1   /* Network data transmit */
#define RAW_ID_MGMT_RX          2   /* Receive of management messages */
#define RAW_ID_MGMT_TX          3   /* Transmit of management messages */
#define RAW_ID_SCRATCH          4   /* Not used */
#define RAW_ID_UNUSED           5   /* Not used */

/*
 * Source/Destination objects for mrf_raw_move()
 */
#define RAW_MAC                 0x00    /* MRF24W MAC as command processor */
#define RAW_MGMT_POOL           0x10    /* For management packets */
#define RAW_DATA_POOL           0x20    /* For network packets */
#define RAW_SCRATCH_POOL        0x30    /* Scratch object, not used */
#define RAW_STACK_MEM           0x40    /* Single level stack to save state of RAW, not used */
#define RAW_COPY                0x70    /* RAW to RAW copy, not used */

void     mrf_raw_init(void);
unsigned mrf_raw_move(unsigned raw_id, unsigned raw_obj, int raw_is_destination, unsigned size);
void     mrf_raw_seek(unsigned raw_id, unsigned index);
void     mrf_raw_read(unsigned raw_id, u_int8_t *dest, unsigned nbytes);
void     mrf_raw_write(unsigned raw_id, const u_int8_t *src, unsigned nbytes);
void     mrf_raw_pread(unsigned raw_id, u_int8_t *dest, unsigned nbytes, unsigned offset);
void     mrf_raw_pwrite(unsigned raw_id, const u_int8_t *src, unsigned nbytes, unsigned offset);

/*
 * RAW message type definitions.
 */

/* Tx Message Types */
#define WF_TYPE_DATA_REQUEST        1   /* Network packet */
#define WF_TYPE_MGMT_REQUEST        2   /* Management message */

/* Rx Message Types */
#define WF_TYPE_DATA_TX_CONFIRM     1   /* Data packet transmitted */
#define WF_TYPE_MGMT_CONFIRM        2   /* Mgmt message transmitted */
#define WF_TYPE_DATA_RX_INDICATE    3   /* Data packet received */
#define WF_TYPE_MGMT_INDICATE       4   /* Mgmt message received */

/* Tx/Rx Data Message Subtypes */
#define WF_SUBTYPE_STD_DATA         1   /* Generic data */
#define WF_SUBTYPE_NULL_DATA        2
                                 /* 3 - reserved value */
#define WF_SUBTYPE_UNTAMPERED_DATA  4

#endif /* __WF_RAW_H */
