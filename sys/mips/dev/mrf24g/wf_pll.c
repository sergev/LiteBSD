/*
 * MRF24WG PLL work-around
 *
 * MRF24WG needs to have PLL registers updated prior to performing chip reset.
 */
#include "wf_universal_driver.h"
#include "wf_global_includes.h"

/*
 * When bit-banging, determines which SPI port to use
 * based on the type of register we are accessing.
 */
static uint8_t GetSpiPort(uint8_t regType)
{
    switch (regType) {
    case ANALOG_PORT_0_REG_TYPE: return 0;
    case ANALOG_PORT_1_REG_TYPE: return 1;
    case ANALOG_PORT_2_REG_TYPE: return 3;
    case ANALOG_PORT_3_REG_TYPE: return 2;
    default:                     return 0xff; // should never happen
    }
}

static void WriteAnalogRegisterBitBang(uint8_t regType, uint16_t address, uint16_t value)
{
    uint8_t  spiPort;
    uint16_t hrVal;
    uint8_t  bitMask8;
    uint16_t bitMask16;
    uint8_t  i;
    uint8_t  regAddress;

    spiPort = GetSpiPort(regType);   // extract SPI port (0-3) from the register type

    // Enable the on-chip SPI and select the desired bank (0-3)
    hrVal = (HR_HOST_ANA_SPI_EN_MASK | (spiPort << 6));
    Write16BitWFRegister(WF_HOST_RESET_REG, hrVal);

    // create register address byte
    regAddress = (address << 2) | SPI_AUTO_INCREMENT_ENABLED_MASK | SPI_WRITE_MASK;

    // bit-bang the regAddress byte, MS bit to LS bit
    bitMask8 = 0x80;        // start with MS bit of byte being bit-banged out
    for (i = 0; i < 8; ++i) {
        hrVal &= ~(HR_HOST_ANA_SPI_DOUT_MASK | HR_HOST_ANA_SPI_CLK_MASK); // zero out DOUT and CLK

        // mask out ADDRESS bit being clocked and write to HOST_ANA_SPI_DOUT (bit 10) in HOST_RESET_REG with the HOST_ANA_SPI_CLK low
        if ((regAddress & bitMask8) > 0) {
            hrVal |= HR_HOST_ANA_SPI_DOUT_MASK;
        }
        Write16BitWFRegister(WF_HOST_RESET_REG, hrVal);

        // now toggle SPI clock high, on rising edge this bit is clocked out
        hrVal |= HR_HOST_ANA_SPI_CLK_MASK;
        Write16BitWFRegister(WF_HOST_RESET_REG, hrVal);

        bitMask8 >>= 1; //  # get to next bit in address byte
    }

    // bit bang data from MS bit to LS bit
    bitMask16 = 0x8000;        // start with MS bit of byte being bit-banged out
    for (i = 0; i < 16; ++i) {
        hrVal &= ~(HR_HOST_ANA_SPI_DOUT_MASK | HR_HOST_ANA_SPI_CLK_MASK); // zero out DOUT and CLK

        // mask in data bit being clock out and write to HOST_ANA_SPI_DOUT (bit 10) in HOST_RESET_REG with the HOST_ANA_SPI_CLK low
        if ((value & bitMask16) > 0) {
            hrVal |= HR_HOST_ANA_SPI_DOUT_MASK;
        }

        Write16BitWFRegister(WF_HOST_RESET_REG, hrVal);

        // now toggle SPI clock high, on rising edge this bit is clocked out
        hrVal |= HR_HOST_ANA_SPI_CLK_MASK;
        Write16BitWFRegister(WF_HOST_RESET_REG, hrVal);

        bitMask16 = bitMask16 >> 1;  // go to next bit in data byte
    }

    // Disable the on-chip SPI
    hrVal &= ~HR_HOST_ANA_SPI_EN_MASK;
    Write16BitWFRegister(WF_HOST_RESET_REG, hrVal);
}

void ResetPll()
{
    // shuttle MRF24WG workaround (benign to production MRF24WG)
    WriteAnalogRegisterBitBang(ANALOG_PORT_3_REG_TYPE, PLL0_REG, 0x8021);
    WriteAnalogRegisterBitBang(ANALOG_PORT_3_REG_TYPE, PLL0_REG, 0x6021);

    // production MRF24WG workaround (benign to shuttle MRF24WG)
    WriteAnalogRegisterBitBang(ANALOG_PORT_1_REG_TYPE, OSC0_REG, 0x6b80);
    WriteAnalogRegisterBitBang(ANALOG_PORT_1_REG_TYPE, BIAS_REG, 0xc000);
}
