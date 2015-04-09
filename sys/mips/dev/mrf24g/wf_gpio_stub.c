/*
 * MRF24WG GPIO Stub Functions
 *
 * Functions to control MRF24WG RESET and HIBERNATE pins needed by the
 * Universal Driver.  Functions in this module should not be called
 * directly by the application code.
 */
#include "board_profile.h"
#include "wf_universal_driver.h"

/*
 * Configures GPIO pins used for RESET and HIBERNATE as outputs
 *
 * Called by Universal Driver during initialization.
 *
 * This function should configure port pins used to control MRF24WG RESET and
 * HIBERNATE pins as outputs.  Always set the level first, then set the port as
 * an output to avoid a glitch.  This function initially sets the HIBERNATE
 * pin high (in hibernate mode) and sets the RESET line low (in reset).
 */
void WF_GpioInit()
{
    // Set level on HIBERNATE I/O to high (MRF24WG in hibernate) and then
    // configure as an output.
    WF_HIBERNATE_SET(1);        // drive pin high
    WF_HIBERNATE_INPUT(0);      // configure as output

    // Set level on RESET I/O to low (MRF24WG in reset) and then
    // configure as an output.
    WF_RESET_SET(0);            // drive pin low
    WF_RESET_INPUT(0);          // configure as output
}

/*
 * Put the MRF24WG into and out of reset.
 *
 * Called by Universal Driver during initialization
 * to take the MRF24WG out of reset.
 * Sets the RESET line high or low.
 *
 * Parameter: level -- WF_HIGH or WF_LOW
 */
void WF_GpioSetReset(uint8_t level)
{
    WF_RESET_SET(level);
}

/*
 * Put the MRF24WG into and out of hibernate.
 *
 * Called by Universal Driver during initialization
 * to take the MRF24WG out of reset.
 * Sets the HIBERNATE line high or low.
 *
 * Parameter: level -- WF_HIGH or WF_LOW
 */
void WF_GpioSetHibernate(uint8_t level)
{
    WF_HIBERNATE_SET(level);
}
