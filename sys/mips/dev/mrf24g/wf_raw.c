/*
 * MRF24WG RAW (Random Access Window)
 *
 * Functions to control RAW windows.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include "wf_universal_driver.h"

/*
 * Raw registers for each raw window being used
 */
static const u_int8_t raw_index_reg[NUM_RAW_WINDOWS] = {
    MRF24_REG_RAW0_INDEX,
    MRF24_REG_RAW1_INDEX,
    MRF24_REG_RAW2_INDEX,
    MRF24_REG_RAW3_INDEX,
    MRF24_REG_RAW4_INDEX,
    MRF24_REG_RAW5_INDEX,
};
static const u_int8_t raw_status_reg[NUM_RAW_WINDOWS] = {
    MRF24_REG_RAW0_STATUS,
    MRF24_REG_RAW1_STATUS,
    MRF24_REG_RAW2_STATUS,
    MRF24_REG_RAW3_STATUS,
    MRF24_REG_RAW4_STATUS,
    MRF24_REG_RAW5_STATUS,
};
static const u_int8_t raw_ctrl0_reg[NUM_RAW_WINDOWS] = {
    MRF24_REG_RAW0_CTRL0,
    MRF24_REG_RAW1_CTRL0,
    MRF24_REG_RAW2_CTRL0,
    MRF24_REG_RAW3_CTRL0,
    MRF24_REG_RAW4_CTRL0,
    MRF24_REG_RAW5_CTRL0,
};
static const u_int8_t raw_ctrl1_reg[NUM_RAW_WINDOWS] = {
    MRF24_REG_RAW0_CTRL1,
    MRF24_REG_RAW1_CTRL1,
    MRF24_REG_RAW2_CTRL1,
    MRF24_REG_RAW3_CTRL1,
    MRF24_REG_RAW4_CTRL1,
    MRF24_REG_RAW5_CTRL1,
};
static const u_int8_t raw_data_reg[NUM_RAW_WINDOWS] = {
    MRF24_REG_RAW0_DATA,
    MRF24_REG_RAW1_DATA,
    MRF24_REG_RAW2_DATA,
    MRF24_REG_RAW3_DATA,
    MRF24_REG_RAW4_DATA,
    MRF24_REG_RAW5_DATA,
};

/*
 * Interrupt mask for each raw window; note that raw0 and raw1 are really
 * 8 bit values and will be cast when used.
 */
static const u_int16_t raw_int_mask[NUM_RAW_WINDOWS] = {
    INTR_RAW0,      /* used in HOST_INTR reg (8-bit register) */
    INTR_RAW1,      /* used in HOST_INTR reg (8-bit register) */
    INTR2_RAW2,     /* used in HOST_INTR2 reg (16-bit register) */
    INTR2_RAW3,     /* used in HOST_INTR2 reg (16-bit register) */
    INTR2_RAW4,     /* used in HOST_INTR2 reg (16-bit register) */
    INTR2_RAW5,     /* used in HOST_INTR2 reg (16-bit register) */
};

/*
 * Perform RAW Move operation.
 * When applicable, return the number of bytes overlayed by the raw move.
 *
 * The function performs a variety of operations (e.g. allocating tx buffers,
 * mounting rx buffers, copying from one raw window to another, etc.)
 *
 * Parameters:
 *  raw_id -- Raw ID 0 thru 5, except is srcDest is RAW_COPY, in which case rawId
 *            contains the source address in the upper 4 bits and destination
 *            address in lower 4 bits.
 *
 *  src_dest -- object that will either be the source or destination of the move:
 *                RAW_MAC
 *                RAW_MGMT_POOL
 *                RAW_DATA_POOL
 *                RAW_SCRATCH_POOL
 *                RAW_STACK_MEM
 *                RAW_COPY (this object not allowed, handled in RawToRawCopy())
 *
 *  raw_is_destination -- true is srcDest is the destination, false if srcDest is
 *                        the source of the move
 *
 *  size -- number of bytes to overlay (not always applicable)
 */
unsigned mrf_raw_move(unsigned raw_id, unsigned src_dest,
    int raw_is_destination, unsigned size)
{
    unsigned intr, mask, start_time, nbytes, ctrl0 = 0;
    int x;

    // save current state of External interrupt and disable it
    x = splimp();

    /* Create control value that will be written to raw control register,
     * which initiates the raw move */
    if (raw_is_destination) {
        ctrl0 |= 0x8000;
    }
    /* fix later, simply need to ensure that size is 12 bits are less */
    ctrl0 |= (src_dest << 8);             /* defines are already shifted by 4 bits */
    ctrl0 |= ((size >> 8) & 0x0f) << 8;   /* MS 4 bits of size (bits 11:8) */
    ctrl0 |= (size & 0x00ff);             /* LS 8 bits of size (bits 7:0) */

    /*
     * This next 'if' block is used to ensure the expected raw interrupt
     * signifying raw move complete is cleared.
     */

    /* if doing a raw move on Raw 0 or 1 (data rx or data tx) */
    if (raw_id <= 1) {
        /* Clear the interrupt bit in the host interrupt register.
         * Raw 0 and 1 are in 8-bit host intr reg. */
        mrf_write_byte(MRF24_REG_INTR, raw_int_mask[raw_id]);
    }
    /* else doing raw move on mgmt rx, mgmt tx, or scratch */
    else {
        /* Clear the interrupt bit in the host interrupt 2 register.
         * Raw 2, 3, and 4 are in 16-bit host intr2 reg. */
        mrf_write(MRF24_REG_INTR2, raw_int_mask[raw_id]);
    }

    /*
     * Now that the expected raw move complete interrupt has been cleared
     * and we are ready to receive it, initiate the raw move operation by
     * writing to the appropriate RawCtrl0.
     */
    mrf_write(raw_ctrl0_reg[raw_id], ctrl0);

    /*
     * Wait for a RAW move to complete.
     */

    /* create mask to check against for Raw Move complete interrupt for either RAW0 or RAW1 */
    if (raw_id <= 1) {
        /* will be either raw 0 or raw 1 */
        mask = (raw_id == 0) ? INTR_RAW0 : INTR_RAW1;
    } else {
        /* will be INTR2 bit in host register, signifying RAW2, RAW3, or RAW4 */
        mask = INTR_INT2;
    }

    start_time = mrf_timer_read();
    for (;;) {
        intr = mrf_read_byte(MRF24_REG_INTR);

        /* If received an external interrupt that signaled the RAW Move
         * completed then break out of this loop. */
        if (intr & mask) {
            /* clear the RAW interrupts, re-enable interrupts, and exit */
            if (mask == INTR_INT2)
                mrf_write(MRF24_REG_INTR2,
                    INTR2_RAW2 | INTR2_RAW3 | INTR2_RAW4 | INTR2_RAW5);
            mrf_write_byte(MRF24_REG_INTR, mask);
            break;
        }

        if (mrf_timer_elapsed(start_time) > 20) {
            printf("--- %s: timeout waiting for interrupt\n", __func__);
            break;
        }
        udelay(10);
    }

    /* read the byte count and return it */
    nbytes = mrf_read(raw_ctrl1_reg[raw_id]);

    // if interrupts were enabled coming into this function, put back to that state
    splx(x);

    /* byte count is not valid for some raw move operations */
    return nbytes;
}

/*
 * Initialize RAW (Random Access Window) on MRF24WG
 */
void mrf_raw_init()
{
    /*
     * By default the MRF24WG firmware mounts Scratch to RAW 1 after reset.
     * This is not being used, so unmount the scratch from this RAW window.
     * The scratch window is not dynamically allocated, but references a static
     * portion of the WiFi device RAM. Thus, the Scratch data is not lost when
     * the scratch window is unmounted.
     */
    mrf_raw_move(1, RAW_SCRATCH_POOL, 0, 0);

    /*
     * Permanently mount scratch memory, index defaults to 0.
     * This function returns the number of bytes in scratch memory.
     */
    mrf_raw_move(RAW_ID_SCRATCH, RAW_SCRATCH_POOL, 1, 0);

    //SetRawDataWindowState(RAW_ID_DATA_TX, WF_RAW_UNMOUNTED);
    //SetRawDataWindowState(RAW_ID_DATA_RX, WF_RAW_UNMOUNTED);
}

/*
 * Set the index within the specified RAW window. If attempt to set RAW index
 * outside boundaries of RAW window (past the end) this function will time out.
 * It's legal to set the index past the end of the raw window so long as there
 * is no attempt to read or write at that index.  For now, flag an event.
 *
 * Parameters:
 *  raw_id -- RAW window ID
 *  offset -- desired index within RAW window
 */
void mrf_raw_seek(unsigned raw_id, unsigned offset)
{
    unsigned status, start_time;

    /* set the index register associated with the raw ID */
    mrf_write(raw_index_reg[raw_id], offset);

    /* Poll the raw status register to determine that:
     *  1) raw set index completed successfully  OR
     *  2) raw set index failed, implying that the raw index was set past the end of the raw window
     */
    start_time = mrf_timer_read();
    for (;;) {
        status = mrf_read(raw_status_reg[raw_id]);
        if (! (status & RAW_STATUS_BUSY))
            break;

        if (mrf_timer_elapsed(start_time) > 5) {
            // if we timed out that means that the caller is trying to set the index
            // past the end of the raw window.  Not illegal in of itself so long
            // as there is no attempt to read or write at this location.  But,
            // applications should avoid this to avoid the timeout in
            printf("--- %s: bad offset=%u out of bounds\n", __func__, offset);
            break;
        }
        udelay(10);
    }
}

/*
 * Read bytes from the specified raw window.
 * Parameters:
 *  raw_id - RAW ID
 *  buffer - buffer to read bytes into
 *  length - number of bytes to read
 */
void mrf_raw_read(unsigned raw_id, u_int8_t *buffer, unsigned length)
{
    mrf_read_array(raw_data_reg[raw_id], buffer, length);
}

/*
 * Write bytes to RAW window.
 * Parameters:
 *  raw_id - RAW ID
 *  buffer - buffer containing bytes to write
 *  length - number of bytes to read
 */
void mrf_raw_write(unsigned raw_id, const u_int8_t *buffer, unsigned length)
{
    mrf_write_array(raw_data_reg[raw_id], buffer, length);
}

/*
 * Read the specified number of bytes from a mounted RAW window
 * from the specified starting offset.
 *
 * Parameters:
 *  raw_id -- RAW window ID being read from
 *  offset -- start index within RAW window to read from
 *  length -- number of bytes to read from the RAW window
 *  dest   -- pointer to host buffer where read data is copied
 */
void mrf_raw_pread(unsigned raw_id, u_int8_t *dest, unsigned length, unsigned offset)
{
    mrf_raw_seek(raw_id, offset);
    mrf_raw_read(raw_id, dest, length);
}

/*
 * Write the specified number of bytes to a mounted RAW window
 * at the specified starting offset.
 *
 * Parameters:
 *  raw_id -- RAW window ID being written to
 *  offset -- start index within RAW window to write to
 *  length -- number of bytes to write to RAW window
 *  src    -- pointer to host buffer write data
 */
void mrf_raw_pwrite(unsigned raw_id, const u_int8_t *src, unsigned length, unsigned offset)
{
    mrf_raw_seek(raw_id, offset);
    mrf_raw_write(raw_id, src, length);
}
