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

#include <bermuda.h>

#include <arch/io.h>
#include <arch/usart.h>
#include <arch/avr/serialio.h>

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
			if(bus->tx_len == 0) {
				return;
			}
				
			if(bus->tx_index + 1 < bus->tx_len) {
				bus->usartif->io(bus, USART_TX_DATA, (void*)&(bus->tx[bus->tx_index]));
				bus->tx_index++;
			}
			else {
				if(bus->tx_index < bus->tx_len) {
					bus->usartif->io(bus, USART_TX_DATA, (void*)&(bus->tx[bus->tx_index]));
				}
				bus->tx_len = 0;
#ifdef __EVENTS__
				BermudaEventSignalFromISR((volatile THREAD**)bus->tx_queue);
#else
				BermudaMutexRelease(&(bus->tx_queue));
#endif
			}
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
 * \todo Implement the receive interface.
 * 
 * The transfer will be started. Based on the parameters it will start a transmit
 * or receive operation. If both should be done it will do a transmit first.
 */
PUBLIC int BermudaUsartTransfer(bus, tx, txlen, baud)
USARTBUS *bus;
const void *tx;
unsigned int txlen;
unsigned int baud;
{
	unsigned int idx = 0;

	if(txlen == 0) {
		return -1;
	}
	
	for(; idx < txlen; idx++) {
		BermudaUsartWriteByte(((const uint8_t*)tx)[idx], NULL);
	}
	
	return 0;
}

#ifdef __EVENTS__
PUBLIC int BermudaUsartListen(bus, rx, rxlen, baud, tmo)
#else
PUBLIC int BermudaUsartListen(bus, rx, rxlen, baud)
#endif
USARTBUS *bus;
void *rx;
unsigned int rxlen;
unsigned int baud;
#ifdef __EVENTS__
unsigned int tmo;
#endif
{
	int rc;
	unsigned int idx = 0;
	bool done;
	
	bus->usartif->io(bus, USART_RX_ENABLE, NULL);
	do {
		bus->rx = &((uint8_t*)rx)[idx];
		bus->rx_len = 1;
		if((rc = BermudaEventWaitNext((volatile THREAD**)bus->rx_queue, tmo)) == -1) {
			return rc;
		} else {
			idx++;
		}
		done = (idx == rxlen);
	} while(!done);
	
	bus->usartif->io(bus, USART_RX_STOP, NULL);
	return rc;
}

