/*
 *  BermudaOS - Serial I/O functions used by stdio
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

//! \file src/arch/avr/usart-dev.c Stdio backend functions.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arch/usart.h>
#include <arch/avr/serialio.h>
#include <arch/avr/328/dev/usartreg.h>

#include <sys/thread.h>
#include <sys/events/event.h>

// private functions
static int BermudaUsartReadByte(FILE *stream);
static int BermudaUsartWriteByte(int c, FILE *stream);
static int usart_write(FILE *stream, const void *buff, size_t size);
static int usart_read(FILE *stream, void *buff, size_t size);

static FDEV_SETUP_STREAM(usart0_io, &usart_write, &usart_read, &BermudaUsartWriteByte,
						 &BermudaUsartReadByte, NULL /* flush */, "USART0", _FDEV_SETUP_RW,
						 USART0);

/**
 * \brief Setup the USART file streams used by functions such as printf.
 * \see BermudaUsartWriteByte
 * \see BermudaUsartReadByte
 * 
 * Stdout will be redirected to BermudaUsartWriteByte. Stdin is redirected to
 * the function BermudaUsartReadByte.
 */
PUBLIC void BermudaUsartSetupStreams()
{
	stdout = &usart0_io;
	stdin  = &usart0_io;
	iob_add(&usart0_io);
}

PUBLIC int usart_open(char *name)
{
	int i = 3;
	for(; i < MAX_OPEN; i++) {
		if(!strcmp(__iob[i]->name, name)) {
			return i;
		}
	}
	return -1;
}

static int usart_write(FILE *stream, const void *buff, size_t size)
{
	size_t x = 0;
	for(; x < size; x++) {
		fputc(((uint8_t*)buff)[x], stream);
	}
	return 0;
}

static int usart_read(FILE *stream, void *buff, size_t size)
{
	size_t x = 0;
	
	for(; x < size; x++) {
		((uint8_t*)buff)[x] = stream->get(stream);
	}
	
	return 0;
}

/**
 * \brief Writes a byte the serial bus.
 * \note This function is used by <b>stdout</b>
 * 
 * Writes a single character (<i>c</i>) to the USART0 (hardware usart).
 */
static int BermudaUsartWriteByte(int c, FILE *stream)
{
	HW_USART *hw = BermudaUsartGetIO((USARTBUS*)stream->data);
	
	if(c == '\n') {
		BermudaUsartWriteByte('\r', stream);
	}

	(*(hw->udr)) = c;
	while(( (*(hw->ucsra)) & BIT(TXCn) ) == 0);
	(*(hw->ucsra)) |= BIT(TXCn);

	return c;
}

/**
 * \brief Tries to read a byte from the serial bus.
 * \note This function is used by stdin.
 * 
 * Waits for one character on USART0 (hardware usart) for 500 miliseconds.
 */
static int BermudaUsartReadByte(FILE *stream)
{
	unsigned char c = 0;
	USARTBUS *bus = stream->data;
	
	bus->rx_len = 1;
	bus->rx = &c;
	bus->rx_index = 0;
	
	bus->usartif->io(bus, USART_RX_ENABLE, NULL);
#ifdef __EVENTS__
	BermudaEventWaitNext((volatile THREAD**)bus->rx_queue, EVENT_WAIT_INFINITE);
#else
	bus->rx_queue = 1;
	BermudaMutexEnter(&(bus->rx_queue));
#endif
	
	bus->usartif->io(bus, USART_RX_STOP, NULL);
	return c;
}

