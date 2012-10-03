/*
 *  BermudaOS - ISR definitions
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

#ifndef __AVR_INTERRUPTS_H
#define __AVR_INTERRUPTS_H

#ifdef SIGNAL
#undef SIGNAL
#endif

/**
 * \def ISR_ENTRY_ATTRIBS
 * \brief ISR attributes.
 * 
 * Function attributes passed to interrupt service handlers.
 */
#define ISR_ENTRY_ATTRIBS signal, used, externally_visible

/**
 * \def ISR_HANDLE_ATTRIBS
 * \brief ISR handle function attributes.
 * \see ISR
 * 
 * Function attributes which are used in ISR handles.
 */
#define ISR_HANDLE_ATTRIBS used, externally_visible

#define SIGNAL(vector) \
	void vector(void) __attribute__((ISR_ENTRY_ATTRIBS)); \
	void vector(void)

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)
#include <arch/avr/328/interrupts.h>
#endif

#endif /* __AVR_INTERRUPTS_H */
