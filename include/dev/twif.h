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

/**
 * \def BERMUDA_TWI_RW_SHIFT
 * \brief Bit location of the R/W bit.
 */
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
 * \typedef TW_IOCTL_MODE
 * \brief Type definition of I/O control modes for TWI.
 * \see BermudaTwIoctl
 */
typedef enum {
	TW_SET_RATE,   //!< Set the TWBR register.
	TW_SET_PRES,   //!< Set the SCL prescaler.
	TW_SET_SLA,    //!< Set new slave address.
	
	TW_RELEASE_BUS,//!< Release the bus.
	TW_GET_STATUS, //!< Get the status from hardware.
	
	TW_SENT_START, //!< Sent the TWI start condition.
	TW_SENT_SLA,   //!< Sent the slave address over the TWI bus.
	TW_SENT_DATA,  //!< Sent a data byte over the TWI bus.
	TW_SENT_STOP,  //!< Sent a stop condition.
	TW_READ_DATA,  //!< Read data from the bus.

} TW_IOCTL_MODE;

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
	int (*transfer)(TWIBUS *twi, const void *tx, unsigned int txlen,  
							  void *rx, unsigned int rxlen, unsigned char sla,
							  uint32_t frq, unsigned int tmo);
	int (*io)(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf);
	void (*isr)(TWIBUS* BUS);
};

/**
 * \struct _twibus
 * \brief TWI bus structure.
 * \see _twif
 *
 * Each different TWI bus has its own _twibus structure.
 */
struct _twibus {
	volatile void *mutex;    //!< TWI bus mutex.
	volatile void *queue;    //!< TWI transfer waiting queue.
	struct _twif *twif;      //!< TWI hardware communication interface.
	void *hwio;              //!< TWI hardware I/O registers.
	const unsigned char *tx; //!< TWI transmit buffer.
	uptr txlen;              //!< Length of the tx buffer.
	unsigned char *rx;       //!< TWI receive buffer.
	uptr rxlen;              //!< Length of the rx buffer.
	uptr index;              //!< Data buffer index.
	
	TWIMODE mode;            //!< TWI communication mode.
	uint8_t sla;             //!< Configured slave address + R/W bit.
	uint32_t freq;           //!< TWI transfer frequency in Hertz.
	
	unsigned char error;     //!< TWI error member.
	unsigned char status;    //!< TWI status
};

#ifdef __cplusplus
extern "C" {
#endif

extern inline uint8_t BermudaTwiUpdateStatus(TWIBUS *twi);
extern void BermudaTwISR(TWIBUS *bus);

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

#ifdef __cplusplus
}
#endif

#endif
