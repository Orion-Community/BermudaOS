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

#include <bermuda.h>
#include <sys/events/event.h>
#include <dev/twif.h>
#include <arch/twi.h>

/**
 * \brief Generic TWI interrupt handler.
 * \param bus TWI bus which raised the interrupt.
 * \warning Should only be called by hardware!
 * 
 * Generic handling of the TWI logic.
 */
PUBLIC void BermudaTwISR(TWIBUS *bus)
{
	switch(bus->status) {
		case TWI_REP_START:
		case TWI_START:
			// TWI start signal has been sent. The interface is ready to sent
			// the slave address we want to address.
			
			if(bus->mode == TWI_MASTER_RECEIVER) {
				bus->sla = (bus->sla << 1) | 1; // set the R bit
			}
			else {
				bus->sla <<= 1; // shift for W bit (0).
			}
			bus->twif->io(bus, TW_SENT_SLA, &(bus->sla));
			break;
			
		default:
			BermudaEventSignalFromISR( (volatile THREAD**)bus->queue);
			break;
	}
	
	return;
}