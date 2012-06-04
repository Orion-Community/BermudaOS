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

//! \file arch/avr/328/dev/twibus.c Hardware TWI bus controller.

#include <bermuda.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#include <dev/twif.h>
#include <arch/avr/328/dev/twibus.h>

static volatile void *twi0_mutex = SIGNALED;
static volatile void *twi0_queue = SIGNALED;

static TWIBUS *twibus0 = NULL;

/**
 * \brief Initialize TWI bus 0.
 * \param bus Bus structure pointer. Should not be NULL.
 */
PUBLIC void BermudaTwi0Init(TWIBUS *bus)
{
	if(bus == NULL) {
		return;
	}
	
	twibus0 = bus;
	bus->twif->transfer = &BermudaTwiTransfer;
	bus->mutex = &twi0_mutex;
	bus->queue = &twi0_queue;
}

/**
 * \brief Set the status register.
 * \param twi Bus to set the new status for.
 * \warning Should only be called from the TWI ISR.
 * \return The updated status value.
 * 
 * Updates the status regsiter in the TWI bus using the hardware status register.
 */
PUBLIC inline uint8_t BermudaTwiUpdateStatus(TWIBUS *twi)
{
	TWIHW *hwio = twi->hwio;
	twi->status = (*(hwio->twsr)) & B11;
	return twi->status;
}

/**
 * \brief Transfer data using the twi bus.
 * \param twi Used TWI bus.
 * \param tx Transmit buffer.
 * \param rx Receive buffer.
 * \param tmo Transfer waiting time-out.
 * \warning A TWI init routine should be called before using this function.
 * \see BermudaTwi0Init
 * 
 * Data is transfered or received using the TWI bus. The mode this function
 * uses depends of the TWIMODE setting in the TWIBUS structure.
 */
PUBLIC int BermudaTwiTransfer(twi, tx, rx, tmo)
TWIBUS       *twi;
const void   *tx;
void         *rx;
unsigned int tmo;
{
	int rc = -1;
	
	if((rc = BermudaEventWait((volatile THREAD**)twi->mutex, tmo)) == -1) {
		return rc;
	}

	// TODO: implement twi
	rc = BermudaEventSignal((volatile THREAD**)twi->mutex);
	return rc;
}

/**
 * \brief Do TWI I/O control.
 * \param bus The TWI bus.
 * \param mode I/O control mode.
 * \param conf I/O configuration.
 * \return 0 on success, -1 when the interface is not free. 1 is returned when
 *         this function is called with an invalid value is the <b>mode</b>
 *         parameter.
 * \see TW_IOCTL_MODE
 */
PRIVATE WEAK int BermudaTwIoctl(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf)
{
	TWIHW *hw = bus->hwio;
	unsigned char sla;
	int rc;
	
	if((rc = BermudaEventWait((volatile THREAD**)bus->mutex, TW_TMO)) == -1) {
		goto end;
	}
	rc = 0;
	
	switch(mode) {
		case TW_SET_RATE:
			*(hw->twbr) = *((unsigned char*)conf);
			break;
		case TW_GET_RATE:
			*((unsigned char*)conf) = *(hw->twbr);
			break;
		case TW_SET_SLA:
			sla = (*((unsigned char*)conf)) << 1; // shift out the GCRE bit
			*(hw->twar) = sla;
			break;
		case TW_GET_SLA:
			sla = (*(hw->twar)) >> 1; // shift for the GCRE bit
			*((unsigned char*)conf) = sla;
			break;
		default:
			rc = 1;
			break;
	}

	end:
	if(rc != -1) {
		BermudaEventSignal((volatile THREAD**)bus->mutex);
	}
	return rc;
}
