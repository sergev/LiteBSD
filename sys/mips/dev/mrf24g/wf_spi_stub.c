/*
 * MRF24WG SPI Stub Functions
 *
 * Functions to control MRF24WG RESET and HIBERNATE pins need by the
 * Universal Driver.  Functions in this module should not be called
 * directly by the application code.
 */
#include "board_profile.h"
#include "wf_universal_driver.h"

/*
 * Initializes SPI controller used to communicate with MRF24WG.
 * Called by Universal Driver during initialization to initialize SPI interface.
 */
void WF_SpiInit()
{
    uint8_t rxData __attribute__((unused));

#ifdef WF_INT_PPS
    WF_INT_PPS();
    WF_HIBERNATE_PPS();
    WF_RESET_PPS();
    WF_CS_PPS();
    WF_SCK_PPS();
    WF_SDI_PPS();
    WF_SDO_PPS();
#endif

    // The MRF24WG chip select line will be controlled using an I/O pin, not by
    // the SPI controller.
    WF_CS_SET(1);           // set the level high (chip select disabled)
    WF_CS_INPUT(0);         // Configure the I/O pin as an output and drive it

    // disable the three possible SPI interrupts (SPI Fault, SPI Rx done,
    // SPI Transfer Done).  Universal Driver does not require SPI interrupts.
    IECCLR(WIFI_SPI_IRQ_E >> 5) = 1 << (WIFI_SPI_IRQ_E & 31);
    IECCLR(WIFI_SPI_IRQ_TX >> 5) = 1 << (WIFI_SPI_IRQ_TX & 31);
    IECCLR(WIFI_SPI_IRQ_RX >> 5) = 1 << (WIFI_SPI_IRQ_RX & 31);

    // disable SPI controller
    WIFI_SPICONCLR = PIC32_SPICON_ON;

    // clear the receive buffer
    rxData = WIFI_SPIBUF;

    // set the SPI baud rate
    WIFI_SPIBRG = ((BUS_FREQ + WF_SPI_FREQ) / 2 / WF_SPI_FREQ) - 1;

    // configure SPI port:
    //   * Master Mode enabled (MSTEN = 1)
    //   * Clock Polarity is idle high, active low (CKP = 1)
    //   * Serial output data changes on transition from idle clock state to active
    //       clock state, or high to low transition (CKE = 0)
    //   * Data sampled at end of output time (SMP = 1), on rising edge
    WIFI_SPICON = PIC32_SPICON_MSTEN |
                  PIC32_SPICON_CKP |
                  PIC32_SPICON_SMP;

    // enable SPI controller
    WIFI_SPICONSET = PIC32_SPICON_ON;
}

/*
 * Select the MRF24WG SPI by setting the CS line low.
 * Called by Universal Driver when preparing to transmit data to the MRF24WG.
 */
void WF_SpiEnableChipSelect()
{
    // if the SPI controller is being shared with another device then save its
    // context here and configure the SPI controller for the MRF24WG
    // (not needed for this hardware platform)

    // select the MRF24WG
    WF_CS_SET(0);
}

/*
 * Deselect the MRF24WG SPI by setting CS high.
 * Called by Universal Driver after completing an SPI transaction.
 */
void WF_SpiDisableChipSelect()
{
    // deselect the MRF24WG
    WF_CS_SET(1);

    // if the SPI controller is being shared with another device then restore its
    // context here from the context that was saved in mrf_spi_enable_cs().
    // (not needed for this hardware platform)

}

/*
 * Transmit and receive SPI bytes with the MRF24WG.
 * Called by Universal Driver to communicate with the MRF24WG.
 * Parameters:
 *   p_txBuf  -- pointer to the transmit buffer
 *   txLength -- number of bytes to be transmitted from p_txBuf
 *   p_rxBuf  -- pointer to receive buffer
 *   rxLength -- number of bytes to read and copy into p_rxBuf
 */
void WF_SpiTxRx(const uint8_t *p_txBuf,
                uint16_t txLength,
                uint8_t *p_rxBuf,
                uint16_t rxLength)
{
    uint16_t byteCount;
    uint16_t i;
    uint8_t  rxTrash __attribute__((unused));

    // TODO: need a check either here or somewhere else to flag an error if
    //       MRF24WG is in hibernate mode.  Another stub function to check if
    //       in hibernate mode?

    // total number of bytes to clock out is whichever is larger, txLength or rxLength
    byteCount = (txLength >= rxLength)?txLength:rxLength;

    // for each byte being clocked
    for (i = 0; i < byteCount; ++i)
    {
        /* if still have bytes to transmit from tx buffer */
        if (txLength > 0)
        {
            WIFI_SPIBUF = *p_txBuf++;
            --txLength;
        }
        /* else done writing bytes out from tx buffer */
        else
        {
            WIFI_SPIBUF = 0xff;  /* clock out a "don't care" byte */
        }

        // wait until byte in SPI Tx buffer has fully clocked and the incoming
        // byte is fully received
        for (;;) {
            unsigned stat = WIFI_SPISTAT;

            if ((stat & PIC32_SPISTAT_SPITBE) &&
                (stat & PIC32_SPISTAT_SPIRBF))
                    break;
        }

        // if still have bytes to read into rx buffer
        if (rxLength > 0)
        {
            *p_rxBuf++ = WIFI_SPIBUF;
            --rxLength;
        }
        // else done reading bytes into rx buffer
        else
        {
            rxTrash = WIFI_SPIBUF;  // read and throw away byte
        }
    }  // end for loop
}
