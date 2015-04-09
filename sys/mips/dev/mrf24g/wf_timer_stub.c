/*
 * MRF24WG Universal Driver Timer Stub Functions
 *
 * This module contains Timer functions needed by the Universal Driver.
 */
#include "wf_universal_driver.h"

/*
 * Configures a timer that increments a 32-bit counter every 1ms.
 * Called by Universal Driver during initialization to start a 1ms timer that
 * will be used for various reasons.
 */
void WF_TimerInit()
{
}

/*
 * Called by the Universal Driver for various timing operations.
 * Returns the current value of the 1ms timer.
 * Returns current value of 1ms timer.
 */
uint32_t WF_TimerRead()
{
    //TODO
    return 0;   //SYSGetMilliSecond();
}
