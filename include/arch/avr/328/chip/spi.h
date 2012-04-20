/*
 *  BermudaOS - Serial Perhical Interface
 *  Copyright (C) 2012   Michel Megens
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file spi.h */

#ifndef __SPI_H
#define __SPI_H

#include <arch/avr/io.h>
#include <bermuda.h>
#include <lib/binary.h>

#define SPI_INIT  0
#define SPI_IRQ   1
#define SPI_SLEEP 2
#define SPI_MODE  3

#define SPI_POLARITY_RISING 0
#define SPI_POLARITY_FALLING 1

#define SPI_CLOCK_PHASE_SAMPLE 0
#define SPI_CLOCK_PHASE_SETUP  1

#define BermudaSpiIsMaster(spi) (spi->flags & BIT(3))
#define BermudaSpiIsInitialized(spi) (spi->flags & BIT(0))

typedef struct spi SPI;

struct spi
{
        char *name;
        unsigned char id;
        
        unsigned char (*transact)(SPI *spi, unsigned char data);
        unsigned char flags; /*
                              * 0   -> set to one when initialized
                              * 1   -> ISR enabled
                              * 2   -> Sleep when waiting for transfer
                              * 3   -> SPI is in master mode when 1
                              * 4   -> SPI 2X
                              */
        unsigned char prescaler;
        volatile unsigned char *spcr, *spsr, *spdr;
} __PACK__;

typedef enum
{
        SPI_MASTER,
        SPI_SLAVE,
} spi_mode_t;

#define BermudaGetSPCR() SFR_IO8(0x2C)
#define BermudaGetSPSR() SFR_IO8(0x2D)
#define BermudaGetSPDR() SFR_IO8(0x2E)

/*
 * SPI defaults
 */
#define SPI_PRESCALER_DEFAULT 128

__DECL
extern SPI *BermudaSPI;

extern unsigned char BermudaSpiRead(SPI *spi);
extern void BermudaSpiWrite(SPI *spi, unsigned char data);

/**
 * \fn BermudaSpiInit(SPI *spi)
 * \brief Initialise the Serial Peripheral Interface.
 * \param spi SPI instance
 *
 * The parameter <i>spi</i> has to be allocated before passed to this function.
 * When this function returns the SPI is ready to use.
 */
extern int BermudaSpiInit(SPI *spi);
extern int BermudaSpiDestroy(SPI *spi);

/**
 * \fn BermudaSetMasterSpi(SPI *spi)
 * \brief Enable master mode.
 * \param spi The SPI structure to enable master mode on.
 *
 * This function puts the SPI interface in master mode
 */
extern void BermudaSetMasterSpi(SPI *spi);

/**
 * \fn BermudaSetSlaveSpi(SPI *spi)
 * \brief Enable slave mode.
 * \param spi The SPI structure to enable slave mode on.
 *
 * This function puts the SPI interface in slave mode
 */
extern void BermudaSetSlaveSpi(SPI *spi);

/**
  * \fn BermudaSpiTransmit(SPI *spi, void *data, size_t len)
  * \brief Transmit one byte over the SPI.
  * \param spi The SPI to use.
  * \param data The data to sent.
  * \param len The length of <i>data</i>.
  * \return [0] on success, -1 otherwise.
  *
  * This function transmits an array of data over the SPI. It should only
  * be called when the SPI is in master mode. When the SPI is not in master
  * mode and this function is called, it will return -2.
  */
extern int BermudaSpiTransmitBuf(SPI *spi, void *data, size_t len);

/**
  * \fn BermudaSpiTxByte(SPI *spi, unsigned char data)
  * \brief Transmit one byte over the SPI.
  * \param spi The SPI to use.
  * \param data The data byte to sent.
  * \return [0] on success, -1 otherwise.
  *
  * This function transmits one data byte over the given SPI.
  */
extern unsigned char BermudaSpiTransmit(SPI *spi, unsigned char data);

/**
 * \fn BermudaSpiNativeInit()
 * \brief Bit-banged initialisation
 *
 * This function initializes the SPI alot faster, but can only be used when
 * all other operations are implemented by the application
 */
extern void BermudaSpiNativeInit();

/**
 * \fn BermudaSpiGetInterface()
 * \brief Return the main SPI interface
 * \return The SPI bus interface
 * 
 * Returns the main SPI. This the SPI which is initialized first.
 */
static inline SPI *BermudaSpiGetInterface()
{
        return BermudaSPI;
}

/**
 * \fn BermudaSpiEnable(unsigned char ss)
 * \brief Enable the given SPI.
 * \param ss The SS pin to enable.
 * 
 * This function pulls the given SS pin low.
 */
static inline void BermudaSpiStart(unsigned char ss)
{
        BermudaDigitalPinWrite(ss, LOW);
}

/**
 * \fn BermudaSpiDisable(unsigned char ss)
 * \brief Disable the given SPI.
 * \param ss The SS pin to disable.
 * 
 * This function will pull the given SS pin high.
 */
static inline void BermudaSpiStop(unsigned char ss)
{
        BermudaDigitalPinWrite(ss, HIGH);
}

#ifdef __LAZY__
/**
 * \fn BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler)
 * \brief Setup the desired socket prescaler.
 * \param spi The SPI to set the prescaler for.
 * \param prescaler The desired prescaler. It must be a power of 2 (<= 128).
 * \return 0 on success, else otherwise.
 *
 * This will configure the SCK prescaler. If the prescaler is not a power of two,
 * -1 will be returned.
 *
 * Bit 0 of <i>prescaler</i> indicates of the SPI should operate in double speed
 * mode.
 */
PRIVATE WEAK int BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler);
/**
 * BermudaSpiSetSckMode(SPI *spi, unsigned char mode)
 * \brief Set the serial clock mode.
 * \param spi The SP interface.
 * \param mode The mode to apply to <i>spi</i>.
 * \return Error code.
 *
 * * [0] Low on idle, sample.
 * * [1] Low on idle, setup.
 * * [2] High on idle, sample.
 * * [4] High on idle, setup.
 */
PRIVATE WEAK int BermudaSpiSetSckMode(SPI *spi, unsigned char mode);
#else
PRIVATE WEAK void BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler);

/**
 * BermudaSpiSetSckMode(SPI *spi, unsigned char mode)
 * \brief Set the serial clock mode.
 * \param spi The SP interface.
 * \param mode The mode to apply to <i>spi</i>.
 *
 * mode[0]: Represents the CPHA bit.
 * mode[1]: Represents the CPOL bit.
 */
PRIVATE WEAK void BermudaSpiSetSckMode(SPI *spi, unsigned char mode);
#endif

/**
 * \fn BermudaSetupSpiRegs(SPI *spi)
 * \brief Setup I/O registers.
 * \param spi The SPI to setup registers for.
 *
 * This function links the SPI structure to the register addresses.
 */
PRIVATE WEAK void BermudaSetupSpiRegs(SPI *spi);

/**
 * \fn BermudaSetSpiMode(SPI *spi, spi_mode_t mode)
 * \brief Set the SPI operation mode.
 * \param spi The SPI to set the operation mode for.
 * \param mode Operation mode to set.
 *
 * This function will put the SPI in either master or slave mode, depending on
 * the value of mode.
 */
PRIVATE WEAK void BermudaSetSpiMode(SPI *spi, spi_mode_t mode);

/**
  * \fn BermudaSpiTxByte(SPI *spi, unsigned char data)
  * \brief Transmit one byte over the SPI.
  * \param spi The SPI to use.
  * \param data The data byte to sent.
  * \return [0] on success, -1 otherwise.
  *
  * This function transmits one data byte over the given SPI.
  */
PRIVATE WEAK unsigned char BermudaSpiTxByte(SPI *spi, unsigned char data);

/**
 * \fn BermudaSetSpiBitOrder(SPI *spi, unsigned char order)
 * \brief The bit order can be configured using this function.
 * \param spi SPI descriptor.
 * \param order The bit order.
 *
 * When order is not 0 the bit order will be LSB first, otherwise MSB first.
 */
PRIVATE inline void BermudaSetSpiBitOrder(SPI *spi, unsigned char order);

/**
 * \fn BermudaSpiEnable(SPI *spi)
 * \brief Enable the SPI.
 * \param spi The SPI to enable.
 *
 * This function enables the given SPI interface.
 */
PRIVATE inline void BermudaSpiEnable(SPI *spi);

/**
 * \fn BermudaSpiDisable(SPI *spi)
 * \brief Enable the SPI.
 * \param spi The SPI to disable.
 *
 * This function disables the given SPI interface.
 */
PRIVATE inline void BermudaSpiDisable(SPI *spi);

#ifdef THREADS
/**
 * \fn BermudaAttatchSpiIRQ(SPI *spi)
 * \brief Setup the SPI ISR.
 * \param spi The ISR will be attatched to this SPI descriptor.
 *
 * This function enables this SPI interrupt flag. If the interrupts are enabled
 * globally, then the SPI ISR will be called when a SPI transfer is complete.
 */
PRIVATE WEAK void BermudaAttatchSpiIRQ(SPI *spi);

/**
 * \fn BermudaDetachSpiIRQ(SPI *spi)
 * \brief Disable the SPI ISR.
 * \param spi The ISR will be dettatched from this SPI descriptor.
 *
 * This function disables the SPI interrupt flag. After calling, the SPI ISR will
 * not be called again by the SPI.
 */
PRIVATE WEAK void BermudaDetachSpiIRQ(SPI *spi);
#endif
__DECL_END

#endif /* __SPI_H */
