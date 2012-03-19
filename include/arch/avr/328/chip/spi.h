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

#ifndef __SPI_H
#define __SPI_H

#include <arch/avr/io.h>
#include <bermuda.h>

typedef struct spi SPI;

struct spi
{
        char *name;
        unsigned char id;
        
        unsigned char (*read) (SPI*);
        void          (*write)(SPI*, unsigned char);
        unsigned char flags : 3; /*
                              * 0   -> set to one when initialized
                              * 1   -> ISR enabled
                              * 2   -> Sleep when waiting for transfer
                              */
        unsigned char prescaler;
        volatile unsigned char *spcr, *spsr, *spdr;
} __PACK__;

#define BermudaGetSPCR() SFR_IO8(0x2C)
#define BermudaGetSPSR() SFR_IO8(0x2D)
#define BermudaGetSPDR() SFR_IO8(0x2E)

/*
 * SPI defaults
 */
#define SPI_PRESCALER_DEFAULT 128

__DECL
extern unsigned char BermudaSpiRead(SPI *spi);
extern void BermudaSpiWrite(SPI *spi, unsigned char data);
extern int BermudaSpiInit(SPI *spi);
extern int BermudaSpiDestroy(SPI *spi);

PRIVATE void BermudaSetupSpiRegs(SPI *spi);
PRIVATE inline void BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler);
__DECL_END

#endif /* __SPI_H */
