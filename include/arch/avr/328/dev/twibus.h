/*
 *  BermudaOS - TWI bus
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

//! \file arch/avr/328/dev/twibus.h

#ifndef __TWI_BUS_H
#define __TWI_BUS_H

#include <bermuda.h>
#include <dev/twif.h>

#define TWI0 twibus0
#define TWI0_INIT BermdudaTwi0Init

// Master transmitter status bytes

/**
 * \brief Start has been sent.
 */
#define TWI_MT_START_ACK 0x8

/**
 * \brief Repeated start has been sent.
 */
#define TWI_MT_RSTART_ACK 0x10

/**
 * \brief Slave address has been sent and ACKed.
 */
#define TWI_MT_SLA_ACK 0x18

/**
 * \brief Slave address has been sent and NACKed.
 */
#define TWI_MT_SLA_NACK 0x20

/**
 * \brief Data has been sent and ACKed.
 */
#define TWI_MT_DATA_ACK 0x28

/**
 * \brief Data has been sent and NACKed.
 */
#define TWI_MT_DATA_NACK 0x30

/**
 * \brief Bus arbitration has been lost.
 * \warning Transmission has to be restarted or stopped!
 * 
 * The bus is lost in sending the SLA+W or by sending a data byte.
 */
#define TWI_MT_ARB_LOST 0x38

//  ********************************
//  * Master receiver              *
//  ********************************
 
/**
 * \brief Start has been sent.
 */
#define TWI_MR_START_ACK 0x8

/**
 * \brief Repeated start has been sent.
 */
#define TWI_MR_RSTART_ACK 0x10

/**
 * \brief Bus arbitration has been lost.
 * \warning Transmission has to be restarted or stopped!
 * 
 * The bus is lost in sending the SLA+W or by sending a data byte.
 */
#define TWI_MR_ARB_LOST 0x38

/**
 * \brief Slave address has been sent and ACKed.
 */
#define TWI_MR_SLA_ACK 0x40

/**
 * \brief Slave address has been sent and NACKed.
 */
#define TWI_MR_SLA_NACK 0x48

/**
 * \brief Data has been received and ACK is returned.
 */
#define TWI_MR_DATA_ACK 0x50

/**
 * \brief Data has been received and NACK is returned.
 */
#define TWI_MR_DATA_NACK 0x58

/**
 * \brief TWI hardware I/O.
 * 
 * Structure containing all hardware I/O registers.
 */
struct _twi_hw {
	volatile reg8_t twbr;  //!< TWI bit rate control register.
	volatile reg8_t twcr;  //!< TWI control register.
	volatile reg8_t twsr;  //!< TWI status register.
	volatile reg8_t twdr;  //!< TWI data register.
	volatile reg8_t twar;  //!< TWI (slave) address register
	volatile reg8_t twamr; //!< TWI (slave) address mask register.
};

/**
 * \typedef TWIHW
 * \brief Type definition of the HW I/O structure.
 */
typedef struct _twi_hw TWIHW;

__DECL

PUBLIC int BermudaTwiTransfer(TWIBUS *twi, const void *tx, void *rx, 
							  unsigned int tmo);

PRIVATE WEAK void BermudaTwiArbitrationLost(TWIBUS *twi);
__DECL_END

#endif
