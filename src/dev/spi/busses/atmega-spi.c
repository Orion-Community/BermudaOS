/*
 *  BermudaOS - ATmega SPI driver
 *  Copyright (C) 2012   Michel Megens <dev@michelmegens.net>
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

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/error.h>
#include <dev/spi.h>
#include <dev/spi-core.h>

#include <arch/io.h>

/* static functions */
static void atmega_spi_calc_freq(uint32_t hz);

static void atmega_spi_calc_freq(uint32_t hz)
{
	SPCR &= ~(SPR0 | SPR1);
	if(hz <= SPI_FRQ(128)) {
		SPCR |= SPR0 | SPR1;
	} else if(hz <= SPI_FRQ(64)) {
		SPCR |= SPR1;
	} else if(hz <= SPI_FRQ(16)) {
		SPCR |= SPR0;
	} /* else pres = 4 */
}

static int atmega_spi_transfer(struct spi_adapter *adapter, struct spi_shared_info *info)
{
	size_t idx;
	
	atmega_spi_calc_freq(info->freq);
	*(info->cs) &= ~info->cspin;
	
	for(idx = 0; idx < adapter->length; idx++) {
		SPDR = adapter->tx[idx];
		while(!(SPSR & BIT(SPIF)));
		adapter->rx[idx] = SPDR;
	}
	
	*(info->cs) |= info->cspin;
	return 0;
}
