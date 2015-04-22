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

void        mrf_raw_init(void);
unsigned    mrf_raw_move(unsigned raw_id, unsigned src_dest, bool raw_is_destination, unsigned size);
void        mrf_raw_seek(unsigned raw_id, unsigned index);
void        mrf_raw_read(unsigned raw_id, u_int8_t *dest, unsigned length);
void        mrf_raw_write(unsigned raw_id, const u_int8_t *src, unsigned length);
void        mrf_raw_pread(unsigned raw_id, u_int8_t *dest, unsigned length, unsigned offset);
void        mrf_raw_pwrite(unsigned raw_id, const u_int8_t *src, unsigned length, unsigned offset);

#endif /* __WF_RAW_H */
