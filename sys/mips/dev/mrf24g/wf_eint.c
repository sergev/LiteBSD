/*
 * MRF24WG External Interrupt
 *
 * Functions pertaining MRF24WG external interrupt
 * MRF24WG interrupts the host CPU to signal data Rx messages, events,
 * and management response messages.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * true if external interrupt needs processing, else false
 */
static volatile bool g_ExIntNeedsServicing;

/*
 * MRF24WG interrupt handler, called directly from the the interrupt routine,
 * _WFInterrupt() in wf_eint_stub.c.
 *
 * Must be called, once, for each MRF24WG interrupt.
 *
 * IMPORTANT: This function, and functions called from this function must not
 * use local variables, especially if the MCU uses overlay memory
 * like the Microchip PIC18 MCU.  The logical stack is contained
 * within the overlay memory, and if the interrupt uses local variables
 * the toolchain cannot be relied on to not overwrite local variables
 * in non-interrupt functions, specifically the function that was
 * interrupted by this interrupt.
 */
void WF_EintHandler()
{
    // used by InterruptCheck()
    g_ExIntNeedsServicing = 1;
}

/*
 * Periodically called to check if the MRF24WG external interrupt occurred, and
 * completes the interrupt processing.
 *
 * Some processing takes place in the actual interrupt routine, and some processing
 * takes place later in the round robin.  This function checks if an interrupt
 * has occurred, and if so, performs the rest of the interrupt processing.
 */
void InterruptCheck()
{
    u_int8_t  hostIntRegValue;
    u_int8_t  hostIntMaskRegValue;
    u_int8_t  hostInt;
    u_int16_t hostInt2;
    u_int32_t assertInfo;

    // in no interrupt to process
    if (! g_ExIntNeedsServicing) {
        return;
    }
    g_ExIntNeedsServicing = 0;

     /* read hostInt register to determine cause of interrupt */
    hostIntRegValue = mrf_read_byte(WF_HOST_INTR_REG);

    hostIntMaskRegValue = mrf_read_byte(WF_HOST_MASK_REG);

    // AND the two registers together to determine which active, enabled interrupt has occurred
    hostInt = hostIntRegValue & hostIntMaskRegValue;

    // if received a level 2 interrupt
    if ((hostInt & WF_HOST_INT_MASK_INT2) == WF_HOST_INT_MASK_INT2)
    {
        // Either a mgmt tx or mgmt rx Raw move complete occurred, which is how
        // this interrupt is normally used.  If this is the case, the event was
        // already handled in WF_EintHandler(), and all we need to do here is
        // clear the interrupt.  However, there is one other event to check for;
        // this interrupt is also used by the MRF24WG to signal that it has
        // hit an assert condition.  So, we check for that here.

        // if the MRF24WG has hit an assert condition
        hostInt2 = mrf_read(WF_HOST_INTR2_REG);
        if (hostInt2 & WF_HOST_INT2_MASK_MAIL_BOX) {
            // module number in upper 8 bits, assert information in lower 20 bits
            assertInfo = (mrf_read(WF_HOST_MAIL_BOX_0_MSW_REG) << 16) |
                          mrf_read(WF_HOST_MAIL_BOX_0_LSW_REG);
            printf("--- %s: assert info=%04x\n", __func__, assertInfo);
        }

        /* clear this interrupt */
        mrf_write(WF_HOST_INTR2_REG, WF_HOST_INT_MASK_INT2);
    }
    /* else if got a FIFO 1 Threshold interrupt (Management Fifo).  Mgmt Rx msg ready to proces. */
    else if (hostInt & WF_HOST_INT_MASK_FIFO_1)
    {
        /* clear this interrupt */
        mrf_write_byte(WF_HOST_INTR_REG, WF_HOST_INT_MASK_FIFO_1);

        // signal that a mgmt msg, either confirm or indicate, has been received
        // and needs to be processed
        SignalMgmtMsgRx();

        mrf_intr_enable();
    }
    /* else if got a FIFO 0 Threshold Interrupt (Data Fifo).  Data Rx msg ready to process. */
    else if (hostInt & WF_HOST_INT_MASK_FIFO_0)
    {
        /* clear this interrupt */
        mrf_write_byte(WF_HOST_INTR_REG, WF_HOST_INT_MASK_FIFO_0);

        // signal that a data msg has been received and needs to be processed
        SignalPacketRx();
    }
    /* else got a Host interrupt that we don't handle */
    else if (hostInt) {
        /* clear this interrupt */
        mrf_write_byte(WF_HOST_INTR_REG, hostInt);
        mrf_intr_enable();
    }
    /* we got a spurious interrupt (no bits set in register) */
    else {
        /* spurious interrupt */
        mrf_intr_enable();
    }
}
