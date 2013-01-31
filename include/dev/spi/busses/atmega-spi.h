/*
 *  BermudaOS - ATmega SPI
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

#ifndef __ATMEGA_SPI_H
#define __ATMEGA_SPI_H

#include <stdlib.h>

#include <dev/error.h>
#include <dev/dev.h>
#include <dev/spi.h>

/**
 * \brief ATmega SPI adapter.
 */
extern struct spi_adapter *atmega_spi_adapter;

/**
 * \brief ATmega SPI adapter.
 */
#define ATMEGA_SPI atmega_spi_adapter;

extern void atmega_spi_init();

#endif
