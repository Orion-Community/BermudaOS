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
#include <avr/interrupt.h>
#include <avr/io.h>
#include <arch/avr/io.h>

static SPI *BermudaSPI = NULL;

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
        sei();
        return 0;
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

SIGNAL(SPI_STC_vect)
{
        return;
}
