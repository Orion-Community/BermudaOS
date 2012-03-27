/*
 *  BermudaOS - Arduino specific I/O module
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

#ifndef __UNO_IO_H
#define __UNO_IO_H

#include <arch/avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PA 0
#define PB 1
#define PC 2
#define PD 3

#define ANALOG_BASE 14
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19

#if defined(__AVR_ATmega328P__)
#define SS   2
#define MOSI 3
#define MISO 4
#define SCK  5

#define SPI_POUT (*(BermudaGetAvrIO()->portb))
#define SPI_DDR  (*(BermudaGetAvrIO()->ddrb))
#else
#error Other arduino boards are not supported yet!
#endif


#ifdef __cplusplus
}
#endif
#endif /* __UNO_IO_H */