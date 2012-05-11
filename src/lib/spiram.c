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
#include <util/delay.h>

PRIVATE WEAK SPI *spi = NULL;
PRIVATE WEAK unsigned char _current_mode = 0xff;

void BermudaSpiRamEnable()
{
        BermudaSpiStart(SS);
}

void BermudaSpiRamDisable()
{
        BermudaSpiStop(SS);
}

/**
 * \fn BermudaSpiRamInit()
 * \brief Initialise the SPI ram.
 * \todo Use high level SPI interface.
 * \todo Add init check.
 * 
 * Initialise the SPI communication to the SPI SRAM chip.
 */
void BermudaSpiRamInit()
{
        SPI *spiram = BermudaSpiGetInterface();
        if(NULL == spiram)
                return;

        BermudaSpiRamDisable();
        spi = spiram;
        return;
}

void BermudaSpiRamWriteByte(unsigned int address, unsigned char byte)
{
        BermudaSpiRamSetMode(SPI_RAM_BYTE);
        BermudaSpiRamEnable();
        spi->transact(spi, WRDA);
        spi->transact(spi, (unsigned char)(address>>8));
        spi->transact(spi, (unsigned char)(address & 0xff));
        spi->transact(spi, byte);
        BermudaSpiRamDisable();
        return;
}

unsigned char BermudaSpiRamReadByte(unsigned int address)
{
        unsigned char byte = 0;
        BermudaSpiRamSetMode(SPI_RAM_BYTE);

        BermudaSpiRamEnable();
        spi->transact(spi, RDDA);
        spi->transact(spi, (unsigned char)(address>>8));
        spi->transact(spi, (unsigned char)(address & 0xff));
        byte = spi->transact(spi, 0xff);
        BermudaSpiRamDisable();
        return byte;
}

void BermudaSpiRamSetMode(spiram_t mode)
{
        if(mode <= SPI_RAM_BUF)
        {
                unsigned char status = HOLD;
                
                switch(mode)
                {
                        case SPI_RAM_BYTE:
                                break;
                                
                        case SPI_RAM_PAGE:
                                status |= 0x80;
                                break;
                                
                        case SPI_RAM_BUF:
                                status |= 0x40;
                                break;

                        default:
                                status = 0;
                                break;
                }
                
                BermudaSpiRamEnable();
                spi->transact(spi, WRSR);
                spi->transact(spi, status);
                BermudaSpiRamDisable();
        }
}
#endif /* __SPI__ && __SPIRAM__*/
