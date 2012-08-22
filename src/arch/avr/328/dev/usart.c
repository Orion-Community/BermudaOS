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

//! \file src/arch/avr/328/dev/usart.c HW specific USART controller.

#if defined(__USART__) || defined(__DOXYGEN__)

#include <stdio.h>
#include <bermuda.h>

#include <lib/binary.h>
#include <dev/usartif.h>

#include <sys/events/event.h>

#include <arch/avr/io.h>
#include <arch/avr/interrupts.h>
#include <arch/avr/328/dev/usart.h>
#include <arch/avr/328/dev/usartreg.h>


//<< Private function declarations >>//
PRIVATE WEAK void BermudaUsartConfigBaud(USARTBUS *bus, unsigned short baud);
PRIVATE WEAK void BermudaUsartIoCtl(USARTBUS *bus, USART_IOCTL_MODE mode, void *arg);
PRIVATE WEAK int BermudaUsartWriteByte(char c, FILE *stream);
PRIVATE WEAK int BermudaUsartReadByte(FILE *stream);

#ifdef __EVENTS__
/**
 * \var usart_mutex
 * \brief Mutex to make transfers mutually exclusive.
 */
static volatile void *usart_mutex;

/**
 * \var usart_tx_queue
 * \brief Transmit waiting queue.
 *
 * Threads waiting for a transfer are put in this queue.
 */
static volatile void *usart_tx_queue = SIGNALED;

/**
 * \var usart_rx_queue
 * \brief Transmit waiting queue.
 *
 * Threads waiting for a transfer are put in this queue.
 */
static volatile void *usart_rx_queue = SIGNALED;
#endif

static FILE usart_in = { 0 };
static FILE usart_out = { 0 };

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

static USARTIF hw_usartif = {
	.io = BermudaUsartIoCtl,
	.isr = BermudaUsartISR,
};

/**
 * \brief Initialise USART0.
 * 
 * USART bus 0 will be initialised by this function.
 */
PUBLIC void BermudaUsart0Init()
{
	USARTBUS *bus = USART0;
#ifdef __EVENTS__
	bus->mutex = (void*)usart_mutex;
	bus->tx_queue = (void*)usart_tx_queue;
	bus->rx_queue = (void*)usart_rx_queue;
#endif
	bus->usartif = &hw_usartif;
	bus->io.hwio = &hw_usart0;
	HW_USART *hw = &hw_usart0;
        
	*(hw->ubrrh) = UBRR0H_VALUE;
	*(hw->ubrrl) = UBRR0L_VALUE;

#ifdef UART2X
	*(hw->ucsra) |= _BV(U2X0);
#else
	*(hw->ucsra) &= ~(_BV(U2X0));
#endif

	*(hw->ucsrc) = _BV(UCSZ01) | _BV(UCSZ00); /* sent data in packets of 8 bits */
	*(hw->ucsrb) = _BV(RXEN0) | _BV(TXEN0);   /* Tx and Rx enable flags to 1 */

	// enable completion interrupts
// 	BermudaEnterCritical();
// 	*(hw->ucsrc) |= BIT(RXCIE0) | BIT(TXCIE0);
// 	BermudaExitCritical();
}

/**
 * \brief I/O control function for the USART of the ATmega328.
 * \param bus The bus to control.
 * \param mode The I/O mode to set.
 * \param arg Argument which might be needed. May be NULL.
 * \warning The argument <i>arg</i> won't be checked for errors.
 * \todo Implement Rx and Tx data.
 */
PRIVATE WEAK void BermudaUsartIoCtl(USARTBUS *bus, USART_IOCTL_MODE mode, void *arg)
{
	HW_USART *hw = BermudaUsartGetIO(bus);
	
	switch(mode)
	{
		case USART_SET_BAUD:
			BermudaUsartConfigBaud(bus, *((unsigned short*)arg));
			break;
			
		case USART_TX_DATA:
			(*(hw->udr)) = *((volatile unsigned char*)arg);
			break;
			
		case USART_RX_DATA:
			*((volatile unsigned char*)arg) = (*(hw->udr));
			break;
		
		default:
			break;
	}
}

/**
 * \brief Configures the baudrate for the given USART bus.
 * \param bus Bus to configure.
 * \param baud Desired baudrate.
 * 
 * The function will determine if the bus should operate in USART 2x mode.
 */
PRIVATE WEAK void BermudaUsartConfigBaud(USARTBUS *bus, unsigned short baud)
{
	unsigned short ubrr = (F_CPU + 8 * baud) / (16 * baud) - 1;
	HW_USART *hw = (HW_USART*)(bus->io.hwio);
	
	if((100 * F_CPU) < (16 * (baud + 1)) * (100 * baud) - baud * BAUD_TOL)
	{
		ubrr = (F_CPU + 4 * baud) / (8 * baud) - 1;
		(*(hw->ucsra)) |= BIT(1);
	}
	
	// program the new rate
	(*(hw->ubrrl)) = baud & 0xFF;
	(*(hw->ubrrh)) = (baud >> 8) & 0xF;
}

/**
 * \brief Setup the USART file streams used by functions such as printf.
 * \todo Put this function in serialio.c
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
 * \todo Put this function in serialio.c
 */
PRIVATE WEAK int BermudaUsartWriteByte(char c, FILE *stream)
{
	HW_USART *hw = BermudaUsartGetIO(USART0);
	
	if(c == '\n') {
		BermudaUsartWriteByte('\r', stream);
	}

	while((UCSR0A & _BV(UDRE0)) == 0);
	(*(hw->udr)) = c;

	return 0;
}

/**
 * \brief Tries to read a byte from the serial bus.
 * \todo Put this function in serialio.c
 */
PRIVATE WEAK int BermudaUsartReadByte(FILE *stream)
{
	unsigned char c = 0;
	BermudaUsartTransfer(USART0, NULL, 0, &c, 1, 9600, 500);
	return c;
}

SIGNAL(USART_TX_STC_vect)
{
	USART0->usartif->isr(USART0, USART_TX);
}

SIGNAL(USART_RX_STC_vect)
{
	USART0->usartif->isr(USART0, USART_RX);
}
#endif
