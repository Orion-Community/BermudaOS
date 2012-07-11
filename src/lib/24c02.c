/*
 *  BermudaOS - Serial EEPROM library
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

//!< \file lib/24c02.c Serial EEPROM library

#include <bermuda.h>

#include <dev/twif.h>
#include <lib/24c02.h>
#include <arch/twi.h>

static TWIBUS *eeprom_bus = NULL;

/**
 * \brief Initialize the 24C02 driver.
 * \param bus Two Wire bus to use.
 * \note It is very important that the bus passed to this routine
 *       is initialized.
 * 
 * When this routine is called, the driver is functional and ready to use.
 */
PUBLIC void Bermuda24c02Init(TWIBUS *bus)
{
	eeprom_bus = bus;
}

PUBLIC int Bermuda24c02WriteByte(unsigned char addr, unsigned char data)
{
	unsigned char tx[] = { addr, data };
	return eeprom_bus->twif->transfer(eeprom_bus, tx, 2, NULL, 0, 
		                              BASE_SLA_24C02, SCL_FRQ_24C02, 500);
}

PUBLIC unsigned char Bermuda24c02ReadByte(unsigned char addr)
{
	unsigned char tx = addr;
	unsigned char rx = 0;
	eeprom_bus->twif->transfer(eeprom_bus, &tx, 1, &rx, 1, BASE_SLA_24C02,
		                       SCL_FRQ_24C02, 500);
	return rx;
}
