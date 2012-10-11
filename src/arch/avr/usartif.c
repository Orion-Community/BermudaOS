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
