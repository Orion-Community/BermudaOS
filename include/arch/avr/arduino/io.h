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

#ifdef _AVR_PORTPINS_H_
#undef PIN0
#undef PIN1
#undef PIN2
#undef PIN3
#undef PIN4
#undef PIN5
#undef PIN6
#undef PIN7
#endif

#define DIGITAL_BASE_PIN 0
#define PIN0 DIGITAL_BASE_PIN+0
#define PIN1 DIGITAL_BASE_PIN+1
#define PIN2 DIGITAL_BASE_PIN+2
#define PIN3 DIGITAL_BASE_PIN+3
#define PIN4 DIGITAL_BASE_PIN+4
#define PIN5 DIGITAL_BASE_PIN+5
#define PIN6 DIGITAL_BASE_PIN+6
#define PIN7 DIGITAL_BASE_PIN+7
#define PIN8 DIGITAL_BASE_PIN+8
#define PIN9 DIGITAL_BASE_PIN+9
#define PIN10 DIGITAL_BASE_PIN+10
#define PIN11 DIGITAL_BASE_PIN+11
#define PIN12 DIGITAL_BASE_PIN+12
#define PIN13 DIGITAL_BASE_PIN+13

#if defined(__AVR_ATmega328P__)
#define SS   10
#define MOSI 11
#define MISO 12
#define SCK  13

#else
#error Other boards than the Arduino Uno boards are not supported yet!
#endif

extern void BermudaDigitalPinWrite(unsigned char pin, unsigned char value);
extern unsigned char BermudaDigitalPinRead(unsigned char pin);

#ifdef __cplusplus
}
#endif
#endif /* __UNO_IO_H */
