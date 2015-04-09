/*
 * MRF24WG Event Queue
 *
 * Functions pertaining to event queue handling
 * As WiFi events occur they are stored in the event queue, and
 * ultimately passed to the WF_ProcessEvent function in wf_event_stub.c
 */
#include <stdio.h>
#include <string.h>
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

#define IncrementReadIndex()                    \
    if (g_eventQueue.readIndex == MAX_EVENTS) { \
        g_eventQueue.readIndex = 0;             \
    } else {                                    \
        ++g_eventQueue.readIndex;               \
    }

#define IncrementWriteIndex()                   \
    if (g_eventQueue.writeIndex == MAX_EVENTS) {\
        g_eventQueue.writeIndex = 0;            \
    } else {                                    \
        ++g_eventQueue.writeIndex;              \
    }

static t_wfEventQueue g_eventQueue;

/*
 * Initialize MRF24WG event queue.
 *
 * Called by Universal Driver during initialization.  After this call the
 * event queue is in an empty state.
 */
void EventQInit()
{
    memset(&g_eventQueue, 0x00, sizeof(g_eventQueue));

    g_eventQueue.readIndex  = 0;
    g_eventQueue.writeIndex = 0;
}

/*
 * Adds an MRF24WG event to the event queue.
 * Called by Universal Driver when events occur.
 *
 * Parameters:
 *  eventType - event type
 *  eventData - data associated with the event; not always used.
 */
void EventEnqueue(uint8_t eventType, uint32_t eventData)
{
#if 0
    if (eventType == WF_EVENT_ERROR) {
        dbgprintf("WF_EVENT_ERROR\n");
    }
#endif

    t_event *p_eventInQueue;

    // if event queue has filled then most likely a series of errors have occurred,
    // so it is probably best to simply throw away any new events
    if (isEventQFull()) {
        return;
    }

    p_eventInQueue = &g_eventQueue.event[g_eventQueue.writeIndex];

    // write data to queue
    p_eventInQueue->eventType = eventType;
    p_eventInQueue->eventData = eventData;

    IncrementWriteIndex();
}

/*
 * Remove an event from the event queue.
 * Returns pointer to event object
 *
 * Called by Universal Driver when it is going to report an event to the
 * application via a callback function.
 *
 * Parameters:
 *  p_event - pointer to where event data is written
 */
void EventDequeue(t_event *p_event)
{
    t_event *p_eventInQueue;

    // event queue should never be empty
    if (isEventQEmpty()) {
        // ToDo: callback here?
        for(;;);
    }

    // point to next event in event queue
    p_eventInQueue = &g_eventQueue.event[g_eventQueue.readIndex];

    // copy event info into callers structure
    p_event->eventType = p_eventInQueue->eventType;
    p_event->eventData = p_eventInQueue->eventData;

    IncrementReadIndex();
}

/*
 * Determine if event queue is empty.
 */
bool isEventQEmpty()
{
    return (g_eventQueue.readIndex == g_eventQueue.writeIndex);
}

/*
 * Queue full when writeIndex + 1 = readIndex
 */
bool isEventQFull()
{
    uint8_t readIndex  = g_eventQueue.readIndex;
    uint8_t writeIndex = g_eventQueue.writeIndex;

    // check wrap case first
    if ((writeIndex == MAX_EVENTS) && (readIndex == 0)) {
        return true;
    }
    // else do regular check
    else if (writeIndex + 1 == readIndex) {
        return true;
    }
    return false;
}
