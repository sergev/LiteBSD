/*
 * MRF24WG timer helper function.
 *
 * Function to determine elapsed time (in ms).
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

#define WF_MAX_TIMER_COUNT  ((uint32_t)0xffffffff)  // count at which timer rolls back to 0

uint32_t GetElapsedTime(uint32_t startTime, uint32_t currentTime)
{
    if (currentTime >= startTime)
        return currentTime - startTime;

    return (WF_MAX_TIMER_COUNT - startTime) + currentTime;
}
