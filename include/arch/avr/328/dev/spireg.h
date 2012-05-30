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

#ifndef __328SPI_REG_H
#define __328SPI_REG_H

#include <bermuda.h>

#include <arch/avr/io.h>

/**
 * \def SPI_DDR
 * \brief DDR for hardware SPI.
 */
#define SPI_DDR (*(AvrIO->ddrb))

/**
 * \def SPI_PORT
 * \brief SPI I/O port.
 */
#define SPI_PORT (*(AvrIO->portb))

/**
 * \def SPI_CTRL
 * \brief SPI control register
 */
#define SPI_CTRL    SFR_IO8(0x2C)

/**
 * \def SPI_STATUS
 * \brief SPI statud register
 */
#define SPI_STATUS  SFR_IO8(0x2D)

/**
 * \def SPI_DATA
 * \brief SPI data register
 */
#define SPI_DATA    SFR_IO8(0x2E)

/**
 * \def SPI_MAX_DIV
 * \brief Maximum SPI clock divider for the AVR 328.
 */
#define SPI_MAX_DIV 128

/**
 * \def SPI_MODE_SHIFT
 * \brief Bit location of the SPI mode bits.
 */
#define SPI_MODE_SHIFT 2

/**
 * \def SPI_SCK
 * \brief SCK pin.
 */
#define SPI_SCK  5

/**
 * \def SPI_MISO
 * \brief MISO pin.
 */
#define SPI_MISO 4

/**
 * \def SPI_MOSI
 * \brief MOSI pin.
 */
#define SPI_MOSI 3

/**
 * \def SPI_SS
 * \brief SS pin.
 */
#define SPI_SS   2

/**
 * \def SPI_MAX_PRES
 * \brief Lowest possible prescaler.
 * \note SPI_DEFAULT_PRES is suggested.
 * \see SPI_DEFAULT_PRES
 */
#define SPI_MAX_PRES 128

/**
 * \def SPI_DEFAULT_PRES
 * \brief Suggested prescaler.
 */
#define SPI_DEFAULT_PRES 16

/**
 * \def SPI_MASTER_ENABLE
 * \brief Bit location of the SPI master enable bit in the SPCR
 */
#define SPI_MASTER_ENABLE BIT(4)

/**
 * \def SPI_IRQ_ENABLE
 * \brief Bit to enable the SPI IRQ.
 * \note The IE bit in the SREG register has to be set.
 */
#define SPI_IRQ_ENABLE BIT(7)

/**
 * \def SPI_ENABLE
 * \brief Bit to enable the SPI globally.
 */
#define SPI_ENABLE BIT(6)

#endif /* __328SPI_REG_H */