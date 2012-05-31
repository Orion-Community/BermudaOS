/*
 *  BermudaOS - Serial Peripheral Interface Bus
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

/** \file arch/avr/328/dev/spibus.h */

#ifndef __SPIBUS328_H_
#define __SPIBUS328_H_

#include <bermuda.h>
#include <dev/spibus.h>
#include <arch/avr/io.h>

/**
 * \def SPI0
 * \brief Definition of the SPI0 hardware initialization routine.
 */
#define SPI0 BermudaSPI0HarwareInit

/**
 * \struct _hwspi
 * \brief Hardware control structure.
 * 
 * Hardware I/O registers which are linked to an SPI node.
 */
struct _hwspi
{
        reg8_t spcr; //!< Control register.
        reg8_t spsr; //!< Status register.
        reg8_t spdr; //!< SPI data transfer register.
} __PACK__;

typedef struct _hwspi HWSPI;

#ifdef __cplusplus
extern "C" {
#endif

extern int BermudaSPI0HardwareInit(DEVICE *dev);

// internal functions
PRIVATE WEAK void BermudaSpiSetRate(SPIBUS *bus, uint32_t rate);
PRIVATE WEAK void BermudaSpiSetMode(SPIBUS *bus, unsigned char mode);
PRIVATE WEAK void select(SPIBUS *bus);
PRIVATE WEAK void deselect(SPIBUS *bus);
PRIVATE WEAK void BermudaSpiRateToHwBits(unsigned long *rate_select, unsigned char spi2x);
PRIVATE WEAK int BermudaHardwareSpiTransfer(SPIBUS *bus, const void *tx, void *rx, 
	                                          unsigned int len, unsigned int tmo);
PRIVATE WEAK int BermudaHardwareSpiWrite(SPIBUS* bus, unsigned char *data,
	                                     unsigned int tmo);

#ifdef __cplusplus
}
#endif
#endif /* __SPIBUS328_H_ */