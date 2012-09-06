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

//!< \file lib/24c02.h Serial EEPROM library

#ifndef __24C02EEPROM_H
#define __24C02EEPROM_H

#include <bermuda.h>
#include <dev/twif.h>

/**
 * \brief 24C02 base slave address.
 * \note Pulling an address line high will change the slave address.
 * 
 * The slave address of a 24C02 chip without any of the
 * address lines selected (pulled high).
 */
#define BASE_SLA_24C02 0xA0

#define SCL_FRQ_24C02 100000UL //! SCL frequency of 100.0KHz

extern void Bermuda24c02Init(char *devname);
extern int Bermuda24c02WriteByte(unsigned char addr, unsigned char data);
extern unsigned char Bermuda24c02ReadByte(unsigned char addr);

#endif /* __24C02EEPROM_H */