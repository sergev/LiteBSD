/*
 * MRF24WG Universal Driver Global Includes
 *
 * This module contains include files needed by the Universal Driver
 */
#ifndef __WF_GLOBAL_INCLUDES_H
#define __WF_GLOBAL_INCLUDES_H

#include "wf_stubs.h"
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
void WF_CPCreate(void);
unsigned GetCpid(void);

void SetHiddenSsid(bool hiddenSsid);
void SetAdHocMode(int mode);

t_wpaKeyInfo *GetWpsPassPhraseInfo(void);

/*
 * Data Tx/Rx definitions
 */
#define SNAP_VAL        ((u_int8_t)0xaa)
#define SNAP_CTRL_VAL   ((u_int8_t)0x03)
#define SNAP_TYPE_VAL   ((u_int8_t)0x00)

#define SNAP_SIZE       (6)

void SignalPacketRx(void);
bool isPacketRx(void);
void ClearPacketRx(void);
void WF_ProcessWiFiRxData(void);
void RxPacketCheck(void);

/*
 * External interrupt definitions
 */
void InterruptCheck(void);

/*
 * Parameter message defintions
 */
void mrf_enable_module_operation(void);
unsigned GetFactoryMax(void);
void YieldPassPhraseToHost(void);
void SetPSK(u_int8_t *psk);

/*
 * Defintions for the PLL work-around
 */
void ResetPll(void);

/*
 * Definitions for power control
 */
void WFConfigureLowPowerMode(int enable_low_power);
void EnsureWFisAwake(void);
bool isPsPollNeedReactivate(void);
void ClearPsPollReactivate(void);

#endif /* __WF_GLOBAL_INCLUDES_H */
