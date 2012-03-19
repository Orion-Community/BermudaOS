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

#ifndef __PORT_IO_H
#define __PORT_IO_H

#include <avr/io.h>
#include <avr/pgmspace.h>

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
        #include <arch/avr/328/io.h>
        #include <arch/avr/328/chip/adc.h>
        #include <arch/avr/328/chip/uart.h>
        #include <arch/avr/328/chip/spi.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define spb(port, bit) (port |= (1<<bit))
#define cpb(port, bit) (port &= ~(1<<bit))

#define ROM __attribute__((progmem))

/* memory io functions */
#define MEM_IO8(addr) (*(volatile unsigned char*)(addr))
#define MEM_IO16(addr) (*(volatile unsigned short*)(addr))

#define IO_OFFSET 0x20
#define SFR_IO8(addr) MEM_IO8((addr)+IO_OFFSET)

/* pin defs */
#define PIN_NOT_AVAILABLE 0

#define INPUT 0x0
#define OUTPUT 0x1

extern const unsigned short ROM BermudaPortToOutput[];
extern const unsigned short ROM BermudaPortToInput[];
extern const unsigned short ROM BermudaPortToMode[];
extern const unsigned char ROM  BermudaPinToPort[];
extern const unsigned char ROM BermudaPinToMask[];

extern inline unsigned char BermudaReadPGMByte(unsigned short);
extern inline unsigned short BermudaReadPGMWord(unsigned short);

extern void BermudaSetPinMode(unsigned char pin, unsigned char mode);

#define BermudaGetIOPort(pin) BermudaReadPGMByte((unsigned short) \
                                BermudaPinToPort+(pin))
#define BermudaGetIOMask(pin) BermudaReadPGMByte((unsigned short) \
                                BermudaPinToMask+(pin))
#define BermudaGetIOMode(port)  ((volatile unsigned char*)pgm_read_word( \
                                BermudaPortToMode+(port)))

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __PORT_IO_H */
