/*
 * MRF24WG Universal Driver Global Includes
 *
 * This module contains include files needed by the Universal Driver
 */
#ifndef __WF_GLOBAL_INCLUDES_H
#define __WF_GLOBAL_INCLUDES_H

#include "wf_events.h"
#include "wf_connection_event_codes.h"
#include "wf_mgmt_msg.h"
#include "wf_ud_state.h"
#include "wf_raw.h"
#include "wf_registers.h"
#include <sys/param.h>
#include <sys/systm.h>

/*
 * Connection profile functions.
 */
void SignalPacketRx(void);
int isPacketRx(void);
void ClearPacketRx(void);
void WF_ProcessWiFiRxData(void);
void RxPacketCheck(void);

/*
 * External interrupt definitions
 */
void InterruptCheck(void);

/*
 * Definitions for power control
 */
int isPsPollNeedReactivate(void);
void ClearPsPollReactivate(void);

#endif /* __WF_GLOBAL_INCLUDES_H */
