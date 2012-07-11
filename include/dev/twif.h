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
#define BERMUDA_TWI_RW_SHIFT 0

#define TWI_WRITE 0 //!< TWI SLA+W bit mask.
#define TWI_READ BIT(0) //!< TWI SLA+R bit mask.

/**
 * \typedef TWIMODE
 * \brief Type definition of the TWI mode.
 * \see TWIBUS
 */
typedef enum
{
	TWI_MASTER_TRANSMITTER, //!< Master transmit mode.
	TWI_MASTER_RECEIVER, //!< Master receive mode.
	TWI_SLAVE_TRANSMITTER, //!< Slave transmit mode.
	TWI_SLAVE_RECEIVER, //!< Slave receive mode.
} TWIMODE;

/**
 * \typedef TW_IOCTL_MODE
 * \brief Type definition of I/O control modes for TWI.
 * \see BermudaTwIoctl
 */
typedef enum
{
	TW_SET_RATE, //!< Set the TWBR register.
	TW_SET_PRES, //!< Set the SCL prescaler.
	TW_SET_SLA, //!< Set new slave address.

	TW_RELEASE_BUS, //!< Release the bus.
	TW_GET_STATUS, //!< Get the status from hardware.
	TW_ENABLE_INTERFACE, //!< Enables the TWI interface.
	TW_DISABLE_INTERFACE, //!< Disable the TWI interface.
	TW_BLOCK_INTERFACE, //!< Blocks the TWI interface.
	TW_SLAVE_LISTEN, //!< Listen for incoming slave requests.

	TW_SENT_START, //!< Sent the TWI start condition.
	TW_SENT_SLA, //!< Sent the slave address over the TWI bus.
	TW_SENT_DATA, //!< Sent a data byte over the TWI bus.
	TW_SENT_STOP, //!< Sent a stop condition.
	TW_READ_DATA, //!< Read data from the bus.

	TW_REPLY_ACK, //!< Used by slave to ACK to the calling master.
	TW_REPLY_NACK, //!< Used by slave to disable ACKing.

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

struct softio
{
	// generic io registers used in twi
	reg8_t io_in; //!< SCL/SDA input register.
	reg8_t io_out; //!< SLA/SDA output register.
	
	// pins
	unsigned char scl; //!< SCL pin.
	unsigned char sda; //!< SDA pin.
} __attribute__((packed));

/**
 * \struct _twif
 * \brief TWI communication interface.
 * \see _twibus
 * 
 * Structure defining the communication function for a TWIBUS.
 */
struct _twif
{
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
	/**
	 * \brief I/O control functions.
	 * \param bus TWI bus structure.
	 * \param mode I/O mode to set/get.
	 * \param conf I/O configuration.
	 * 
	 * Controls the I/O logic. Implemented by the TWI driver.
	 */
	int (*io)(TWIBUS *bus, TW_IOCTL_MODE mode, void *conf);

	/**
	 * \brief ISR handler.
	 * \param bus TWI bus.
	 * 
	 * The ISR handler which will handle the logic.
	 */
	void (*isr)(TWIBUS * bus);
} __attribute__((packed));

/**
 * \struct _twibus
 * \brief TWI bus structure.
 * \see _twif
 *
 * Each different TWI bus has its own _twibus structure.
 */
struct _twibus
{
#ifdef __EVENTS__
	volatile void *mutex; //!< TWI bus mutex.
	
	volatile void *master_queue; //!< TWI master waiting queue.
	volatile void *slave_queue;  //!< TWI slave waiting queue.
#elif __THREADS__
	mutex_t mutex;
	mutex_t queue;
#endif

	struct _twif *twif; //!< TWI hardware communication interface.
	union {
		void *hwio; //!< TWI hardware I/O registers.
		struct softio *softio; //!< Software TWI registers;
	} io;

	const unsigned char *master_tx; //!< TWI transmit buffer.
	uptr master_tx_len; //!< Length of the tx buffer.
	unsigned char *master_rx; //!< TWI receive buffer.
	uptr master_rx_len; //!< Length of the rx buffer.
	uptr master_index; //!< Data buffer index.

	const unsigned char *slave_tx; //!< TWI transmit buffer.
	uptr slave_tx_len; //!< Length of the tx buffer.
	unsigned char *slave_rx; //!< TWI receive buffer.
	uptr slave_rx_len; //!< Length of the rx buffer.
	uptr slave_index; //!< Data buffer index.

	TWIMODE mode; //!< TWI communication mode.
	uint8_t sla; //!< Configured slave address + R/W bit.
	uint32_t freq; //!< TWI transfer frequency in Hertz.

	unsigned char error; //!< TWI error member.
	unsigned char status; //!< TWI status
	bool busy; //!< When set to !0 the interface is busy.
} __attribute__((packed));

#ifdef __cplusplus
extern "C"
{
#endif

extern void BermudaTwISR(TWIBUS *bus);
extern int BermudaTwiSlaveListen(TWIBUS *bus, uptr *num, void *rx, uptr rxlen, 
	unsigned int tmo);
extern int BermudaTwiSlaveRespond(TWIBUS *bus, const void *tx, uptr txlen,
	unsigned int tmo);

#ifdef __cplusplus
}
#endif
#endif
