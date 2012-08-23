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

#include <bermuda.h>
#include <stdio.h>

#include <arch/usart.h>
#include <arch/avr/serialio.h>
#include <arch/avr/328/dev/usartreg.h>

// private functions
PRIVATE WEAK int BermudaUsartReadByte(FILE *stream);
PRIVATE WEAK int BermudaUsartWriteByte(char c, FILE *stream);

static FILE usart_in = { 0 };
static FILE usart_out = { 0 };

/**
 * \brief Setup the USART file streams used by functions such as printf.
 */
PUBLIC void BermudaUsartSetupStreams()
{
		fdev_setup_stream(&usart_out, &BermudaUsartWriteByte, NULL, _FDEV_SETUP_WRITE);
		fdev_setup_stream(&usart_in, NULL, &BermudaUsartReadByte, _FDEV_SETUP_READ);
		stdout = &usart_out;
		stdin  = &usart_in;
}

/**
 * \brief Writes a byte the serial bus.
 */
PRIVATE WEAK int BermudaUsartWriteByte(char c, FILE *stream)
{
	HW_USART *hw = BermudaUsartGetIO(USART0);
	
	USART0->usartif->io(USART0, USART_STOP, NULL);
	if(c == '\n') {
		BermudaUsartWriteByte('\r', stream);
	}

	while(( (*(hw->ucsra)) & BIT(UDRE0) ) == 0);
	(*(hw->udr)) = c;

	return 0;
}

/**
 * \brief Tries to read a byte from the serial bus.
 */
PRIVATE WEAK int BermudaUsartReadByte(FILE *stream)
{
	unsigned char c = 0;
	BermudaUsartListen(USART0, &c, 1, 9600, 500);
	return c;
}
