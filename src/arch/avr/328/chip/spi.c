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

/** \file spi.c */

#include <bermuda.h>
#include <lib/binary.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <arch/avr/io.h>

/**
 * \var BermudaSPI
 * \brief Global SPI structure.
 * 
 * Global definition of the SPI. Initialised to NULL by default
 */
static SPI *BermudaSPI = NULL;

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
int BermudaSpiInit(SPI *spi)
{
        cli();
        if(NULL == spi)
        {
                sei();
                return -1;
        }
        else
                BermudaSPI = spi;
        
        BermudaSetupSpiRegs(spi);
        BermudaSetSckPrescaler(spi, SPI_PRESCALER_DEFAULT);
        BermudaSetMasterSpi(spi);
        
        BermudaUnsetSpiClockMode(spi, 1); /* LOW when idle */
        BermudaSetSpiClockMode(spi, 1); /* sample on trailing edge */
        BermudaSetSpiBitOrder(spi, 1); /* LSB first */
        spi->name = "spi0";
        spi->id = 0;
#ifdef THREADS
        BermudaAttatchSpiIRQ(spi);
        BermudaThreadCreate("BermudaSpiThread", &BermudaSpiThread, BermudaSPI,
                                128);
#endif
        BermudaSpiEnable(spi);
        
        sei();
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
 * \fn BermudaSpiEnable(SPI *spi)
 * \brief Enable the SPI.
 * \param spi The SPI to enable.
 * 
 * This function enables the given SPI interface.
 */
PRIVATE inline void BermudaSpiEnable(SPI *spi)
{
        spb(*spi->spcr, SPE);
        spi->flags |= (1<<SPI_INIT);
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
        spi->flags &= ~(1<<SPI_INIT);
}

/**
 * \fn BermudaSetupSpiRegs(SPI *spi)
 * \brief Setup I/O registers.
 * \param spi The SPI to setup registers for.
 * 
 * This function links the SPI structure to the register addresses.
 */
PRIVATE void BermudaSetupSpiRegs(SPI *spi)
{
        spi->spcr = &BermudaGetSPCR();
        spi->spsr = &BermudaGetSPSR();
        spi->spdr = &BermudaGetSPDR();
}

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
PRIVATE inline int BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler)
{
        unsigned char X2 = (prescaler & 0x1);
        prescaler &= ~(0x1);
        
        if(!BermudaIsPowerOfTwo(prescaler))
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
        
        spi->flags |= (X2 << 4);
        return 0;
}

/**
 * \fn BermudaSetSpiClockMode(SPI *spi, unsigned char mode)
 * \brief Set a clock mode.
 * \param spi SPI to set the mode for.
 * \param mode The mode to set.
 * \return -1 if the mode is not a power of two.
 * 
 * BIT 0 : When bit 0 is a logical 1 the leading edge will be falling and the
 *         SCK pin will be high on idle.
 * 
 * BIT 1 : When bit 1 is logical 1 the data will be sampled on the leading edge.
 */
PRIVATE inline int BermudaSetSpiClockMode(SPI *spi, unsigned char mode)
{
        if(!BermudaIsPowerOfTwo(mode))
        {
                switch(mode)
                {
                        case 1:
                                spb(*spi->spcr, CPOL);
                                break;
                        case 2:
                                spb(*spi->spcr, CPHA);
                                break;
                        default:
                                break;
                }
                return 0;
        }
        else
                return -1;
        
}

/**
 * \fn BermudaUnsetSpiClockMode(SPI *spi, unsigned char mode)
 * \brief Unset a clock mode.
 * \param spi SPI to clear the mode for.
 * \param mode The mode to clear.
 * \return -1 if the mode is not a power of two.
 * 
 * BIT 0 : When bit 0 is a logical 1 the leading edge will be rising and the
 *         SCK pin will be low on idle.
 * 
 * BIT 1 : When bit 1 is logical 1 the data will be sampled on the trailing edge.
 */
PRIVATE inline int BermudaUnsetSpiClockMode(SPI *spi, unsigned char mode)
{
        if(!BermudaIsPowerOfTwo(mode))
        {
                switch(mode)
                {
                        case 1:
                                cpb(*spi->spcr, CPOL);
                                break;
                        case 2:
                                cpb(*spi->spcr, CPHA);
                                break;
                        default:
                                break;
                }
                return 0;
        }
        else
                return -1;
}

/**
 * \fn BermudaSetSpiMode(SPI *spi, spi_mode_t mode)
 * \brief Set the SPI operation mode.
 * \param spi The SPI to set the operation mode for.
 * \param mode Operation mode to set.
 * 
 * This function will put the SPI in either master or slave mode, depending on
 * the value of mode.
 */
PRIVATE void BermudaSetSpiMode(SPI *spi, spi_mode_t mode)
{
        if(SPI_MASTER == mode)
        {
                spb(*spi->spcr, MSTR);
                spi->flags |= 1 << SPI_MODE;
        }
        else
        {
                cpb(*spi->spcr, MSTR);
                spi->flags &= ~( 1 << SPI_MODE);
        }
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
        spi->flags |= (1<<SPI_IRQ);
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
        spi->flags &= ~(1<<SPI_IRQ);
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
#endif
