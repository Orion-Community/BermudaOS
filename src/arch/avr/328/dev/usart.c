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

#include <bermuda.h>

#include <lib/binary.h>
#include <dev/usartif.h>

#include <sys/events/event.h>

#include <arch/avr/io.h>
#include <arch/avr/328/dev/uart.h>
#include <arch/avr/328/dev/usartreg.h>


//<< Private function declarations >>//
PRIVATE WEAK void BermudaUsartBusInit(USARTBUS *bus);

#ifdef __EVENTS__
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

/**
 * \var BermudaUART0
 * \brief Global USART 0 variable.
 *
 * Global definition of the first hardware USART.
 */
USARTBUS BermudaUART0;

static HW_USART hw_usart0 = {
	.ucsra = &UCSR0A,
	.ucsrb = &UCSR0B,
	.ucsrc = &UCSR0C,
	.ubrrl = &UBRR0L,
	.ubrrh = &UBRR0H,
	.udr   = &UDR0,
};

PUBLIC void BermudaUsart0Init()
{
	USARTBUS *bus = USART0;
	bus->mutex = (void*)usart_mutex;
	bus->queue = (void*)usart_queue;
	bus->io.hwio = &hw_usart0;
	
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
