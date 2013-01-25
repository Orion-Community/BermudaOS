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

#ifndef __TWI_IO_H_
#define __TWI_IO_H_

/**
 * \def TWBR
 * \brief TW bit rate register.
 * \see BermudaTwiCalcTWBR
 * 
 * Set the bit rate. Usually calculated using BermudaTwiCalcTWBR.
 */
#define TWBR  MEM_IO8(0xB8)

/**
 * \def TWCR
 * \brief TW control register.
 * \note Most operations on TWCR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Controls the Two Wire Interface.
 */
#define TWCR  MEM_IO8(0xBC)

/**
 * \def TWSR
 * \brief TW status register.
 * \note Most operations on TWSR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 */
#define TWSR  MEM_IO8(0xB9)

/**
 * \def TWDR
 * \brief TW data register.
 * \note All operations on TWDR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Shift register to receive and transmit data.
 */
#define TWDR  MEM_IO8(0xBB)

/**
 * \def TWAR
 * \brief TW SLA register.
 * \note Most operations on TWDR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Shift register to receive and transmit data.
 */
#define TWAR  MEM_IO8(0xBA)

/**
 * \def TWAMR
 * \brief TW SLA mask register
 * 
 * Masks bits in the TWAR.
 */
#define TWAMR MEM_IO8(0xBD)

#endif