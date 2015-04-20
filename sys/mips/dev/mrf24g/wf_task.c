/*
 * MRF24WG run-time task.
 *
 * Called from round-robin to check for WiFi events.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

void WF_Task()
{
    InterruptCheck();

    RxPacketCheck();

    // if PS-Poll was disabled temporarily and needs to be reenabled, and, we are in
    // a connected state
    if (isPsPollNeedReactivate() &&
        UdGetConnectionState() == CS_CONNECTED)
    {
        ClearPsPollReactivate();
        WFConfigureLowPowerMode(WF_LOW_POWER_MODE_ON);
    }
}
