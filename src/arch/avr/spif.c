/*
 *  BermudaOS - Serial Peripheral Interface
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

#if defined(__SPI__) || defined(__DOXYGEN__)

#include <bermuda.h>

#include <arch/spi.h>
#include <arch/avr/spif.h>

#include <sys/events/event.h>

PUBLIC __link void BermudaSpiISR(SPIBUS *bus)
{
	if(bus->master_rx) {
		bus->ctrl->io(bus, SPI_READ_DATA, (void*)&(bus->master_rx[bus->master_index-1]));
	}
	if(bus->master_index < bus->master_len) {
		bus->ctrl->io(bus, SPI_WRITE_DATA, (void*)&(bus->master_tx[bus->master_index]));
		bus->master_index++;
	}
#ifdef __EVENTS__
	else {
		BermudaEventSignalFromISR((volatile THREAD**)bus->master_queue);
	}
#endif
}
#endif