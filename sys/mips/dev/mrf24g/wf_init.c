/*
 * MRF24WG Initialization
 *
 * Functions pertaining Universal Driver and MRF24WG initialization.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include "wf_universal_driver.h"
#include "wf_ud_state.h"

/*
 * Initialize the MRF24WG for operations.
 * Must be called before any other WiFi API calls.
 * Returns the device ROM and patch version.
 */
unsigned mrf_setup()
{
    unsigned msec, value, mask, mask2, rom_version;

    UdStateInit();          // initialize internal state machine
    UdSetInitInvalid();     // init not valid until it gets through this state machine

    /*
     * Reset the chip.
     */

    /* Clear the power bit to disable low power mode. */
    mrf_write(MRF24_REG_PSPOLL, 0);

    /* Set HOST_RESET bit in register to put device in reset. */
    mrf_write(MRF24_REG_RESET, mrf_read(MRF24_REG_RESET) | RESET_SOFT_RESET);

    /* Clear HOST_RESET bit in register to take device out of reset. */
    mrf_write(MRF24_REG_RESET, mrf_read(MRF24_REG_RESET) & ~RESET_SOFT_RESET);

    /*
     * Wait for chip to initialize itself, up to 2 sec.
     * Usually it takes about 140 msec until all registers are ready..
     */
    for (msec=0; msec<2000; msec++) {
        value = mrf_read(MRF24_REG_WFIFO_BCNT0);
        if (value & FIFO_BCNT_MASK)
            break;
        udelay(1000);
    }

    /*
     * Setup interrupts.
     */

    /* Disable the interrupts gated by the 16-bit host int register. */
    mrf_write(MRF24_REG_MASK2, 0);
    mrf_write(MRF24_REG_INTR2, 0xffff);

    /* disable the interrupts gated the by main 8-bit int register. */
    mrf_write_byte(MRF24_REG_MASK, 0);
    mrf_write_byte(MRF24_REG_INTR, 0xff);

    /* Initialize the /INT signal allowing the MRF24 to interrupt
     * the Host from this point forward. */
    mrf_intr_init();

    /* Enable the following interrupts in the 8-bit int register. */
    mask = INTR_FIFO0 | INTR_FIFO1 | INTR_RAW0 | INTR_RAW1 /*| INTR_INT2*/;
    mrf_write_byte(MRF24_REG_MASK, mask);
    mrf_write_byte(MRF24_REG_INTR, mask);

    /* Enable the following interrupts in the 16-bit int register. */
    mask2 = INTR2_RAW2 | INTR2_RAW3 | INTR2_RAW4 | INTR2_RAW5 | INTR2_MAILBOX;
    mrf_write(MRF24_REG_MASK2, mask2);
    mrf_write(MRF24_REG_INTR2, mask2);

    /*
     * Finish the MRF24WG intitialization.
     */
    mrf_raw_init();                     // initialize RAW driver
    mrf_enable_module_operation();      // legacy, but still needed
    rom_version = mrf_get_system_version();
    if (rom_version < 0x3000) {
        /* MRF24WB chip not supported by this driver. */
        return 0;
    }

    mrf_set_tx_confirm(0);              // Disable Tx Data confirms (from the MRF24W)
    mrf_powersave_disable();
    UdSetInitValid();                   // Chip initialized successfully.
    return rom_version;
}
