/*
 * MRF24WG Stub Functions
 *
 * This module contains prototypes for stub functions.
 */
#ifndef __WF_STUBS_H
#define __WF_STUBS_H

/*
 * GPIO Functions
 */
void WF_GpioSetHibernate(unsigned level);
void WF_GpioSetReset(unsigned level);

/*
 * SPI Functions
 */
unsigned WF_ReadByte(unsigned regId);
void     WF_WriteByte(unsigned regId, unsigned value);
void     WF_Write(unsigned regId, unsigned value);
unsigned WF_Read(unsigned regId);
void     WF_WriteArray(unsigned regId, const u_int8_t *p_Buf, unsigned length);
void     WF_ReadArray(unsigned regId, u_int8_t *p_Buf, unsigned length);

/*
 * External Interrupt Functions
 */
void WF_EintInit(void);
void WF_EintEnable(void);
void WF_EintDisable(void);
bool WF_isEintDisabled(void);
bool WF_isEintPending(void);
void WF_EintHandler(void);

/*
 * 1ms Timer Function
 */
unsigned WF_TimerRead(void);

/*
 * Event Handler Functions
 */
void WF_ProcessEvent(unsigned eventType, unsigned eventData);
void WF_ProcessRxPacket(void);

#endif /* __WF_STUBS_H */
