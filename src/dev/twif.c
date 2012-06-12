/*
 *  BermudaOS - TWI interface
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

#if defined(__TWI__) || defined(__DOXYGEN__)

#include <bermuda.h>
#include <sys/events/event.h>
#include <dev/twif.h>
#include <arch/twi.h>

/**
 * \brief Generic TWI interrupt handler.
 * \param bus TWI bus which raised the interrupt.
 * \warning Should only be called by hardware!
 * \todo Master receiver, slave transmitter and slave receiver.
 * 
 * Generic handling of the TWI logic. It will first sent all data in the transmit
 * buffer, if present. Then it will receive data in the receive buffer, if a
 * Rx buffer address is configured.
 */
PUBLIC void BermudaTwISR(TWIBUS *bus)
{
	unsigned char sla = bus->sla << 1;
	TW_IOCTL_MODE mode;
	
	switch(bus->status) {
		case TWI_REP_START:
		case TWI_START:
			// TWI start signal has been sent. The interface is ready to sent
			// the slave address we want to address.
			
			if(bus->mode == TWI_MASTER_RECEIVER) {
				sla |= 1;
			}

			bus->twif->io(bus, TW_SENT_SLA, &sla);
			break;
			
		case TWI_MT_SLA_ACK:
		case TWI_MT_DATA_ACK:
			if(bus->index < bus->txlen) {
				bus->twif->io(bus, TW_SENT_DATA, (void*)&(bus->tx[bus->index]));
				bus->index++;
			}
			else if(bus->rxlen) {
				bus->mode = TWI_MASTER_RECEIVER;
				bus->index = 0;
				bus->twif->io(bus, TW_START, NULL);
			}
			else { // end of transfer
				bus->index = 0;
				bus->error = E_SUCCESS;
				bus->twif->io(bus, TW_SENT_STOP, NULL);
				BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
			}
			break;
			
		case TWI_MT_SLA_NACK:
		case TWI_MT_DATA_NACK:
		case TWI_MASTER_ARB_LOST:
			if(bus->status == TWI_MASTER_ARB_LOST) {
				mode = TW_RELEASE_BUS;
			}
			else {
				mode = TW_SENT_STOP;
			}
			bus->error = bus->status;
			bus->index = 0;
			bus->twif->io(bus, mode, NULL);
			BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
			break;
			
			
		default:
			bus->error = E_GENERIC;
			bus->index = 0;
			bus->twif->io(bus, TW_RELEASE_BUS, NULL);
			BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
			break;
	}
	
	return;
}
#endif /* __TWI__ */
