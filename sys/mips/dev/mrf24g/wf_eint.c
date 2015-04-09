/*
 * MRF24WG External Interrupt
 *
 * Functions pertaining MRF24WG external interrupt
 * MRF24WG interrupts the host CPU to signal data Rx messages, events,
 * and management response messages.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

static uint8_t g_HostIntSaved = 0;

/*
 * true if external interrupt needs processing, else false
 */
static volatile bool g_ExIntNeedsServicing;

/*
 * Keep these as static globals instead of local variables in the Eint Handler.
 * If declared as local variables, causes stack corruption in PIC18, or other
 * MCU's with overlay memory.
 */
static uint8_t  g_EintHostIntRegValue;
static uint8_t  g_EintHostIntMaskRegValue;
static uint8_t  g_EintHostInt;

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
    /*--------------------------------------------------------*/
    /* if driver is waiting for a RAW Move Complete interrupt */
    /*--------------------------------------------------------*/
    if (isWaitingForRawMoveCompleteInterrupt())
    {
        // read hostInt register and hostIntMask register to determine cause of interrupt
        // TODO: [NOTE: Stellaris requires two reads to get proper SPI read?]
        g_EintHostIntRegValue      = Read8BitWFRegister(WF_HOST_INTR_REG);
        g_EintHostIntRegValue      = Read8BitWFRegister(WF_HOST_INTR_REG);

        g_EintHostIntMaskRegValue  = Read8BitWFRegister(WF_HOST_MASK_REG);

        // AND the two registers together to determine which active, enabled interrupt has occurred
        g_EintHostInt = g_EintHostIntRegValue & g_EintHostIntMaskRegValue;


        // If a Raw move complete interrupt occurred
        if (g_EintHostInt & (WF_HOST_INT_MASK_RAW_0_INT_0 | WF_HOST_INT_MASK_RAW_1_INT_0 | WF_HOST_INT_MASK_INT2))
        {
            // Let the Raw driver know which interrupt occurred and clear the flag
            SignalRawInterruptEvent(g_EintHostInt);
            ClearWaitingForRawMoveCompleteInterrupt();

            /* if no other interrupts occurred other than a RAW0/RAW1/RAW2/RAW3/RAW4 Raw Move Complete */
            if ((g_EintHostInt & ~(WF_HOST_INT_MASK_RAW_0_INT_0 |
                                   WF_HOST_INT_MASK_RAW_1_INT_0 |
                                   WF_HOST_INT_MASK_INT2)) == 0)
            {
                /* clear the RAW interrupts, re-enable interrupts, and exit */
                Write8BitWFRegister(WF_HOST_INTR_REG, (WF_HOST_INT_MASK_RAW_0_INT_0 |
                                                       WF_HOST_INT_MASK_RAW_1_INT_0 |
                                                       WF_HOST_INT_MASK_INT2));

                Write16BitWFRegister(WF_HOST_INTR2_REG, (WF_HOST_INT_MASK_RAW_2_INT_0 |
                                                         WF_HOST_INT_MASK_RAW_3_INT_0 |
                                                         WF_HOST_INT_MASK_RAW_4_INT_0 |
                                                         WF_HOST_INT_MASK_RAW_5_INT_0));

                WF_EintEnable();
                return;
            }
            /* else we got a RAW0/RAW1/RAW2/RAW3/RAW4/RAW5 Raw Move Complete interrupt, but, there is also at */
            /* least one other interrupt present                                                               */
            else
            {
                // save the other interrupts and clear them, along with the Raw Move Complete interrupts
                // keep interrupts disabled
                Write16BitWFRegister(WF_HOST_INTR2_REG, (WF_HOST_INT_MASK_RAW_2_INT_0   |
                                                         WF_HOST_INT_MASK_RAW_3_INT_0   |
                                                         WF_HOST_INT_MASK_RAW_4_INT_0   |
                                                         WF_HOST_INT_MASK_RAW_5_INT_0));
                g_HostIntSaved |= (g_EintHostInt & ~(WF_HOST_INT_MASK_RAW_0_INT_0 | WF_HOST_INT_MASK_RAW_1_INT_0 | WF_HOST_INT_MASK_INT2));
                Write8BitWFRegister(WF_HOST_INTR_REG, g_EintHostInt);

                // leave interrupt disabled for now
            }
        }
        /*--------------------------------------------------------------------------------------------------*/
        /* else we did not get a 'RAW Move Complete' interrupt, but we did get at least one other interrupt */
        /*--------------------------------------------------------------------------------------------------*/
        else
        {
            g_HostIntSaved |= g_EintHostInt;
            Write8BitWFRegister(WF_HOST_INTR_REG, g_EintHostInt);
            WF_EintEnable();
        }
    }

    // used by InterruptCheck()
    g_ExIntNeedsServicing = true;
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
    uint8_t  hostIntRegValue;
    uint8_t  hostIntMaskRegValue;
    uint8_t  hostInt;
    uint16_t hostInt2;
    uint32_t assertInfo;

    // in no interrupt to process
    if (!g_ExIntNeedsServicing)
    {
        return;
    }

    g_ExIntNeedsServicing = false;

     /* read hostInt register to determine cause of interrupt */
    hostIntRegValue = Read8BitWFRegister(WF_HOST_INTR_REG);

    // OR in the saved interrupts during the time when we were waiting for raw complete, set by WFEintHandler()
    hostIntRegValue |= g_HostIntSaved;

    // done with the saved interrupts, clear variable
    g_HostIntSaved = 0;

    hostIntMaskRegValue  = Read8BitWFRegister(WF_HOST_MASK_REG);

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
        hostInt2 = Read16BitWFRegister(WF_HOST_INTR2_REG);
        if (hostInt2 & WF_HOST_INT_MASK_MAIL_BOX_0_WRT)
        {
            // module number in upper 8 bits, assert information in lower 20 bits
            assertInfo = (((uint32_t)Read16BitWFRegister(WF_HOST_MAIL_BOX_0_MSW_REG)) << 16) |
                                     Read16BitWFRegister(WF_HOST_MAIL_BOX_0_LSW_REG);
            // signal this event
            EventEnqueue(WF_EVENT_MRF24WG_MODULE_ASSERT, assertInfo);
        }

        /* clear this interrupt */
        Write16BitWFRegister(WF_HOST_INTR2_REG, WF_HOST_INT_MASK_INT2);
    }
    /* else if got a FIFO 1 Threshold interrupt (Management Fifo).  Mgmt Rx msg ready to proces. */
    else if ((hostInt & WF_HOST_INT_MASK_FIFO_1_THRESHOLD) == WF_HOST_INT_MASK_FIFO_1_THRESHOLD)
    {
        /* clear this interrupt */
        Write8BitWFRegister(WF_HOST_INTR_REG, WF_HOST_INT_MASK_FIFO_1_THRESHOLD);

        // signal that a mgmt msg, either confirm or indicate, has been received
        // and needs to be processed
        SignalMgmtMsgRx();
    }
    /* else if got a FIFO 0 Threshold Interrupt (Data Fifo).  Data Rx msg ready to process. */
    else if ((hostInt & WF_HOST_INT_MASK_FIFO_0_THRESHOLD) == WF_HOST_INT_MASK_FIFO_0_THRESHOLD)
    {
        /* clear this interrupt */
        Write8BitWFRegister(WF_HOST_INTR_REG, WF_HOST_INT_MASK_FIFO_0_THRESHOLD);

        // signal that a data msg has been received and needs to be processed
        SignalPacketRx();
    }
    /* else got a Host interrupt that we don't handle */
    else if (hostInt) {
        /* clear this interrupt */
        Write8BitWFRegister(WF_HOST_INTR_REG, hostInt);
        WF_EintEnable();
    }
    /* we got a spurious interrupt (no bits set in register) */
    else {
        /* spurious interrupt */
        WF_EintEnable();
    }
}
