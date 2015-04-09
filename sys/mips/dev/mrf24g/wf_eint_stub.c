/*
 * MRF24WG External Interrupt Stub Functions
 *
 * Summary: Functions to control MRF24WG External Interrupt.
 */
#include "board_profile.h"
#include "wf_universal_driver.h"

/*
 * Configure host processor external interrupt.
 * This line is asserted low by MRF24WG.
 * Called by Universal Driver during initialization.
 */
void WF_EintInit()
{
    WIFI_IECxCLR = WIFI_INT_MASK;       // disable INT1 interrupt

    // configure INT1 pin
    WF_INT_SET(1);                      // set the level of I/O pin high to deassert interrupt (no pullup on board)
    WF_INT_INPUT(1);                    // configure pin as an input
    WIFI_INT_EDGE(0);                   // falling edge triggered

    // ensure the interrupt is cleared
    WIFI_IFSxCLR = WIFI_INT_MASK;       // clear INT1IF bit (clears any pending interrupt)
#if 0
    // TODO: set interrupt priority and subpriority
    WIFI_IPC_IP(3);                     // set priority=3
    WIFI_IPC_IS(0);                     // sub priority=0
#endif
    // enable the interrupt
    WIFI_IECxSET = WIFI_INT_MASK;       // enable INT1
}

/*
 * Enable the MRF24WG external interrupt.
 * Called by Universal Driver during normal operations
 * to enable the MRF24WG external interrupt.
 *
 * When using level-triggered interrupts it is possible that host MCU
 * could miss a falling edge; this can occur because during normal
 * operations as the Universal Driver disables the external interrupt for
 * short periods.  If the MRF24WG asserts the interrupt line while the
 * interrupt is disabled the falling edge won't be detected.  So, a
 * check must be made to determine if an interrupt is pending; if so, the
 * interrupt must be forced.
 * This is not an issue for level-triggered interrupts.
 */
void WF_EintEnable()
{
    // PIC32 uses level-triggered interrupts, so it is possible the Universal Driver
    // may have temporarily disabled the external interrupt, and then missed the
    // falling edge when the MRF24WG asserted the interrupt line.  The code below
    // checks for this condition and forces the interrupt if needed.

    // if interrupt line is low, then PIC32 may have missed the falling edge
    // while the interrupt was disabled.
    if ( WF_INT_VAL == 0 )
    {
        // Need to force the interrupt for two reasons:
        //   1) there is an event that needs to be serviced
        //   2) MRF24WG won't generate another falling edge until the interrupt
        //      is processed.
        WIFI_IFSxSET = WIFI_INT_MASK; // this will force the INT1 interrupt as soon as we
                             // we enabled it below.
    }

    /* enable the external interrupt */
    WIFI_IECxSET = WIFI_INT_MASK;
}

/*
 * Disable the MRF24WG external interrupt/
 *
 * Called by Universal Driver during normal operations
 * to disable the MRF24WG external interrupt.
 */
void WF_EintDisable()
{
    WIFI_IECxCLR = WIFI_INT_MASK;
}

/*
 * Determines if the external interrupt is disabled.
 * Called by Universal Driver during normal operations to check if the current
 * state of the external interrupt is disabled.
 * Returns True if interrupt is disabled, else False.
 */
bool WF_isEintDisabled()
{
    return (WIFI_INT_IE == 0);
}

/*
 * Determine if the external interrupt is pending.
 * Called by Universal Driver during normal operations
 * to check if an external interrupt is pending.
 * Returns True if interrupt is pending, else False.
 */
bool WF_isEintPending()
{
    // if interrupt line is low, but interrupt request has not occurred
    return(((WF_INT_VAL == 0) && (WIFI_INT_IF == 0)));
}

/*
 * MRF24WG external interrupt handler.
 *
 * This interrupt handler should:
 * 1) ensure the interrupt is disabled upon exit (Universal Driver will reenable it)
 * 2) clear the interrupt
 * 3) call WFEintHandler()
 */
void _WFInterrupt()             // TODO: assign to PIC32_IRQ_INT4 vector
{
    // clear EINT
    WIFI_IFSxCLR = WIFI_INT_MASK;        // clear the interrupt
    WIFI_IECxCLR = WIFI_INT_MASK;        // disable external interrupt
    WF_EintHandler();           // call Univeral Driver handler function
}
