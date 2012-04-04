/*
 *  BermudaOS - SpiRam library
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

/** \file spiram.c */
#if defined(__SPI__) && defined(__SPIRAM__)

#include <stdlib.h>
#include <arch/spi.h>
#include <arch/io.h>
#include <lib/spiram.h>

PRIVATE WEAK SPI *spi = NULL;

void BermudaSpiRamInit()
{
        SPI *spiram = BermudaSpiGetInterface();
        if(NULL == spiram)
                return;

        if(!BermudaSpiIsInitialized(spiram))
                return;

        /* configure the hold pin on pin 2 */
        BermudaSetPinMode(PIN2, OUTPUT);
        BermudaDigitalPinWrite(PIN2, HIGH);
        spi = spiram;

        return;
}

void BermudaSpiRamWriteByte(unsigned int address, unsigned char byte)
{
        BermudaSpiRamSetMode(SPI_RAM_BYTE);
        spi->transact(spi, WRDA);
        spi->transact(spi, (unsigned char)(address>>8));
        spi->transact(spi, (unsigned char)(address & 0xff));
        spi->transact(spi, byte);
        return;
}

void BermudaSpiRamSetMode(spiram_t mode)
{
        if(mode <= SPI_RAM_BUF)
        {
                spi->transact(spi, RDSR);
                unsigned char status = spi->transact(spi, 0xff);
                
                switch(mode)
                {
                        case SPI_RAM_BYTE:
                                status &= ~(3<<6);
                                break;
                                
                        case SPI_RAM_PAGE:
                                status &= ~(3<<6);
                                status |= 0x80;
                                break;
                                
                        case SPI_RAM_BUF:
                                status &= ~(3<<6);
                                status |= 0x40;
                                break;

                        default:
                                status = 0;
                                break;
                }
                spi->transact(spi, WRSR);
                spi->transact(spi, status);
        }
}
#endif /* __SPI__ && __SPIRAM__*/
