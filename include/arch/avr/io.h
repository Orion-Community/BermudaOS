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

/** \file include/arch/avr/io.h */

#ifndef __PORT_IO_H
#define __PORT_IO_H

#include <avr/io.h>
#include <avr/pgmspace.h>

extern unsigned long BermudaTimerGetSysTick();

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \fn BermudaMutexRelease(unsigned char *lock)
 * \brief Release the mutex lock from <i>lock</i>.
 * \param lock Lock pointer.
 * 
 * This function releases the lock from <i>lock</i> mutually exclusive.
 */
extern inline void BermudaMutexRelease(unsigned char *lock);

/**
 * \fn BermudaMutexEnter(unsigned char *lock)
 * \brief Enter locking state.
 * \param lock Lock pointer.
 * 
 * This function locks a variable mutually exclusive.
 */
extern void BermudaMutexEnter(unsigned char *lock);

/**
 * \def BermudaEnterCritical
 * \brief Enter IO safe state.
 * \note Disables interrupts.
 * \warning System might get corrupted when BermudaExitCritical is not called.
 */
#define BermudaEnterCritical() __asm__ __volatile__( \
                               "in __tmp_reg__, __SREG__" "\n\t" \
                               "cli"                      "\n\t" \
                               "push __tmp_reg__"         "\n\t")
                               
/**
 * \def BermudaExitCritical
 * \brief Leave IO safe state.
 * \note Restores interrupts to saved state.
 * \warning Do not call without calling BermudaEnterCritical before.
 */
#define BermudaExitCritical() __asm__ __volatile__( \
                              "pop __tmp_reg__"           "\n\t" \
                              "out __SREG__, __tmp_reg__" "\n\t")

/**
 * \def BermudaInterruptsSave
 * \brief Save interrupt settings.
 * 
 * The interrupt settings will be saved on the stack.
 */
#define BermudaInterruptsSave() __asm__ __volatile__( \
"in __tmp_reg__, __SREG__" "\n\t" \
"push __tmp_reg__")

/**
 * \def BermudaInterruptsRestore
 * \brief Restore the interrupts settings.
 * \warning Only call after calling BermudaInterruptsSave.
 * \note Please keep in mind that BermudaInterruptsSave will push the IRQ settings
 *       on the stack.
 * \see BermudaInterruptsSave
 * 
 * Pop the saved interrupt settings form the stack.
 */
#define BermudaInterruptsRestore() __asm__ __volatile__( \
"pop __tmp_reg__" "\n\t" \
"out __SREG__, __tmp_reg__")

#define spb(port, bit) (port |= (1<<bit))
#define cpb(port, bit) (port &= ~(1<<bit))

#define ROM __attribute__((progmem))

/* memory io functions */
#define MEM_IO8(addr) (*(volatile unsigned char*)(addr))
#define MEM_IO16(addr) (*(volatile unsigned short*)(addr))

#define IO_OFFSET 0x20
#define SFR_IO8(addr) MEM_IO8((addr+IO_OFFSET))

/* pin defs */
#define PIN_NOT_AVAILABLE 0

#define INPUT 0x0
#define OUTPUT 0x1

#define LOW 0x0
#define HIGH 0x1

extern const unsigned short ROM BermudaPortToOutput[];
extern const unsigned short ROM BermudaPortToInput[];
extern const unsigned short ROM BermudaPortToMode[];
extern const unsigned char ROM  BermudaPinToPort[];
extern const unsigned char ROM BermudaPinToMask[];

extern inline unsigned char BermudaReadPGMByte(unsigned short);
extern inline unsigned short BermudaReadPGMWord(unsigned short);

extern void BermudaSetPinMode(unsigned char pin, unsigned char mode);
extern void BermudaDigitalPinWrite(unsigned char pin, unsigned char value);
extern unsigned char BermudaDigitalPinRead(unsigned char pin);

#define BermudaGetIOPort(pin) BermudaReadPGMByte((unsigned short) \
                                BermudaPinToPort+(pin))
#define BermudaGetIOMask(pin) BermudaReadPGMByte((unsigned short) \
                                BermudaPinToMask+(pin))
#define BermudaGetIOMode(port)  ((volatile unsigned char*)pgm_read_word( \
                                BermudaPortToMode+(port)))
#define BermudaGetOuputReg(port) ((volatile unsigned char*)pgm_read_word( \
                                BermudaPortToOutput+(port)))
#define BermudaGetInputReg(port) ((volatile unsigned char*)pgm_read_word( \
                                BermudaPortToInput+(port)))

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
        #include <arch/avr/328/io.h>
        #include <arch/avr/328/chip/adc.h>
        #include <arch/avr/328/chip/uart.h>
		#include <arch/avr/328/dev/twibus.h>
#endif

#ifdef __ARDUINO__
        #include <arch/avr/arduino/io.h>
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __PORT_IO_H */
