/*
 * MRF24WG run-time task.
 *
 * Called from round-robin to check for WiFi events.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

extern void ChipResetStateMachine(void);

static void EventCheck()
{
    t_event event;

    // if event queue empty then nothing to do
    if (isEventQEmpty()) {
        return;
    }

    // get next event from queue
    EventDequeue(&event);

    // notify application of event
    WF_ProcessEvent(event.eventType, event.eventData);
}

static void PsPollCheck()
{
    // if PS-Poll was disabled temporarily and needs to be reenabled, and, we are in
    // a connected state
    if (isPsPollNeedReactivate() &&
        UdGetConnectionState() == CS_CONNECTED)
    {
        ClearPsPollReactivate();
        WFConfigureLowPowerMode(WF_LOW_POWER_MODE_ON);
    }
}

void WF_Task()
{
    ChipResetStateMachine();

    EventCheck();

    InterruptCheck();

    RxPacketCheck();

    PsPollCheck();
}
