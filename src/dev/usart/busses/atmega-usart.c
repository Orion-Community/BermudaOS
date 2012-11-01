/*
 *  BermudaOS - ATmega USART bus controller
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
#include <string.h>

#include <dev/dev.h>
#include <dev/usart/usart.h>
#include <dev/usart/busses/atmega_usart.h>

#include <fs/vfile.h>
#include <fs/vfs.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <arch/irq.h>

#include "atmega_priv.h"

//<< Private function declarations >>//
static void BermudaUsartConfigBaud(USARTBUS *bus, unsigned short baud);
static void BermudaUsartIoCtl(USARTBUS *bus, USART_IOCTL_MODE mode, void *arg);
static int BermudaUsartReadByte(FILE *stream);
static int BermudaUsartWriteByte(int c, FILE *stream);
static int usart_write(FILE *stream, const void *buff, size_t size);
static int usart_read(FILE *stream, void *buff, size_t size);
static void BermudaUsartISR(USARTBUS *bus, unsigned char transtype);
static int usart_open(char *name);

#ifdef __EVENTS__
/**
 * \var usart_mutex
 * \brief Mutex to make transfers mutually exclusive.
 */
static volatile void *usart_mutex = SIGNALED;

/**
 * \var usart_rx_queue
 * \brief Transmit waiting queue.
 *
 * Threads waiting for a transfer are put in this queue.
 */
static volatile void *usart_rx_queue = SIGNALED;
#endif

static FDEV_SETUP_STREAM(usart0_io, &usart_write, &usart_read, &BermudaUsartWriteByte,
						 &BermudaUsartReadByte, NULL /* flush */, "USART0", _FDEV_SETUP_RW,
						 USART0);

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
	.io = &BermudaUsartIoCtl,
	.close = NULL,
	.open = &usart_open,
};

/**
 * \brief Initialise USART0.
 * 
 * USART bus 0 will be initialised by this function.
 */
PUBLIC void BermudaUsart0Init()
{
	USARTBUS *bus = USART0;
	bus->tx = NULL; 
	bus->rx = NULL;
	bus->tx_len = 0;
	bus->rx_len = 0;
	bus->tx_index = 0;
	bus->rx_index = 0;
#ifdef __EVENTS__
	bus->mutex = (void*)&usart_mutex;
	bus->rx_queue = (void*)&usart_rx_queue;
#else
	bus->mutex = 0;
	bus->tx_queue = 1;
	bus->rx_queue = 1;
#endif
	bus->usartif = &hw_usartif;
	bus->io.hwio = &hw_usart0;
	HW_USART *hw = BermudaUsartGetIO(bus);
        
	*(hw->ubrrh) = UBRR0H_VALUE;
	*(hw->ubrrl) = UBRR0L_VALUE;

#ifdef UART2X
	*(hw->ucsra) |= _BV(U2X0);
#else
	*(hw->ucsra) &= ~(_BV(U2X0));
#endif

	*(hw->ucsrc) = _BV(UCSZ01) | _BV(UCSZ00); /* sent data in packets of 8 bits */
	*(hw->ucsrb) = _BV(TXEN0);   /* Tx and Rx enable flags to 1 */
}

/**
 * \brief I/O control function for the USART of the ATmega328.
 * \param bus The bus to control.
 * \param mode The I/O mode to set.
 * \param arg Argument which might be needed. May be NULL.
 * \warning The argument <i>arg</i> won't be checked for errors.
 * \todo Implement Rx and Tx data.
 */
static void BermudaUsartIoCtl(USARTBUS *bus, USART_IOCTL_MODE mode, void *arg)
{
	HW_USART *hw = BermudaUsartGetIO(bus);
	
	switch(mode)
	{
		case USART_SET_BAUD:
			BermudaUsartConfigBaud(bus, *((unsigned short*)arg));
			break;
			
		case USART_TX_ENABLE:
			(*(hw->ucsrb)) |= BIT(TXCIE0);
			break;
			
		case USART_TX_STOP:
			(*(hw->ucsrb)) &= ~BIT(TXCIE0);
			
		case USART_RX_ENABLE:
			(*(hw->ucsrb)) |= BIT(RXCIE0) | _BV(RXEN0);
			break;
			
		case USART_RX_STOP:
			(*(hw->ucsrb)) &= ~(BIT(RXCIE0) | _BV(RXEN0));
			break;
			
		case USART_TX_DATA:
			while(( (*(hw->ucsra)) & BIT(UDRE0) ) == 0);
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

static int usart_open(char *name)
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

/**
 * \brief Configures the baudrate for the given USART bus.
 * \param bus Bus to configure.
 * \param baud Desired baudrate.
 * 
 * The function will determine if the bus should operate in USART 2x mode.
 */
static void BermudaUsartConfigBaud(USARTBUS *bus, unsigned short baud)
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
 * \brief After care of the USART transfers.
 * \param bus The bus which has done a transfer.
 * \param transtype The type of transfer which has been done.
 * \warning Should never be called by user applications!
 * 
 * This interrupt handler is called by hardware when there is a transfer done.
 */
static __link void BermudaUsartISR(USARTBUS *bus, unsigned char transtype)
{
	switch(transtype) {
		case USART_TX:
			break;
		
		case USART_RX:
			if(bus->rx_len == 0) {
				return;
			}
			
			if(bus->rx_index + 1 < bus->rx_len) {
				bus->usartif->io(bus, USART_RX_DATA, (void*)&(bus->rx[bus->rx_index]));
				bus->rx_index++;
			}
			else {
				if(bus->rx_index < bus->rx_len) {
					bus->usartif->io(bus, USART_RX_DATA, (void*)&(bus->rx[bus->rx_index]));
				}
				bus->rx_len = 0;
#ifdef __EVENTS__
				BermudaEventSignalFromISR((volatile THREAD**)bus->rx_queue);
#else
				BermudaMutexRelease(&(bus->rx_queue));
#endif
			}
			break;
		
		default:
			break;
	}
}

SIGNAL(USART_RX_STC_vect)
{
	BermudaUsartISR(USART0, USART_RX);
	return;
}
