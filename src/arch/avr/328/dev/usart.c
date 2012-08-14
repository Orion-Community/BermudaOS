/*
 *  BermudaOS - USART
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

#include <stdlib.h>
#include <stdio.h>
#include <bermuda.h>

#include <lib/binary.h>
#include <dev/usartif.h>

#include <arch/avr/io.h>
#include <arch/avr/328/dev/uart.h>
#include <arch/avr/328/dev/usartreg.h>

#ifdef ___EVENTS__
/**
 * \var usart_mutex
 * \brief Mutex to make transfers mutually exclusive.
 */
static volatile void *usart_mutex;

/**
 * \var usart_queue
 * \brief Transfer waiting queue.
 *
 * Threads waiting for a transfer are put in this queue.
 */
static volatile void *usart_queue = SIGNALED;
#endif

PUBLIC void BermudaUsart0Init()
{
	UBRR0H = UBRR0H_VALUE;
	UBRR0L = UBRR0L_VALUE;

#ifdef UART2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~(_BV(U2X0));
#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* sent data in packets of 8 bits */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Tx and Rx enable flags to 1 */

	// enable completion interrupts
}
