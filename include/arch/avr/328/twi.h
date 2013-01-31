/*
 *  BermudaOS - ATmega TWI registers
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

/**
 * \def I2C_PRES_1
 * \brief Prescaler of 1. 
 * 
 * This value has no effect on the prescaler value.
 */
#define I2C_PRES_1 B0

/**
 * \def I2C_PRES_4
 * \brief Prescaler value 4.
 */
#define I2C_PRES_4 B1

/**
 * \def I2C_PRES_16
 * \brief Prescaler value 16
 */
#define I2C_PRES_16 B10

/**
 * \def I2C_PRES_64
 * \brief Prescaler value 64
 */
#define I2C_PRES_64 B11

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
#define I2C_FRQ(x, n) \
	(F_CPU/(16+(2*x*n)))
	
#define I2C_CALC_TWBR(__freq, __pres) \
(F_CPU - (16*__freq)) / (2*__pres*__freq)
#endif
#endif