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

static TWIBUS *eeprom_bus = NULL;

PUBLIC void Bermuda24c02Init(TWIBUS *bus)
{
	eeprom_bus = bus;
}