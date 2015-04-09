/*
 * MRF24WG RAW (Random Access Window)
 *
 * Functions to control RAW windows.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

#define WF_RAW_STATUS_REG_BUSY_MASK     ((uint16_t)(0x0001))

/*
 * These macros set a flag bit if the raw index is set past the end
 * of the raw window, or clear the flag bit if the raw index is set
 * within the raw window.
 */
#define SetIndexOutOfBoundsFlag(rawId)      g_RawIndexPastEnd |= g_RawAccessOutOfBoundsMask[rawId]
#define ClearIndexOutOfBoundsFlag(rawId)    g_RawIndexPastEnd &= ~g_RawAccessOutOfBoundsMask[rawId]
#define isIndexOutOfBounds(rawId)           ((g_RawIndexPastEnd & g_RawAccessOutOfBoundsMask[rawId]) > 0)

typedef struct {
    volatile uint8_t rawInterruptMask;                    // filled in by interrupt
    bool             waitingForRawMoveCompleteInterrupt;  // set in this module, cleared by interrupt
} t_rawMoveState;

/* raw registers for each raw window being used */
static const uint8_t  g_RawIndexReg[NUM_RAW_WINDOWS]  = {
    RAW_0_INDEX_REG,  RAW_1_INDEX_REG,  RAW_2_INDEX_REG,  RAW_3_INDEX_REG,  RAW_4_INDEX_REG, RAW_5_INDEX_REG
};
static const uint8_t  g_RawStatusReg[NUM_RAW_WINDOWS] = {
    RAW_0_STATUS_REG, RAW_1_STATUS_REG, RAW_2_STATUS_REG, RAW_3_STATUS_REG, RAW_4_STATUS_REG, RAW_5_STATUS_REG
};
static const uint16_t g_RawCtrl0Reg[NUM_RAW_WINDOWS]  = {
    RAW_0_CTRL_0_REG, RAW_1_CTRL_0_REG, RAW_2_CTRL_0_REG, RAW_3_CTRL_0_REG, RAW_4_CTRL_0_REG, RAW_5_CTRL_0_REG
};
static const uint16_t g_RawCtrl1Reg[NUM_RAW_WINDOWS]  = {
    RAW_0_CTRL_1_REG, RAW_1_CTRL_1_REG, RAW_2_CTRL_1_REG, RAW_3_CTRL_1_REG, RAW_4_CTRL_1_REG, RAW_5_CTRL_1_REG
};
static const uint16_t g_RawDataReg[NUM_RAW_WINDOWS]   = {
    RAW_0_DATA_REG,   RAW_1_DATA_REG,   RAW_2_DATA_REG,   RAW_3_DATA_REG,   RAW_4_DATA_REG, RAW_5_DATA_REG
};

/*
 * Interrupt mask for each raw window; note that raw0 and raw1 are really
 * 8 bit values and will be cast when used.
 */
static const uint16_t g_RawIntMask[NUM_RAW_WINDOWS] = {
    WF_HOST_INT_MASK_RAW_0_INT_0,   /* used in HOST_INTR reg (8-bit register) */
    WF_HOST_INT_MASK_RAW_1_INT_0,   /* used in HOST_INTR reg (8-bit register) */
    WF_HOST_INT_MASK_RAW_2_INT_0,   /* used in HOST_INTR2 reg (16-bit register) */
    WF_HOST_INT_MASK_RAW_3_INT_0,   /* used in HOST_INTR2 reg (16-bit register) */
    WF_HOST_INT_MASK_RAW_4_INT_0,   /* used in HOST_INTR2 reg (16-bit register) */
    WF_HOST_INT_MASK_RAW_5_INT_0,   /* used in HOST_INTR2 reg (16-bit register) */
};

/* keeps track of whether raw tx/rx data windows mounted or not */
static uint8_t RawWindowState[2];   // [0] is RAW Rx window, [1] is RAW Tx window

const uint8_t g_RawAccessOutOfBoundsMask[NUM_RAW_WINDOWS] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20
};
static uint8_t g_RawIndexPastEnd = 0;   /* no indexes are past end of window */

static t_rawMoveState RawMoveState;

/*
 * Wait for a RAW move to complete.
 * Returns a number of bytes that were overlayed (not always applicable).
 */
static uint16_t WaitForRawMoveComplete(uint8_t rawId)
{
    uint8_t  rawIntMask;
    uint16_t byteCount;
    uint8_t  regId;
    uint32_t elapsedTime;
    uint32_t startTime;

    /* create mask to check against for Raw Move complete interrupt for either RAW0 or RAW1 */
    if (rawId <= RAW_ID_1) {
        /* will be either raw 0 or raw 1 */
        rawIntMask = (rawId == RAW_ID_0)?WF_HOST_INT_MASK_RAW_0_INT_0:WF_HOST_INT_MASK_RAW_1_INT_0;
    } else {
        /* will be INTR2 bit in host register, signifying RAW2, RAW3, or RAW4 */
        rawIntMask = WF_HOST_INT_MASK_INT2;
    }

    startTime = WF_TimerRead();
    for (;;) {
        //InterruptCheck();

        /* If received an external interrupt that signaled the RAW Move
         * completed then break out of this loop. */
        if (RawMoveState.rawInterruptMask & rawIntMask) {
            break;
        }

        elapsedTime = GetElapsedTime(startTime, WF_TimerRead());
        if (elapsedTime > 20) {
            EventEnqueue(WF_EVENT_ERROR, UD_ERROR_RAW_INTERRUPT_TIMEOUT);
            break;
        }
    }

    /* read the byte count and return it */
    regId = g_RawCtrl1Reg[rawId];
    byteCount = Read16BitWFRegister(regId);

    return byteCount;
}

/*
 * Initialize RAW (Random Access Window) on MRF24WG
 */
void RawInit()
{
    // Used in interrupt routine and functions in this module.  The reason for
    // this mechanism is because when waiting for a Raw Move complete interrupt
    // we need to save the state if any other interrupts occur at the same time so
    // we don't lose them
    RawMoveState.rawInterruptMask  = 0;                         // interrupt will write to this
    RawMoveState.waitingForRawMoveCompleteInterrupt = false;    // not waiting for RAW move complete

    // By default the MRF24WG firmware mounts Scratch to RAW 1 after reset. This
    // is not being used, so unmount the scratch from this RAW window.
    ScratchUnmount(RAW_ID_1);

    /* Permanently mount scratch memory, index defaults to 0. */
    /* If one needs to know, this function returns the number of bytes in scratch memory */
    ScratchMount(RAW_SCRATCH_ID);

    SetRawDataWindowState(RAW_DATA_TX_ID, WF_RAW_UNMOUNTED);
    SetRawDataWindowState(RAW_DATA_RX_ID, WF_RAW_UNMOUNTED);
}

bool isWaitingForRawMoveCompleteInterrupt()
{
    return RawMoveState.waitingForRawMoveCompleteInterrupt;
}

void ClearWaitingForRawMoveCompleteInterrupt()
{
    RawMoveState.waitingForRawMoveCompleteInterrupt = false;
}

void SignalRawInterruptEvent(uint8_t rawIntMask)
{
    RawMoveState.rawInterruptMask = rawIntMask;
}

/*
 * Mounts RAW scratch window.
 * Returns size, in bytes, of Scratch buffer.
 *
 * The scratch window is not dynamically allocated, but references a static
 * portion of the WiFi device RAM. Thus, the Scratch data is not lost when
 * the scratch window is unmounted.
 *
 * Parameters:
 *  rawId -- RAW window ID being used to mount the scratch data
 */
uint16_t ScratchMount(uint8_t rawId)
{
    uint16_t byteCount;

    byteCount = RawMove(rawId, RAW_SCRATCH_POOL, true, 0);
    return byteCount;
}

/*
 * Unmount RAW scratch window.
 * Returns size, in bytes, of Scratch buffer.
 *
 * The scratch window is not dynamically allocated, but references a static
 * portion of the WiFi device RAM. Thus, the Scratch data is not lost when
 * the scratch window is unmounted.
 *
 * Parameters:
 *  rawId -- RAW window ID that was used to mount the scratch window
 */
void ScratchUnmount(uint8_t rawId)
{
    RawMove(rawId, RAW_SCRATCH_POOL, false, 0);
}

/*
 * Allocate a Mgmt Tx buffer.
 * Returns True if mgmt tx buffer successfully allocated, else False.
 *
 * Determines if WiFi chip has enough memory to allocate a tx mgmt buffer, and,
 * if so, allocates it.
 *
 * Parameters:
 *  bytesNeeded -- number of bytes needed for the mgmt tx message
 */
bool AllocateMgmtTxBuffer(uint16_t bytesNeeded)
{
    uint16_t bufAvail;
    uint16_t byteCount;

    /* get total bytes available for MGMT tx memory pool */
    bufAvail = Read16BitWFRegister(WF_HOST_WFIFO_BCNT1_REG) & 0x0fff; /* LS 12 bits contain length */

    /* if enough bytes available to allocate */
    if (bufAvail >= bytesNeeded) {
        /* allocate and create the new Mgmt Tx buffer */
        byteCount = RawMove(RAW_MGMT_TX_ID, RAW_MGMT_POOL, true, bytesNeeded);
        if (byteCount == 0) {
             EventEnqueue(WF_EVENT_ERROR, UD_ERROR_MGMT_BUFFER_ALLOCATION_FAILED);
             return false;
        }
        ClearIndexOutOfBoundsFlag(RAW_MGMT_TX_ID);
        return true;
    }
    /* else not enough bytes available at this time to satisfy request */
    else {
        /* if we allocated some bytes, but not enough, then deallocate what was allocated */
        if (bufAvail > 0) {
            RawMove(RAW_MGMT_RX_ID, RAW_MGMT_POOL, false, 0);
        }
        return false;
    }
}

/*
 * Deallocates a mgmt Rx buffer
 * Called by WiFi driver when its finished processing a Rx mgmt message.
 */
void DeallocateMgmtRxBuffer()
{
    /* Unmount (release) mgmt packet now that we are done with it */
    RawMove(RAW_MGMT_RX_ID, RAW_MGMT_POOL, false, 0);
}

/*
 * Write bytes to RAW window.
 * Parameters:
 *  rawId   - RAW ID
 *  pBuffer - Buffer containing bytes to write
 *  length  - number of bytes to read
 */
void RawSetByte(uint16_t rawId, const uint8_t *p_buffer, uint16_t length)
{
    uint8_t regId;

    // if trying to write past end of raw window
    if (isIndexOutOfBounds(rawId)) {
        EventEnqueue(WF_EVENT_ERROR, UD_ERROR_RAW_SET_BYTE_OUT_OF_BOUNDS);
    }

    /* write data to raw window */
    regId = g_RawDataReg[rawId];
    WriteWFArray(regId, p_buffer, length);
}

/*
 * Read bytes from the specified raw window.
 * Returns error code.
 *
 * Parameters:
 *  rawId   - RAW ID
 *  pBuffer - Buffer to read bytes into
 *  length  - number of bytes to read
 */
void RawGetByte(uint16_t rawId, uint8_t *pBuffer, uint16_t length)
{
    uint8_t regId;

    // if the raw index was previously set out of bounds
    if (isIndexOutOfBounds(rawId)) {
        // trying to read past end of raw window
        EventEnqueue(WF_EVENT_ERROR, UD_ERROR_RAW_GET_BYTE_OUT_OF_BOUNDS);
    }

    regId = g_RawDataReg[rawId];
    ReadWFArray(regId, pBuffer, length);
}

/*
 * Sends a management frame to the WiFi chip.
 *
 * The management header, which consists of a type and subtype, have already
 * been written to the frame before this function is called.
 *
 * Parameters:
 *  bufLen -- number of bytes that comprise the management frame.
 */
void SendRAWManagementFrame(uint16_t bufLen)
{
    /* Notify WiFi device that management message is ready to be processed */
    RawMove(RAW_MGMT_TX_ID, RAW_MAC, false, bufLen);
}

/*
 * Mounts most recent Rx message.
 * Returns length, a number of bytes in the received message.
 *
 * This function mounts the most recent Rx message from the WiFi chip, which
 * could be either a management or a data message.
 *
 * Parameters:
 *  rawId -- RAW ID specifying which raw window to mount the rx packet in.
 */
uint16_t RawMountRxBuffer(uint8_t rawId)
{
    uint16_t length;

    length = RawMove(rawId, RAW_MAC, true, 0);

    // the length should never be 0 if notified of an Rx msg
    if (length == 0) {
        EventEnqueue(WF_EVENT_ERROR, UD_ERROR_RAW_RX_MOUNT_FAILED);
    }

    /* if mounting a Raw Rx data frame */
    if (rawId == RAW_DATA_RX_ID) {
        /* notify WiFi driver that an Rx data frame is mounted */
        SetRawDataWindowState(RAW_DATA_RX_ID, WF_RAW_DATA_MOUNTED);
    }
    return length;
}

/*
 * Read the specified number of bytes from a mounted RAW window
 * from the specified starting index.
 * Returns error code.
 *
 * Parameters:
 *  rawId      -- RAW window ID being read from
 *  startIndex -- start index within RAW window to read from
 *  length     -- number of bytes to read from the RAW window
 *  p_dest     -- pointer to Host buffer where read data is copied
 */
void RawRead(uint8_t rawId, uint16_t startIndex, uint16_t length, uint8_t *p_dest)
{
    RawSetIndex(rawId, startIndex);
    RawGetByte(rawId, p_dest, length);
}

/*
 * Write the specified number of bytes to a mounted RAW window
 * at the specified starting index.
 *
 * Parameters:
 *  rawId      -- RAW window ID being written to
 *  startIndex -- start index within RAW window to write to
 *  length     -- number of bytes to write to RAW window
 *  p_src      -- pointer to Host buffer write data
 */
void RawWrite(uint8_t rawId, uint16_t startIndex, uint16_t length, const uint8_t *p_src)
{
    /* set raw index in dest memory */
    RawSetIndex(rawId, startIndex);

    /* write data to RAW window */
    RawSetByte(rawId, p_src, length);
}

/*
 * Sets the index within the specified RAW window.
 *
 * Sets the index within the specified RAW window. If attempt to set RAW index
 * outside boundaries of RAW window (past the end) this function will time out.
 * It's legal to set the index past the end of the raw window so long as there
 * is no attempt to read or write at that index.  For now, flag an event.
 *
 * Parameters:
 *  rawId -- RAW window ID
 *  index -- desired index within RAW window
 */
void RawSetIndex(uint16_t rawId, uint16_t index)
{
    uint8_t  regId;
    uint16_t regValue;
    uint32_t elapsedTime;
    uint32_t startTime;

    /* get the index register associated with the raw ID and write to it */
    regId = g_RawIndexReg[rawId];
    Write16BitWFRegister(regId, index);

    /* Get the raw status register address associated with the raw ID.  This will be polled to
     * determine that:
     *  1) raw set index completed successfully  OR
     *  2) raw set index failed, implying that the raw index was set past the end of the raw window
     */
    regId = g_RawStatusReg[rawId];

    /* read the status register until set index operation completes or times out */
    startTime = WF_TimerRead();
    while (1) {
        regValue = Read16BitWFRegister(regId);
        if ((regValue & WF_RAW_STATUS_REG_BUSY_MASK) == 0) {
            ClearIndexOutOfBoundsFlag(rawId);
            break;
        }

        elapsedTime = GetElapsedTime(startTime, WF_TimerRead());
        if (elapsedTime > 5) {
            // if we timed out that means that the caller is trying to set the index
            // past the end of the raw window.  Not illegal in of itself so long
            // as there is no attempt to read or write at this location.  But,
            // applications should avoid this to avoid the timeout in
            SetIndexOutOfBoundsFlag(rawId);
            EventEnqueue(WF_EVENT_ERROR, UD_ERROR_RAW_SET_INDEX_OUT_OF_BOUNDS);
            break;
        }
    }
}

/*
 * Allocate a Data Tx buffer for use by the TCP/IP stack.
 * Returns True if data tx buffer successfully allocated, else False.
 *
 * Determines if WiFi chip has enough memory to allocate a tx data buffer,
 * and, if so, allocates it.
 *
 * Parameters:
 *  bytesNeeded -- number of bytes needed for the data tx message
 */
bool AllocateDataTxBuffer(uint16_t bytesNeeded)
{
    uint16_t bufAvail;
    uint16_t byteCount;

    /* get total bytes available for DATA tx memory pool */
    bufAvail = Read16BitWFRegister(WF_HOST_WFIFO_BCNT0_REG) & 0x0fff; /* LS 12 bits contain length */
    if (bufAvail < bytesNeeded) {
        /* not enough bytes available at this time to satisfy request */
        return false;
    }

    /* allocate and create the new Tx buffer (mgmt or data) */
    byteCount = RawMove(RAW_DATA_TX_ID, RAW_DATA_POOL, true, bytesNeeded);
    if (byteCount == 0) {
        EventEnqueue(WF_EVENT_ERROR, UD_TX_ALLOCATION_FAILED);
        return false;
    }

    /* flag this raw window as mounted (in use) */
    SetRawDataWindowState(RAW_DATA_TX_ID, WF_RAW_DATA_MOUNTED);
    return true;
}

/*
 * Deallocate a Data Rx buffer.
 *
 * Typically called by MACGetHeader(), the assumption being that when
 * the stack is checking for a newly received data message it is finished
 * with the previously received data message.  Also called by MACGetHeader()
 * if the SNAP header is invalid and the packet is thrown away.
 */
void DeallocateDataRxBuffer()
{
    // TODO: verify data rx is mounted

    SetRawDataWindowState(RAW_DATA_RX_ID, WF_RAW_UNMOUNTED);

    /* perform deallocation of raw rx buffer */
    RawMove(RAW_DATA_RX_ID, RAW_DATA_POOL, false, 0);
}

/*
 * Perform RAW Move operation.
 * When applicable, return the number of bytes overlayed by the raw move.
 *
 * The function performs a variety of operations (e.g. allocating tx buffers,
 * mounting rx buffers, copying from one raw window to another, etc.)
 *
 * Parameters:
 *  rawId -- Raw ID 0 thru 5, except is srcDest is RAW_COPY, in which case rawId
 *           contains the source address in the upper 4 bits and destination
 *           address in lower 4 bits.
 *
 *  srcDest -- object that will either be the source or destination of the move:
 *                RAW_MAC
 *                RAW_MGMT_POOL
 *                RAW_DATA_POOL
 *                RAW_SCRATCH_POOL
 *                RAW_STACK_MEM
 *                RAW_COPY (this object not allowed, handled in RawToRawCopy())
 *
 *  rawIsDestination -- true is srcDest is the destination, false if srcDest is
 *                      the source of the move
 *
 *  size -- number of bytes to overlay (not always applicable)
 */
uint16_t RawMove(uint16_t rawId,
                 uint16_t srcDest,
                 bool     rawIsDestination,
                 uint16_t size)
{
    uint16_t byteCount;
    uint8_t  regId;
    uint8_t  regValue;
    uint16_t ctrlVal = 0;
    bool intDisabled;

    // save current state of External interrupt and disable it
    intDisabled = WF_isEintDisabled();
    WF_EintDisable();

    /*
     * These variables are shared with the ISR so need to be careful when
     * setting them.  The WF_EintHandler() is the isr that will touch these
     * variables but will only touch them if
     * RawMoveState.waitingForRawMoveCompleteInterrupt is set to TRUE.
     * RawMoveState.waitingForRawMoveCompleteInterrupt is only set TRUE here
     * and only here.  So as long as we set RawMoveState.rawInterrupt first
     * and then set RawMoveState.waitingForRawMoveCompleteInterrupt
     * to TRUE, we are guaranteed that the ISR won't touch
     * RawMoveState.rawInterrupt and
     * RawMoveState.waitingForRawMoveCompleteInterrupt.
     */
    RawMoveState.rawInterruptMask  = 0;
    RawMoveState.waitingForRawMoveCompleteInterrupt = true;

    /* Create control value that will be written to raw control register,
     * which initiates the raw move */
    if (rawIsDestination) {
        ctrlVal |= 0x8000;
    }
    /* fix later, simply need to ensure that size is 12 bits are less */
    ctrlVal |= (srcDest << 8);              /* defines are already shifted by 4 bits */
    ctrlVal |= ((size >> 8) & 0x0f) << 8;   /* MS 4 bits of size (bits 11:8) */
    ctrlVal |= (size & 0x00ff);             /* LS 8 bits of size (bits 7:0) */

    /*
     * This next 'if' block is used to ensure the expected raw interrupt
     * signifying raw move complete is cleared.
     */

    /* if doing a raw move on Raw 0 or 1 (data rx or data tx) */
    if (rawId <= RAW_ID_1) {
        /* Clear the interrupt bit in the host interrupt register (Raw 0 and 1 are in 8-bit host intr reg) */
        regValue = (uint8_t)g_RawIntMask[rawId];
        Write8BitWFRegister(WF_HOST_INTR_REG, regValue);
    }
    /* else doing raw move on mgmt rx, mgmt tx, or scratch */
    else {
        /* Clear the interrupt bit in the host interrupt 2 register (Raw 2,3, and 4 are in 16-bit host intr2 reg */
        regValue = g_RawIntMask[rawId];
        Write16BitWFRegister(WF_HOST_INTR2_REG, regValue);
    }

    /*
     * Now that the expected raw move complete interrupt has been cleared
     * and we are ready to receive it, initiate the raw move operation by
     * writing to the appropriate RawCtrl0.
     */
    regId = g_RawCtrl0Reg[rawId];                   /* get RawCtrl0 register address for desired raw ID */
    Write16BitWFRegister(regId, ctrlVal);           /* write ctrl value to register */

    // enable interrupts so we get raw move complete interrupt
    WF_EintEnable();
    byteCount = WaitForRawMoveComplete(rawId);      /* wait for raw move to complete */

    // if interrupts were disabled coming into this function, put back to that state
    if (intDisabled) {
        WF_EintDisable();
    }

    /* byte count is not valid for all raw move operations */
    return byteCount;
}

/* sets and gets the state of RAW data tx/rx windows */
void SetRawDataWindowState(uint8_t rawId, uint8_t state)
{
    RawWindowState[rawId] = state;
}

uint8_t GetRawDataWindowState(uint8_t rawId)
{
    return RawWindowState[rawId];
}
