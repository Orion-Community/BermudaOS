/*
 *  BermudaOS - I/O
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

#ifndef __IO328_H
#define __IO328_H

#include <arch/avr/io.h>
#include <avr/io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BERMUDA_PORTB SFR_IO8(0x5)
#define BERMUDA_DDRB  SFR_IO8(0x4)
#define BERMUDA_PINB  SFR_IO8(0x3)

#define BERMUDA_PORTC SFR_IO8(0x8)
#define BERMUDA_DDRC  SFR_IO8(0x7)
#define BERMUDA_PINC  SFR_IO8(0x6)

#define BERMUDA_PORTD  SFR_IO8(0xB)
#define BERMUDA_DDRD   SFR_IO8(0xA)
#define BERMUDA_PIND   SFR_IO8(0x9)

#ifdef __cplusplus
}
#endif
#endif /* __IO328_H */