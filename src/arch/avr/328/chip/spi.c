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

#include <bermuda.h>
#include <lib/binary.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <arch/avr/io.h>

static SPI *BermudaSPI = NULL;

#ifdef THREADS
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
        
#ifdef THREADS
        BermudaThreadCreate("BermudaSpiThread", &BermudaSpiThread, BermudaSPI,
                                128);
#endif
        sei();
        return 0;
}

void BermudaSetMasterSpi(SPI *spi)
{
        BermudaSetSpiMode(spi, SPI_MASTER);
}

void BermudaSetSlaveSpi(SPI *spi)
{
        BermudaSetSpiMode(spi, SPI_SLAVE);
}

PRIVATE void BermudaSetupSpiRegs(SPI *spi)
{
        spi->spcr = &BermudaGetSPCR();
        spi->spsr = &BermudaGetSPSR();
        spi->spdr = &BermudaGetSPDR();
}

PRIVATE inline void BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler)
{
        spi->prescaler = prescaler;
        
        switch(prescaler)
        {
                default:
                        spi->prescaler = 0;
                        break;
        }
}

PRIVATE inline int BermudaSetSpiClockMode(SPI *spi, unsigned int mode)
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

PRIVATE inline int BermudaUnsetSpiClockMode(SPI *spi, unsigned int mode)
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
PRIVATE inline void BermudaAttatchSpiIRQ(SPI *spi)
{
        spb(*spi->spcr, SPIE);
}

PRIVATE inline void BermudaDetachSpiIRQ(SPI *spi)
{
        cpb(*spi->spcr, SPIE);
}
#endif

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
