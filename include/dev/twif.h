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

//! \file dev/twif.h TWI interface

#ifndef __TWIF_H
#define __TWIF_H

#include <bermuda.h>

#include <dev/dev.h>
#include <lib/binary.h>

#define BERMUDA_TWI_RW_SHIFT 7

/**
 * \typedef TWIMODE
 * \brief Type definition of the TWI mode.
 * \see TWIBUS
 */
typedef enum {
	TWI_MASTER_TRANSMITTER, //!< Master transmit mode.
	TWI_MASTER_RECEIVER,    //!< Master receive mode.
	TWI_SLAVE_TRANSMITTER,  //!< Slave transmit mode.
	TWI_SLAVE_RECEIVER,     //!< Slave receive mode.
} TWIMODE;

/**
 * \struct _twif
 * \brief TWI communication interface.
 * \see _twibus
 * 
 * Structure defining the communication function for a TWIBUS.
 */
struct _twif {
	/**
	 * \brief Function pointer to the transfer function.
	 * \param bus Bus interface to use with the transfer.
	 * \param tx Transmit buffer.
	 * \param rx Receiving buffer.
	 * \param tmo Transfer waiting time-out.
	 * 
	 * Transfer data, depending on the set TWIMODE, over the TWI interface.
	 */
	int (*transfer)(struct _twif *bus, const void *tx, void *rx, unsigned int tmo);
};

/**
 * \struct _twibus
 * \brief TWI bus structure.
 * \see _twif
 *
 * Each different TWI bus has its own _twibus structure.
 */
struct _twibus {
	volatile void *queue;    //!< TWI transfer waiting queue.
	struct _twif *twif;      //!< TWI hardware communication interface.
	void *hwio;              //!< TWI hardware I/O registers.
	TWIMODE mode;            //!< TWI communication mode.
	uint8_t sla;             //!< Configured slave address + R/W bit.
	volatile uint8_t status; //!< Status of TWI after a transmission.
};

/**
 * \typedef TWIBUS
 * \brief Type definition of the TWI bus.
 */
typedef struct _twibus TWIBUS;

/**
 * \typedef TWIF
 * \brief Type definition of the TWI interface.
 */
typedef struct _twif TWIF;

/**
 * \brief Get the TWI status register.
 * \param twi TWI bus to get a status from.
 * 
 * Safely gets the status from the given TWI bus.
 */
static inline uint8_t BermudaTwiGetStatus(TWIBUS *twi)
{
	uint8_t ret = 0;
	
	BermudaEnterCritical();
	ret = twi->status;
	BermudaExitCritical();
	return ret;
}

extern inline uint8_t BermudaTwiUpdateStatus(TWIBUS *twi);

/**
 * \brief Set the slave address.
 * \param dev TWI device.
 * \param sla Slave address to set.
 * \param rm R/W bit.
 * \return 0 on success, -1 when the device is held locked by another thread.
 * 
 * Sets the slave address + the read/write bit.
 */
static inline int BermudaTwiSetSla(DEVICE *dev, uint8_t sla, uint8_t rw)
{
	sla &= rw << BERMUDA_TWI_RW_SHIFT;
	TWIBUS *bus = dev->data;
	
	if(BermudaDeviceIsLocked(dev)) {
		return -1;
	}
	else {
		bus->sla = sla;
		return 0;
	}
}

#endif
