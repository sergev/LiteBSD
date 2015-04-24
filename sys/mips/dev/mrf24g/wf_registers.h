/*
 * MRF24WG Universal Driver Registers
 *
 * This module contains MRF24WG register definitions
 */
#ifndef __WF_REGISTERS_H
#define __WF_REGISTERS_H

/*
 * Address bit used when accessing MRF24WG registers via SPI interface.
 */
#define MRF24_READ_MODE         0x40

/*
 * MRF24WG direct registers.
 */
#define MRF24_REG_INTR          0x01    /* 8-bit: 1st level interrupt bits */
#define MRF24_REG_MASK          0x02    /* 8-bit: 1st level interrupt mask */
#define MRF24_REG_RAW2_DATA     0x06    /* 8-bit: RAW2 data - Mgmt Rx */
#define MRF24_REG_RAW3_DATA     0x07    /* 8-bit: RAW3 data - Mgmt Tx */
#define MRF24_REG_RAW4_DATA     0x08    /* 8-bit: RAW4 data - Scratch Tx/Rx */
#define MRF24_REG_RAW5_DATA     0x09    /* 8-bit: RAW5 data - not used */
#define MRF24_REG_RAW4_CTRL0    0x0a    /* 16-bit: RAW4 control0 - Scratch Tx/Rx */
#define MRF24_REG_RAW4_CTRL1    0x0b    /* 16-bit: RAW4 control1 - Scratch Tx/Rx */
#define MRF24_REG_RAW4_INDEX    0x0c    /* 16-bit: RAW4 index - Scratch Tx/Rx */
#define MRF24_REG_RAW4_STATUS   0x0d    /* 16-bit: RAW4 status - Scratch Tx/Rx */
#define MRF24_REG_RAW5_CTRL0    0x0e    /* 16-bit: RAW5 control0 - Not used */
#define MRF24_REG_RAW5_CTRL1    0x0f    /* 16-bit: RAW5 control1 - Not used */
#define MRF24_REG_MAILBOX0_HI   0x10    /* 16-bit: mailbox0 upper half */
#define MRF24_REG_MAILBOX0_LO   0x12    /* 16-bit: mailbox0 lower half */
#define MRF24_REG_RAW2_CTRL0    0x18    /* 16-bit: RAW2 control0 - Mgmt Rx */
#define MRF24_REG_RAW2_CTRL1    0x19    /* 16-bit: RAW2 control1 - Mgmt Rx */
#define MRF24_REG_RAW2_INDEX    0x1a    /* 16-bit: RAW2 index - Mgmt Rx */
#define MRF24_REG_RAW2_STATUS   0x1b    /* 16-bit: RAW2 status - Mgmt Rx */
#define MRF24_REG_RAW3_CTRL0    0x1c    /* 16-bit: RAW3 control0 - Mgmt Tx */
#define MRF24_REG_RAW3_CTRL1    0x1d    /* 16-bit: RAW3 control0 - Mgmt Tx */
#define MRF24_REG_RAW3_INDEX    0x1e    /* 16-bit: RAW3 index - Mgmt Tx */
#define MRF24_REG_RAW3_STATUS   0x1f    /* 16-bit: RAW3 status - Mgmt Tx */
#define MRF24_REG_RAW0_DATA     0x20    /* 8-bit: RAW0 data - Data Rx */
#define MRF24_REG_RAW1_DATA     0x21    /* 8-bit: RAW1 data - Data Tx */
#define MRF24_REG_RAW5_INDEX    0x22    /* 16-bit: RAW5 index - Not used */
#define MRF24_REG_RAW5_STATUS   0x23    /* 16-bit: RAW5 status - Not used */
#define MRF24_REG_RAW0_CTRL0    0x25    /* 16-bit: RAW0 control0 - Data Rx */
#define MRF24_REG_RAW0_CTRL1    0x26    /* 16-bit: RAW0 control1 - Data Rx */
#define MRF24_REG_RAW0_INDEX    0x27    /* 16-bit: RAW0 index - Data Rx */
#define MRF24_REG_RAW0_STATUS   0x28    /* 16-bit: RAW0 status - Data Rx */
#define MRF24_REG_RAW1_CTRL0    0x29    /* 16-bit: RAW1 control0 - Data Tx */
#define MRF24_REG_RAW1_CTRL1    0x2a    /* 16-bit: RAW1 control1 - Data Tx */
#define MRF24_REG_RAW1_INDEX    0x2b    /* 16-bit: RAW1 index - Data Tx */
#define MRF24_REG_RAW1_STATUS   0x2c    /* 16-bit: RAW1 status - Data Tx */
#define MRF24_REG_INTR2         0x2d    /* 16-bit: 2nd level interrupt bits */
#define MRF24_REG_MASK2         0x2e    /* 16-bit: 2nd level interrupt mask */
#define MRF24_REG_WFIFO_BCNT0   0x2f    /* 16-bit: available write size for fifo 0 (data tx) */
#define MRF24_REG_WFIFO_BCNT1   0x31    /* 16-bit: available write size for fifo 1 (mgmt tx) */
#define MRF24_REG_RFIFO_BCNT0   0x33    /* 16-bit: number of bytes in read fifo 0 (data rx) */
#define MRF24_REG_RFIFO_BCNT1   0x35    /* 16-bit: number of bytes in read fifo 1 (mgmt rx) */
#define MRF24_REG_RESET         0x3c    /* 16-bit: reset and analog SPI */
#define MRF24_REG_PSPOLL        0x3d    /* 16-bit: control low power mode */
#define MRF24_REG_ADDR          0x3e    /* 16-bit: move the data window */
#define MRF24_REG_DATA          0x3f    /* 16-bit: read or write address-indexed register */

/*
 * MRF24WG indirect registers accessed via the ADDR and DATA registers.
 */
#define MRF24_INDEX_HW_STATUS           0x2a    /* hardware status, read only */
#define MRF24_INDEX_CONFIG_CONTROL0     0x2e    /* used to initiate Hard reset */
#define MRF24_INDEX_WAKE_CONTROL        0x2f    /*  */
#define MRF24_INDEX_SCRATCHPAD0         0x3d    /* scratchpad */
#define MRF24_INDEX_SCRATCHPAD1         0x3e    /* read to determine when low power is done */
#define MRF24_INDEX_PSPOLL_CONFIG       0x40    /*  */
#define MRF24_INDEX_XTAL_SETTLE_TIME    0x41    /*  */

/*
 * Bits for 1st level interrupt and mask registers.
 */
#define INTR_INT2               0x01    /* 2nd level interrupt */
#define INTR_RAW0               0x02    /* RAW0 Move Complete (Data Rx) */
#define INTR_RAW1               0x04    /* RAW1 Move Complete (Data Tx) */
#define INTR_FIFO0              0x40    /* Data Rx */
#define INTR_FIFO1              0x80    /* Mgmt Rx */

/*
 * Bits for 2nd level interrupt and mask registers.
 */
#define INTR2_MAILBOX           0x0001  /* Assertion */
#define INTR2_RAW4              0x0004  /* RAW4 Move Complete (Scratch) */
#define INTR2_RAW5              0x0008  /* RAW5 Move Complete (Not used) */
#define INTR2_RAW2              0x0010  /* RAW2 Move Complete (Mgmt Rx) */
#define INTR2_RAW3              0x0020  /* RAW3 Move Complete (Mgmt Tx) */

/*
 * Bits for FIFO byte count registers.
 */
#define FIFO_BCNT_MASK          0x0fff  /* lower 12 bits contain length */

/*
 * Bits for Reset register.
 */
#define RESET_SOFT_RESET        0x0001  /* Soft reset */
#define RESET_ANA_SPI_CLK       0x0200  /* Clock for analog SPI */
#define RESET_ANA_SPI_DOUT      0x0400  /* Data out for analog SPI */
#define RESET_ANA_SPI_EN        0x1000  /* Chip enable for analog SPI */

/*
 * Bits for PSPOLL register.
 */
#define PSPOLL_LP_ENABLE        0x0001  /* Enable low power mode */

/*
 * Bits for HW_STATUS register.
 */
#define HW_STATUS_RESET_DONE    0x1000  /* hardware reset completed */

/*
 * Bits for RAWx_STATUS register.
 */
#define RAW_STATUS_BUSY         0x0001  /* still busy */

/*
 * Address bits used when accessing analog chip registers
 * through software SPI via RESET register.
 */
#define ANA_SPI_READ_MODE       0x01    /* bit 0 */
#define ANA_SPI_NO_AUTO_INCR    0x02    /* bit 1 */

// SPI Port 3 Registers (Port 5 if going through Master SPI controller)
#define ANALOG_REG_PLL0         (0 * 2)
#define ANALOG_REG_PLL1         (1 * 2)
#define ANALOG_REG_PLL2         (2 * 2)
#define ANALOG_REG_PLL3         (3 * 2)
#define ANALOG_REG_PLL4         (4 * 2)
#define ANALOG_REG_PLL5         (5 * 2)
#define ANALOG_REG_PLL6         (6 * 2)
#define ANALOG_REG_PLL7         (7 * 2)
#define ANALOG_REG_PLL8         (8 * 2)
#define ANALOG_REG_PLL9         (9 * 2)

#define ANALOG_REG_OSC0         (0 * 2)
#define ANALOG_REG_OSC1         (1 * 2)
#define ANALOG_REG_OSC2         (2 * 2)
#define ANALOG_REG_PLDO         (3 * 2)
#define ANALOG_REG_BIAS         (4 * 2)
#define ANALOG_REG_SPARE        (5 * 2)

#ifdef KERNEL
/*
 * SPI Functions
 */
unsigned mrf_read_byte(unsigned regno);
void     mrf_write_byte(unsigned regno, unsigned value);
void     mrf_write(unsigned regno, unsigned value);
unsigned mrf_read(unsigned regno);
void     mrf_write_array(unsigned regno, const u_int8_t *src, unsigned nbytes);
void     mrf_read_array(unsigned regno, u_int8_t *dest, unsigned nbytes);
#endif

#endif /* __WF_REGISTERS_H */
