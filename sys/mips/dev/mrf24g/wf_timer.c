/*
 * MRF24WG timer helper function.
 *
 * Function to determine elapsed time (in ms).
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

#define WF_MAX_TIMER_COUNT  0xffffffffU // count at which timer rolls back to 0

u_int32_t GetElapsedTime(u_int32_t startTime, u_int32_t currentTime)
{
    if (currentTime >= startTime)
        return currentTime - startTime;

    return (WF_MAX_TIMER_COUNT - startTime) + currentTime;
}
