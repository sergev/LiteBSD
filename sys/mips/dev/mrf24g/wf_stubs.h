/*
 * MRF24WG Stub Functions
 *
 * This module contains prototypes for stub functions.
 */
#ifndef __WF_STUBS_H
#define __WF_STUBS_H

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
