/*
 *  BermudaOS - Serial Peripheral Interface
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

/** \file */

#if  defined(__SPI__) || defined(__DOXYGEN__)

#include <bermuda.h>
#include <sys/thread.h>
#include <sys/sched.h>
#include <lib/binary.h>
#include <avr/interrupt.h>
#include <arch/avr/io.h>

/**
 * \var BermudaSPI
 * \brief Global SPI structure.
 * 
 * Global definition of the SPI. Initialised to NULL by default
 */
SPI *BermudaSPI = NULL;

#ifdef THREADS
/**
 * \var BermudaSpiThread
 * \brief Global SPI thread definition
 * 
 * Global structure for the SPI thread. Only used when compiled with multithreading
 * enabled.
 */
static THREAD *BermudaSpiThread = NULL;
#endif

/**
 * \fn BermudaSpiInit(SPI *spi)
 * \brief Initialise the Serial Peripheral Interface.
 * \param spi SPI instance
 * 
 * The parameter <i>spi</i> has to be allocated before passed to this function.
 * When this function returns the SPI is ready to use.
 */
int BermudaSpiHardwareInit(SPI *spi)
{
        unsigned char ints = 0;
        BermudaSafeCli(&ints);
        if(NULL == spi)
        {
                sei();
                return -1;
        }
        else
                BermudaSPI = spi;
        
        BermudaSetupSpiRegs(spi);
        BermudaSetSckPrescaler(spi, B11);
        
        BermudaSpiSetSckMode(spi, 0);
        BermudaSetSpiBitOrder(spi, 0); /* MSB first */
        spi->transact = &BermudaSpiTransmit;
        
#ifdef THREADS
        BermudaAttatchSpiIRQ(spi);
        BermudaThreadCreate("BermudaSpiThread", &BermudaSpiThread, BermudaSPI,
                                128);
#endif
        BermudaSetMasterSpi(spi);
        BermudaSpiEnable(spi);
        
        BermudaIntsRestore(ints);
        return 0;
}

/**
 * \fn BermudaSetMasterSpi(SPI *spi)
 * \brief Enable master mode.
 * \param spi The SPI structure to enable master mode on.
 * 
 * This function puts the SPI interface in master mode
 */
void BermudaSetMasterSpi(SPI *spi)
{
        BermudaSetSpiMode(spi, SPI_MASTER);
}

/**
 * \fn BermudaSetSlaveSpi(SPI *spi)
 * \brief Enable slave mode.
 * \param spi The SPI structure to enable slave mode on.
 * 
 * This function puts the SPI interface in slave mode
 */
void BermudaSetSlaveSpi(SPI *spi)
{
        BermudaSetSpiMode(spi, SPI_SLAVE);
}

/**
  * \fn BermudaSpiTransmitBuf(SPI *spi, void *data, size_t len)
  * \brief Transmit an array.
  * \param spi The SPI to use.
  * \param data The data to sent.
  * \param len The length of <i>data</i>.
  * \return [0] on success, -1 otherwise.
  *
  * This function transmits an array of data over the SPI. It should only
  * be called when the SPI is in master mode. When the SPI is not in master
  * mode and this function is called, it will return -2.
  */
int BermudaSpiTransmitBuf(SPI *spi, void *data, size_t len)
{
        if(NULL == spi || NULL == data)
                return -1;
        
        int i = 0;
        for(; i < len; i++)
                BermudaSpiTxByte(spi, ((unsigned char*)data)[i]);
        
        return 0;
}

/**
  * \fn BermudaSpiTransmit(SPI *spi, unsigned char data)
  * \brief Transmit one byte over the SPI.
  * \param spi The SPI to use.
  * \param data The data byte to sent.
  * \return The data sent from the slave.
  *
  * This function transmits one data byte over the given SPI.
  */
unsigned char BermudaSpiTransmit(SPI *spi, unsigned char data)
{
        return BermudaSpiTxByte(spi, data);
}

/**
 * \fn BermudaSpiEnable(SPI *spi)
 * \brief Enable the SPI.
 * \param spi The SPI to enable.
 * 
 * This function enables the given SPI interface.
 */
PRIVATE inline void BermudaSpiEnable(SPI *spi)
{
        spb(*spi->spcr, SPE);
}

/**
 * \fn BermudaSpiDisable(SPI *spi)
 * \brief Enable the SPI.
 * \param spi The SPI to disable.
 * 
 * This function disables the given SPI interface.
 */
PRIVATE inline void BermudaSpiDisable(SPI *spi)
{
        cpb(*spi->spcr, SPE);
}

/**
 * \fn BermudaSetupSpiRegs(SPI *spi)
 * \brief Setup I/O registers.
 * \param spi The SPI to setup registers for.
 * 
 * This function links the SPI structure to the register addresses.
 */
PRIVATE WEAK void BermudaSetupSpiRegs(SPI *spi)
{
        spi->spcr = &BermudaGetSPCR();
        spi->spsr = &BermudaGetSPSR();
        spi->spdr = &BermudaGetSPDR();
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
PRIVATE WEAK int BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler)
{
        unsigned char X2 = (prescaler & 0x1);
        prescaler &= ~(0x1);
        
        if(BermudaIsPowerOfTwo(prescaler))
                return -1;
        
        spi->prescaler = prescaler;
        
        switch(prescaler)
        {
                case 2:
                        spb(*spi->spsr, SPI2X);
                        cpb(*spi->spcr, SPR0);
                        cpb(*spi->spcr, SPR1);
                        break;
                        
                case 4:
                        cpb(*spi->spsr, SPI2X);
                        cpb(*spi->spcr, SPR0);
                        cpb(*spi->spcr, SPR1);
                        break;
                        
                case 8:
                        spb(*spi->spsr, SPI2X);
                        spb(*spi->spcr, SPR0);
                        cpb(*spi->spcr, SPR1);
                        break;
                        
                case 16:
                        cpb(*spi->spsr, SPI2X);
                        spb(*spi->spcr, SPR0);
                        cpb(*spi->spcr, SPR1);
                        break;
                        
                case 32:
                        spb(*spi->spsr, SPI2X);
                        cpb(*spi->spcr, SPR0);
                        spb(*spi->spcr, SPR1);
                        break;
                        
                case 64:
                        if(X2)
                        {
                                spb(*spi->spsr, SPI2X);
                                spb(*spi->spcr, SPR0);
                        }
                        
                        spb(*spi->spcr, SPR1);
                        break;
                        
                case 128:
                        cpb(*spi->spsr, SPI2X);
                        spb(*spi->spcr, SPR0);
                        spb(*spi->spcr, SPR1);
                        break;
                        
                default:
                        spi->prescaler = 0;
                        break;
        }
        
        return 0;
}

#else
PRIVATE WEAK void BermudaSetSckPrescaler(SPI *spi, unsigned char pres)
{
        *spi->spcr &= ~B11; // disable both prescaler bits
        *spi->spsr &= ~B1;  // disable spi2x bit
        
        *spi->spcr |= pres & B11; // set prescaler bits
        *spi->spsr |= (pres & B100) >> 2; // set spi2x bit
}
#endif

#ifdef __LAZY__
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
PRIVATE WEAK int BermudaSpiSetSckMode(SPI *spi, unsigned char mode)
{
        if(mode > 4)
                goto fail;
        if(BermudaIsPowerOfTwo(mode))
        {
                cpb(*spi->spcr, CPOL);
                cpb(*spi->spcr, CPHA);
                
                switch(mode)
                {                                
                        case 1:
                                spb(*spi->spcr, CPHA);
                                break;
                                
                        case 2:
                                spb(*spi->spcr, CPOL);
                                break;
                                
                        case 4:
                                spb(*spi->spcr, CPOL);
                                spb(*spi->spcr, CPHA);
                                break;
                                
                        default:
                                break;
                }
                return 0;
        }
        
        fail:
        return -1;
}
#else
/**
 * BermudaSpiSetSckMode(SPI *spi, unsigned char mode)
 * \brief Set the serial clock mode.
 * \param spi The SP interface.
 * \param mode The mode to apply to <i>spi</i>.
 * 
 * mode[0]: Represents the CPHA bit.
 * mode[1]: Represents the CPOL bit.
 */
PRIVATE WEAK void BermudaSpiSetSckMode(SPI *spi, unsigned char mode)
{
        *spi->spcr &= ~B1100; // results in spcr & 11110011
        *spi->spcr &= (mode & B11) << 2;
        return;
}
#endif

/**
 * \fn BermudaSetSpiMode(SPI *spi, spi_mode_t mode)
 * \brief Set the SPI operation mode.
 * \param spi The SPI to set the operation mode for.
 * \param mode Operation mode to set.
 * 
 * This function will put the SPI in either master or slave mode, depending on
 * the value of mode.
 */
PRIVATE WEAK void BermudaSetSpiMode(SPI *spi, spi_mode_t mode)
{
        if(SPI_MASTER == mode)
        {
                /*
                 * Set SCK, MOSI and SS as output and MISO as input
                 * The SS pin is be set HIGH
                 */
                BermudaSetPinMode(SCK, OUTPUT);
                BermudaSetPinMode(MOSI, OUTPUT);
                BermudaSetPinMode(SS, OUTPUT);

                BermudaDigitalPinWrite(SCK, LOW);
                BermudaDigitalPinWrite(MOSI, LOW);
                BermudaDigitalPinWrite(SS, HIGH);
                
                spb(*spi->spcr, MSTR);
        }
        else
        {
                /*
                 * Set the SS, SCK and MOSI as input and the MISO as output.
                 */
                BermudaSetPinMode(SCK, INPUT);
                BermudaSetPinMode(MOSI, INPUT);
                BermudaSetPinMode(SS, INPUT);
                BermudaSetPinMode(MISO, OUTPUT);

                BermudaDigitalPinWrite(MISO, LOW);
                
                cpb(*spi->spcr, MSTR);
        }
}

/**
  * \fn BermudaSpiTxByte(SPI *spi, unsigned char data)
  * \brief Transmit one byte over the SPI.
  * \param spi The SPI to use.
  * \param data The data byte to sent.
  * \return [0] on success, -1 otherwise.
  *
  * This function transmits one data byte over the given SPI.
  */
PRIVATE WEAK unsigned char BermudaSpiTxByte(SPI *spi, unsigned char data)
{
#ifdef __THREADS__
        BermudaThreadEnterIO(BermudaCurrentThread);
#endif
        *(spi->spdr) = data;
#ifndef THREADS
        /* wait for the transfer */
        while(!(*spi->spsr & BIT(SPIF)));
#else /* if THREADS */
        BermudaThreadSleep();
#endif
        unsigned char ret = *(spi->spdr);
#ifdef __THREADS__
        BermudaThreadExitIO(BermudaCurrentThread);
#endif
        return ret;
}

#ifdef THREADS
/**
 * \fn BermudaAttatchSpiIRQ(SPI *spi)
 * \brief Setup the SPI ISR.
 * \param spi The ISR will be attatched to this SPI descriptor.
 * 
 * This function enables this SPI interrupt flag. If the interrupts are enabled
 * globally, then the SPI ISR will be called when a SPI transfer is complete.
 */
PRIVATE inline void BermudaAttatchSpiIRQ(SPI *spi)
{
        spb(*spi->spcr, SPIE);
}

/**
 * \fn BermudaDetachSpiIRQ(SPI *spi)
 * \brief Disable the SPI ISR.
 * \param spi The ISR will be dettatched from this SPI descriptor.
 * 
 * This function disables the SPI interrupt flag. After calling, the SPI ISR will
 * not be called again by the SPI.
 */
PRIVATE inline void BermudaDetachSpiIRQ(SPI *spi)
{
        cpb(*spi->spcr, SPIE);
}
#endif

/**
 * \fn BermudaSetSpiBitOrder(SPI *spi, unsigned char order)
 * \brief The bit order can be configured using this function.
 * \param spi SPI descriptor.
 * \param order The bit order.
 * 
 * When order is not 0 the bit order will be LSB first, otherwise MSB first.
 */
PRIVATE inline void BermudaSetSpiBitOrder(SPI *spi, unsigned char order)
{
        if(order)
                spb(*spi->spcr, DORD);
        else
                cpb(*spi->spcr, DORD);
}

#ifdef THREADS
SIGNAL(SPI_STC_vect)
{
        return;
}

THREAD(BermudaSpiThread, data)
{
        while(1);
}
#endif /* THREADS */

#endif /* __SPI */
