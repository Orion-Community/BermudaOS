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

//! \file src/dev/twif.c Generic Two Wire Interface.

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
	unsigned char sla = bus->sla & (~BIT(0));
	TW_IOCTL_MODE mode;
	
	switch(bus->status) {
		/* master start */
		case TWI_MASTER_REP_START:
		case TWI_MASTER_START:
			// TWI start signal has been sent. The interface is ready to sent
			// the slave address we want to address.
			bus->index = 0;
			if(bus->mode == TWI_MASTER_RECEIVER) {
				sla |= 1;
			}

			bus->twif->io(bus, TW_SENT_SLA, &sla);
			break;
		
		/* master transmitter */
		case TWI_MT_SLA_ACK: // slave ACKed SLA+W
		case TWI_MT_DATA_ACK: // slave ACKed data
			if(bus->index < bus->txlen) {
				bus->twif->io(bus, TW_SENT_DATA, (void*)&(bus->tx[bus->index]));
				bus->index++;
			}
			else if(bus->rxlen) {
				bus->mode = TWI_MASTER_RECEIVER;
				bus->twif->io(bus, TW_SENT_START, NULL); // sent repeated start
			}
			else { // end of transfer
				bus->error = E_SUCCESS;
				bus->twif->io(bus, TW_SENT_STOP, NULL);
#ifdef __EVENTS__
				BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
#endif
			}
			break;
			
		case TWI_MT_SLA_NACK: // slave NACKed slave address
		case TWI_MT_DATA_NACK: // slave NACKed data byte
		case TWI_MASTER_ARB_LOST: // lost bus control
			if(bus->status == TWI_MASTER_ARB_LOST) {
				mode = TW_RELEASE_BUS;
			}
			else {
				mode = TW_SENT_STOP;
			}
			bus->error = bus->status;
			bus->index = 0;
			bus->twif->io(bus, mode, NULL);
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
#endif
			break;
		
		/* master receiver */
		case TWI_MR_DATA_ACK: 
		case TWI_MR_SLA_ACK: // slave ACKed SLA+R
			break;
			
		case TWI_MR_SLA_NACK:
		case TWI_MR_DATA_NACK:
			if(bus->index < bus->rxlen) {
				bus->twif->io(bus, TW_READ_DATA, (void*)&(bus->rx[bus->index]));
			}
			
			bus->error = bus->status;
			bus->index = 0;
			bus->twif->io(bus, TW_SENT_STOP, NULL);
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
#endif
			break;

		/* slave receiver cases */
		case TWI_SR_SLAW_ACK:
		case TWI_SR_GC_ACK:
		case TWI_SR_GC_ARB_LOST:
		case TWI_SR_SLAW_ARB_LOST:
			if(bus->rxlen) {
				mode = TW_REPLY_ACK;
				bus->index = 0; // reset receive buffer.
			}
			else {
				mode = TW_REPLY_NACK;
				bus->error = bus->status;
#ifdef __EVENTS__
				BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
#endif
			}
			bus->twif->io(bus, mode, NULL);
			break;

		default:
			bus->error = E_GENERIC;
			bus->index = 0;
			bus->twif->io(bus, TW_RELEASE_BUS, NULL);
#ifdef __EVENTS__
			BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
#endif
			break;
	}
	
	return;
}
#endif /* __TWI__ */
