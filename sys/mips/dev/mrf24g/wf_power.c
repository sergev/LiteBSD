/*
 * MRF24WG power control
 *
 * Functions to controll MRF24WG power
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * Enumeration of valid values for WFSetPowerSaveMode()
 */
typedef enum {
    PS_POLL_ENABLED = 0,    /* power save mode enabled  */
    PS_POLL_DISABLED        /* power save mode disabled */
} t_WFPsPwrMode;

typedef struct WFPwrModeReq {
    u_int8_t mode;
    u_int8_t wake;
    u_int8_t rcvDtims;
    u_int8_t reserved;      /* pad byte */
} t_WFPwrModeReq;

static u_int8_t g_powerSaveState = WF_PS_OFF;
static int      g_reactivatePsPoll = 0;

static void SendPowerModeMsg(t_WFPwrModeReq *power_mode)
{
    u_int8_t hdr[2];

    hdr[0] = WF_TYPE_MGMT_REQUEST;
    hdr[1] = WF_SUBTYPE_SET_POWER_MODE;

    mrf_mgmt_send(hdr, sizeof(hdr), (u_int8_t*)power_mode, sizeof(*power_mode), 1);
}

/*
 * Set the desired power save state of the MRF24W.
 * MACInit must be called first.
 *
 * Parameters:
 *  powerSaveState - Value of the power save state desired.
 *
 *   Value                       Definition
 *   -----                       ----------
 *   WF_PS_HIBERNATE             MRF24W in hibernate state
 *   WF_PS_PS_POLL_DTIM_ENABLED  MRF24W in PS-Poll mode with DTIM enabled
 *   WF_PS_PS_POLL_DTIM_DISABLED MRF24W in PS-Poll mode with DTIM disabled
 *   WF_PS_OFF              MRF24W is not in any power-save state
 */
static void PowerStateSet(u_int8_t powerSaveState)
{
    g_powerSaveState = powerSaveState;
}

void WF_PowerStateGet(u_int8_t *p_powerState)
{
    *p_powerState = g_powerSaveState;
}

static void SetPsPollReactivate()
{
    g_reactivatePsPoll = 1;
}

int isPsPollNeedReactivate()
{
    return g_reactivatePsPoll;
}

void ClearPsPollReactivate()
{
    g_reactivatePsPoll = 0;
}

/*
 * Enable PS Poll mode.  PS-Poll (Power-Save Poll) is a mode allowing for
 * longer battery life.  The MRF24W coordinates with the Access Point to go
 * to sleep and wake up at periodic intervals to check for data messages, which
 * the Access Point will buffer.  The listenInterval in the Connection
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
 *  listenInterval - Number of 100ms intervals between instances when
 *                   the MRF24W wakes up to receive buffered messages
 *                   from the network (1 = 100ms, 2 = 200ms, etc.)
 *  dtimInterval   - Number of DTIM intervals between instances when
 *                   the MRF24W wakes up to receive buffered messages
 *                   from the network.
 *  useDtim        - true if dtimInterval is being used, else false
 */
void WF_PsPollEnable(unsigned listenInterval, unsigned dtimInterval, int useDtim)
{
    t_WFPwrModeReq   pwrModeReq;

    // if not currently connected then return
    if (UdGetConnectionState() != CS_CONNECTED) {
        printf("--- %s: not connected\n", __func__);
        return;
    }

    mrf_conn_set_listen_interval(listenInterval);
    mrf_conn_set_dtim_interval(dtimInterval);

    // fill in request structure and send message to MRF24WG
    pwrModeReq.mode     = PS_POLL_ENABLED;
    pwrModeReq.wake     = 0;
    pwrModeReq.rcvDtims = useDtim;
    SendPowerModeMsg(&pwrModeReq);

    WFConfigureLowPowerMode(1);

    if (useDtim) {
        PowerStateSet(WF_PS_PS_POLL_DTIM_ENABLED);
    } else {
        PowerStateSet(WF_PS_PS_POLL_DTIM_DISABLED);
    }
}

/*
 * Disable PS Poll mode.  The MRF24W will stay active and not go sleep.
 * MACInit must be called first.
 */
void WF_PsPollDisable()
{
    t_WFPwrModeReq pwrModeReq;

    pwrModeReq.mode     = PS_POLL_DISABLED;
    pwrModeReq.wake     = 1;
    pwrModeReq.rcvDtims = 1;
    SendPowerModeMsg(&pwrModeReq);

    WFConfigureLowPowerMode(0);
    PowerStateSet(WF_PS_OFF);
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
 * MACInit must be called first.
 *
 * Parameters:
 *  enable - true to enable low power mode
 */
void WFConfigureLowPowerMode(int enable)
{
    u_int16_t lowPowerStatusRegValue;

    //-------------------------------------
    // if activating PS-Poll mode on MRF24W
    //-------------------------------------
    if (enable) {
        //dbgprintf("Enable PS\n");
        mrf_write(MRF24_REG_PSPOLL, PSPOLL_LP_ENABLE);
    }
    //---------------------------------------------------------------------------------------------
    // else deactivating PS-Poll mode on MRF24W (taking it out of low-power mode and waking it up)
    //---------------------------------------------------------------------------------------------
    else {
        //dbgprintf("Disable PS\n");
        mrf_write(MRF24_REG_PSPOLL, 0);

        /* poll the response bit that indicates when the MRF24W has come out of low power mode */
        do {
            // set the index register to the register we wish to read
            mrf_write(MRF24_REG_ADDR, MRF24_INDEX_SCRATCHPAD1);

            // read register
            lowPowerStatusRegValue = mrf_read(MRF24_REG_DATA);
        } while (lowPowerStatusRegValue & PSPOLL_LP_ENABLE);
    }
}

#if 0
/*
 * Enables Hibernate mode on the MRF24W, which effectively turns off the
 * device for maximum power savings.
 *
 * MRF24W state is not maintained when it transitions to hibernate mode.
 * To remove the MRF24W from hibernate mode call WF_Init().
 */
void WF_Hibernate()
{
    WF_GpioSetHibernate(1);
    PowerStateSet(WF_PS_HIBERNATE);
}
#endif

void EnsureWFisAwake()
{
    // if the application has enabled PS mode
    if (g_powerSaveState == WF_PS_PS_POLL_DTIM_ENABLED ||
        g_powerSaveState == WF_PS_PS_POLL_DTIM_DISABLED) {
        // wake up MRF24WG
        WFConfigureLowPowerMode(0);

        // set flag to put it back in PS-Poll when appropriate
        SetPsPollReactivate();
    }
}
