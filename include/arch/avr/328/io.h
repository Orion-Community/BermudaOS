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

#define BermudaGetPORTB() SFR_IO8(0x5)
#define BermudaGetAddressPORTB() ((volatile unsigned char*)&BermudaGetPORTB())
#define BermudaGetDDRB() SFR_IO8(0x4)
#define BermudaGetAddressDDRB() ((volatile unsigned char*)&BermudaGetDDRB())
#define BermudaGetPINB() SFR_IO8(0x3)
#define BermudaGetAddressPINB() ((volatile unsigned char*)&BermudaGetPINB())

#define BermudaGetPORTC() SFR_IO8(0x8)
#define BermudaGetAddressPORTC() ((volatile unsigned char*)&BermudaGetPORTC())
#define BermudaGetDDRC() SFR_IO8(0x7)
#define BermudaGetAddressDDRC() ((volatile unsigned char*)&BermudaGetDDRC())
#define BermudaGetPINC() SFR_IO8(0x6)
#define BermudaGetAddressPINC() ((volatile unsigned char*)&BermudaGetPINC())

#define BermudaGetPORTD() SFR_IO8(0xB)
#define BermudaGetAddressPORTD() ((volatile unsigned char*)&BermudaGetPORTD())
#define BermudaGetDDRD() SFR_IO8(0xA)
#define BermudaGetAddressDDRD() ((volatile unsigned char*)&BermudaGetDDRD())
#define BermudaGetPIND() SFR_IO8(0x9)
#define BermudaGetAddressPIND() ((volatile unsigned char*)&BermudaGetPIND())

#define BermudaGetSREG() SFR_IO8(0x3F)
#define BermudaGetAddressSREG() ((volatile unsigned char *)&BermudaGetSREG())

struct avr_io
{
        volatile unsigned char *portb, *portc, *portd;
        volatile unsigned char *pinb,  *pinc,  *pind;
        volatile unsigned char *ddrb,  *ddrc,  *ddrd;
        volatile unsigned char *sreg;
} __attribute__((packed));

extern const struct avr_io io;
#define BermudaGetAvrIO() (&io)
#define AvrIO BermudaGetAvrIO()
#ifdef __cplusplus
}
#endif
#endif /* __IO328_H */