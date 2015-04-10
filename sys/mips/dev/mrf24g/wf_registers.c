/*
 * MRF24WG register access
 *
 * Functions to access registers on MRF24WG.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * Using global buffers because the register functions are called from the
 * external interrupt routine, and for parts that use overlay memory using local
 * buffers fails.
 */
static u_int8_t g_txBuf[3];
static u_int8_t g_rxBuf[3];

/*
 * Read WF 8-bit register.
 * Returns register value.
 */
u_int8_t Read8BitWFRegister(u_int8_t regId)
{
    g_txBuf[0] = regId | WF_READ_REGISTER_MASK;
    WF_SpiEnableChipSelect();
    WF_SpiTxRx(g_txBuf, 1, g_rxBuf, 2);
    WF_SpiDisableChipSelect();

    return g_rxBuf[1];   /* register value returned in the second byte clocking */
}

/*
 * Write WF 8-bit register.
 */
void Write8BitWFRegister(u_int8_t regId, u_int8_t value)
{
    g_txBuf[0] = regId | WF_WRITE_REGISTER_MASK;
    g_txBuf[1] = value;

    WF_SpiEnableChipSelect();
    WF_SpiTxRx(g_txBuf, 2, g_rxBuf, 1);
    WF_SpiDisableChipSelect();
}

/*
 * Write WF 16-bit register.
 */
void Write16BitWFRegister(u_int8_t regId, u_int16_t value)
{
    g_txBuf[0] = regId | WF_WRITE_REGISTER_MASK;
    g_txBuf[1] = (u_int8_t)(value >> 8);       /* MS byte being written */
    g_txBuf[2] = (u_int8_t)(value & 0x00ff);   /* LS byte being written */

    WF_SpiEnableChipSelect();
    WF_SpiTxRx(g_txBuf, 3, g_rxBuf, 1);
    WF_SpiDisableChipSelect();
}

/*
 * Read WF 16-bit register.
 * Returns register value.
 */
u_int16_t Read16BitWFRegister(u_int8_t regId)
{
    g_txBuf[0] = regId | WF_READ_REGISTER_MASK;
    WF_SpiEnableChipSelect();
    WF_SpiTxRx(g_txBuf, 1, g_rxBuf, 3);
    WF_SpiDisableChipSelect();

    return (((u_int16_t)g_rxBuf[1]) << 8) | ((u_int16_t)(g_rxBuf[2]));
}

/*
 * Write a data block to specified raw register.
 * Parameters:
 *  regId  -- Raw register being written to
 *  pBuf   -- pointer to array of bytes being written
 *  length -- number of bytes in pBuf
 */
void WriteWFArray(u_int8_t regId, const u_int8_t *p_Buf, u_int16_t length)
{
    g_txBuf[0] = regId;

    WF_SpiEnableChipSelect();

    /* output cmd byte */
    WF_SpiTxRx(g_txBuf, 1, g_rxBuf, 1);

    /* output data array bytes */
    WF_SpiTxRx(p_Buf, length, g_rxBuf, 1);

    WF_SpiDisableChipSelect();
}

/*
 * Read a block of data from a raw register.
 * Parameters:
 *  regId  -- Raw register being read from
 *  pBuf   -- pointer where to write out bytes
 *  length -- number of bytes to read
 */
void ReadWFArray(u_int8_t regId, u_int8_t *p_Buf, u_int16_t length)
{
    WF_SpiEnableChipSelect();

    /* output command byte */
    g_txBuf[0] = regId | WF_READ_REGISTER_MASK;
    WF_SpiTxRx(g_txBuf, 1, g_rxBuf, 1);

    /* read data array (send garbage tx byte) */
    WF_SpiTxRx(g_txBuf, 1, p_Buf, length);

    WF_SpiDisableChipSelect();
}
