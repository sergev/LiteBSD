/*
 * MRF24WG power control
 *
 * Functions to controll MRF24WG power
 */
#include <sys/param.h>
#include <sys/systm.h>
#include "wf_universal_driver.h"
#include "wf_ud_state.h"

/*
 * Fill in request structure and send message to MRF24WG.
 */
static void set_power_mode(int disable, int wake, int use_dtim)
{
    u_int8_t hdr[6];

    hdr[0] = WF_TYPE_MGMT_REQUEST;
    hdr[1] = WF_SUBTYPE_SET_POWER_MODE;
    hdr[2] = disable;
    hdr[3] = wake;
    hdr[4] = use_dtim;
    hdr[5] = 0;

    mrf_mgmt_send(hdr, sizeof(hdr), 0, 0, 1);
}

/*
 * Enable PS Poll mode.  PS-Poll (Power-Save Poll) is a mode allowing for
 * longer battery life.  The MRF24W coordinates with the Access Point to go
 * to sleep and wake up at periodic intervals to check for data messages, which
 * the Access Point will buffer.  The listen_interval in the Connection
 * Algorithm defines the sleep interval.  By default, PS-Poll mode is disabled.
 *
 * When PS Poll is enabled, the WF Host Driver will automatically force the
 * MRF24W to wake up each time the Host sends Tx data or a control message
 * to the MRF24W.  When the Host message transaction is complete the
 * MRF24W driver will automatically re-enable PS Poll mode.
 *
 * When the application is likely to experience a high volume of data traffic
 * then PS-Poll mode should be disabled for two reasons:
 *  1. No power savings will be realized in the presence of heavy data traffic.
 *  2. Performance will be impacted adversely as the WiFi Host Driver
 *     continually activates and deactivates PS-Poll mode via SPI messages.
 *
 * Parameters:
 *  listen_interval - Number of 100ms intervals between instances when
 *                    the MRF24W wakes up to receive buffered messages
 *                    from the network (1 = 100ms, 2 = 200ms, etc.)
 *  dtim_interval   - Number of DTIM intervals between instances when
 *                    the MRF24W wakes up to receive buffered messages
 *                    from the network.
 *  use_dtim        - true if dtim_interval is being used, else false
 */
void mrf_powersave_enable(unsigned listen_interval, unsigned dtim_interval, int use_dtim)
{
    // if not currently connected then return
    if (UdGetConnectionState() != CS_CONNECTED) {
        printf("--- %s: not connected\n", __func__);
        return;
    }

    mrf_conn_set_listen_interval(listen_interval);
    mrf_conn_set_dtim_interval(dtim_interval);
    set_power_mode(0, 0, use_dtim);
    mrf_powersave_activate(1);
}

/*
 * Disable PS Poll mode.  The MRF24W will stay active and not go sleep.
 */
void mrf_powersave_disable()
{
    set_power_mode(1, 1, 1);
    mrf_powersave_activate(0);
}

/*
 * Driver function to configure PS Poll mode.
 *
 * This function is only used by the driver, not the application.  This
 * function, other than at initialization, is only used when the application
 * has enabled PS-Poll mode.  This function is used to temporarily deactivate
 * PS-Poll mode when there is mgmt or data message tx/rx and then, when message
 * activity has ceased, to again activate PS-Poll mode.
 *
 * Parameters:
 *  enable - true to enable low power mode
 */
void mrf_powersave_activate(int enable)
{
    unsigned scratch1;

    if (enable) {
        /*
         * Activate PS-Poll mode on MRF24W.
         */
        //dbgprintf("Enable PS\n");
        mrf_write(MRF24_REG_PSPOLL, PSPOLL_LP_ENABLE);
    } else {
        /*
         * Deactivate PS-Poll mode on MRF24W.
         * Take it out of low-power mode and wake it up.
         */
        //dbgprintf("Disable PS\n");
        mrf_write(MRF24_REG_PSPOLL, 0);

        /* Poll the response bit that indicates when the MRF24W
         * has come out of low power mode. */
        do {
            mrf_write(MRF24_REG_ADDR, MRF24_INDEX_SCRATCHPAD1);
            scratch1 = mrf_read(MRF24_REG_DATA);
        } while (scratch1 & PSPOLL_LP_ENABLE);
    }
}
