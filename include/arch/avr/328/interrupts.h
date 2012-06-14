/*
 *  BermudaOS - AVR ATmega328(P) ISR definitions
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

#ifndef __ATmega328_ISR_H
#define __ATmega328_ISR_H

#define signal_vect(num) __vector_ ## num

#ifdef _AVR_IOXXX_H_
#undef TIMER0_OVF_vect
#undef SPI_STC_vect
#undef TWI_vect

#undef sei
#undef cli
#endif

/* vectors */
#define TIMER0_OVF_vect signal_vect(16)
#define SPI_STC_vect signal_vect(17)
#define TWI_STC_vect signal_vect(24)

#define sei() __asm__ __volatile__ ("sei" ::: "memory")
#define cli() __asm__ __volatile__ ("cli" ::: "memory")

#endif /* __ATmega328_ISR_H */