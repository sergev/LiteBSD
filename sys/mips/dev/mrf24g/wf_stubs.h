/*
 * MRF24WG Stub Functions
 *
 * This module contains prototypes for stub functions.
 */
#ifndef __WF_STUBS_H
#define __WF_STUBS_H

//== I/O Stub Function Protypes ========================================
//   Located in wf_io_stub.c
void WF_GpioSetHibernate(unsigned level);
void WF_GpioSetReset(unsigned level);

//== MRF24WG SPI Stub Function Protypes ========================================
//   Located in wf_spi_stub.c
void WF_SpiEnableChipSelect(void);
void WF_SpiDisableChipSelect(void);
void WF_SpiTxRx(const u_int8_t *p_txBuf, unsigned txLength, u_int8_t *p_rxBuf, unsigned rxLength);

//== MRF24WG External Interrupt Stub Function Protypes =========================
//   Located in wf_eint_stub.c
void WF_EintInit(void);
void WF_EintEnable(void);
void WF_EintDisable(void);
bool WF_isEintDisabled(void);
bool WF_isEintPending(void);
void WF_EintHandler(void);

//== MRF24WG 1ms Timer Stub Function Protypes ==================================
//   Located in wf_timer_stub.c
unsigned WF_TimerRead(void);

//== MRF24WG Event Handler Stub Function Protypes ==============================
// Located in wf_event_stub.c
void WF_ProcessEvent(unsigned eventType, unsigned eventData);

#endif /* __WF_STUBS_H */
