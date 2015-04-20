/*
 * MRF24WG Stub Functions
 *
 * This module contains prototypes for stub functions.
 */
#ifndef __WF_STUBS_H
#define __WF_STUBS_H

/*
 * SPI Functions
 */
unsigned mrf_read_byte(unsigned regId);
void     mrf_write_byte(unsigned regId, unsigned value);
void     mrf_write(unsigned regId, unsigned value);
unsigned mrf_read(unsigned regId);
void     mrf_write_array(unsigned regId, const u_int8_t *p_Buf, unsigned length);
void     mrf_read_array(unsigned regId, u_int8_t *p_Buf, unsigned length);

/*
 * External Interrupt Functions
 */
void    mrf_intr_init(void);
int     mrf_intr_enable(void);
int     mrf_intr_disable(void);
void    WF_EintHandler(void);

/*
 * 1ms Timer Function
 */
unsigned mrf_timer_read(void);
int      mrf_timer_elapsed(unsigned start_time);

/*
 * Event Handler Functions
 */
void    WF_ProcessRxPacket(void);

#endif /* __mrf_STUBS_H */
