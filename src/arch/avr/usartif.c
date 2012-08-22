/*
 *  BermudaOS - Generic USART interface
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

//! \file src/arch/avr/usartif.c ATmega specific USART.

#if defined(__USART__) || defined(__DOXYGEN__)

#include <bermuda.h>

#include <arch/io.h>
#include <arch/usart.h>

#include <dev/usartif.h>
#include <sys/events/event.h>

/**
 * \brief After care of the USART transfers.
 * \param bus The bus which has done a transfer.
 * \param transtype The type of transfer which has been done.
 * \warning Should never be called by user applications!
 * 
 * This interrupt handler is called by hardware when there is a transfer done.
 */
PUBLIC __link void BermudaUsartISR(USARTBUS *bus, unsigned char transtype)
{
	switch(transtype) {
		case USART_TX:
			if(bus->tx_index < bus->tx_len) {
				bus->usartif->io(bus, USART_TX_DATA, (void*)&(bus->tx[bus->tx_index]));
				bus->tx_index++;
			}
			else {
				bus->usartif->io(bus, USART_STOP, NULL);
				bus->tx_len = 0;
#ifdef __EVENTS__
				BermudaEventSignalFromISR((volatile THREAD**)bus->tx_queue);
#endif
			}
			break;
		
		case USART_RX:
			if(bus->rx_index < bus->rx_len) {
				bus->usartif->io(bus, USART_RX_DATA, (void*)&(bus->rx[bus->rx_index]));
				bus->rx_index++;
			}
			else {
				bus->rx_len = 0;
#ifdef __EVENTS__
				BermudaEventSignalFromISR((volatile THREAD**)bus->tx_queue);
#endif
			}
			break;
		
		default:
			break;
	}
}

/**
 * \brief Initiate a USART transfer.
 * \param bus Bus to use for the transfer.
 * \param tx Transmit buffer.
 * \param txlen Transmit buffer length.
 * \param rx Receive buffer.
 * \param rxlen Receive buffer length.
 * \param baud Desired baudrate.
 * \param tmo Time-out. Maximum time to wait for the transfer to complete.
 * \return An error code will be return. 0 for success -1 on failure (eg wrong
 *         parameters or transmission timed out.
 * The transfer will be started. Based on the parameters it will start a transmit
 * or receive operation. If both should be done it will do a transmit first.
 */
PUBLIC int BermudaUsartTransfer(bus, tx, txlen, rx, rxlen, baud, tmo)
USARTBUS *bus;
const void *tx;
unsigned int txlen;
void *rx;
unsigned int rxlen;
unsigned int baud;
unsigned int tmo;
{
	int rc = -1;
	USART_IOCTL_MODE mode;
	void *buffer = NULL;

	if(txlen == 0 && rxlen == 0) {
		return rc;
	}
#ifdef __EVENTS__
	if((rc = BermudaEventWait((volatile THREAD**)bus->mutex, tmo)) == -1) {
		return -1;
	}
#endif

	if(rxlen) {
		bus->rx = rx;
		bus->rx_len = rxlen;
		bus->rx_index = 0;
		mode = USART_RX_DATA;
		buffer = (void*)rx;
	}
	
	if(txlen) {
		bus->tx = tx;
		bus->tx_len = txlen;
		bus->tx_index = 1;
		mode = USART_TX_DATA;
		buffer = (void*)tx;
	}
	
	bus->usartif->io(bus, USART_START, NULL);
	bus->usartif->io(bus, mode, buffer);
#ifdef __EVENTS__
	rc = BermudaEventWaitNext((volatile THREAD**)bus->tx_queue, tmo);
#endif
	
#ifdef __EVENTS__
	BermudaEventSignal((volatile THREAD**)bus->mutex);

#endif
	return rc;
}
#endif
