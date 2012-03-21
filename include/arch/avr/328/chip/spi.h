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

#ifdef __SPI__
#ifndef __SPI_H
#define __SPI_H

#include <arch/avr/io.h>
#include <bermuda.h>

#define SPI_INIT  0
#define SPI_IRQ   1
#define SPI_SLEEP 2
#define SPI_MODE  3

#define SPI_POLARITY_RISING 0
#define SPI_POLARITY_FALLING 1

#define SPI_CLOCK_PHASE_SAMPLE 0
#define SPI_CLOCK_PHASE_SETUP  1

typedef struct spi SPI;

struct spi
{
        char *name;
        unsigned char id;
        
        unsigned char (*read) (SPI*);
        void          (*write)(SPI*, unsigned char);
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
extern unsigned char BermudaSpiRead(SPI *spi);
extern void BermudaSpiWrite(SPI *spi, unsigned char data);
extern int BermudaSpiInit(SPI *spi);
extern int BermudaSpiDestroy(SPI *spi);
extern void BermudaSetMasterSpi(SPI *spi);
extern void BermudaSetSlaveSpi(SPI *spi);

PRIVATE WEAK void BermudaSetupSpiRegs(SPI *spi);
PRIVATE WEAK void BermudaSetSpiMode(SPI *spi, spi_mode_t mode);
PRIVATE WEAK inline int BermudaSetSpiClockMode(SPI *spi, unsigned char mode);
PRIVATE WEAK inline int BermudaUnsetSpiClockMode(SPI *spi, unsigned char mode);
PRIVATE WEAK inline void BermudaSetSpiBitOrder(SPI *spi, unsigned char order);
PRIVATE WEAK inline int BermudaSetSckPrescaler(SPI *spi, unsigned char prescaler);
PRIVATE WEAK inline void BermudaSpiEnable(SPI *spi);
PRIVATE WEAK inline void BermudaSpiDisable(SPI *spi);

#ifdef THREADS
PRIVATE WEAK inline void BermudaAttatchSpiIRQ(SPI *spi);
PRIVATE WEAK inline void BermudaDetachSpiIRQ(SPI *spi);
#endif
__DECL_END

#endif /* __SPI_H */
#endif /* __SPI   */
