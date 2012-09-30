/*
 *  BermudaOS - ATmega I2C private header
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

/**
 * \file include/dev/i2c/busses/atmega_priv.h ATmega private header.
 * \brief Header file for the I2C bus of the megaAVR architecture.
 */

#ifndef __I2C_ATMEGA_PRIV_H_
#define __I2C_ATMEGA_PRIV_H_

#include <stdlib.h>
#include <stdio.h>

#include <arch/io.h>

//  ********************************
//  * Master transmitter           *
//  ********************************

/**
 * \brief Start has been sent.
 */
#define I2C_MASTER_START 0x8

/**
 * \brief Repeated start has been sent.
 */
#define I2C_MASTER_REP_START 0x10

/**
 * \brief Slave address has been sent and ACKed.
 */
#define I2C_MT_SLA_ACK 0x18

/**
 * \brief Slave address has been sent and NACKed.
 */
#define I2C_MT_SLA_NACK 0x20

/**
 * \brief Data has been sent and ACKed.
 */
#define I2C_MT_DATA_ACK 0x28

/**
 * \brief Data has been sent and NACKed.
 */
#define I2C_MT_DATA_NACK 0x30

/**
 * \brief Bus arbitration has been lost.
 * \warning Transmission has to be restarted or stopped!
 * 
 * The bus is lost in sending the SLA+W or by sending a data byte.
 */
#define I2C_MASTER_ARB_LOST 0x38

//  ********************************
//  * Master receiver              *
//  ********************************

/**
 * \brief Slave address has been sent and ACKed.
 */
#define I2C_MR_SLA_ACK 0x40

/**
 * \brief Slave address has been sent and NACKed.
 */
#define I2C_MR_SLA_NACK 0x48

/**
 * \brief Data has been received and ACK is returned.
 */
#define I2C_MR_DATA_ACK 0x50

/**
 * \brief Data has been received and NACK is returned.
 */
#define I2C_MR_DATA_NACK 0x58

//  ********************************
//  * Slave receiver               *
//  ********************************

#define I2C_SR_SLAW_ACK 0x60 //!< Own slave address has been received and ACKed.

/**
 * \brief Lost arbitration as master.
 * 
 * Arbitration lost in SLA+R/W as Master; own SLA+W has been 
 * received; ACK has been returned.
 */
#define I2C_SR_SLAW_ARB_LOST 0x68
#define I2C_SR_GC_ACK 0x70 //!< General call received, ACK returned.

/**
 * \brief Lost arbitration as master.
 * 
 * Arbitration lost in SLA+R/W as Master; General call address has 
 * been received; ACK has been returned.
 */
#define I2C_SR_GC_ARB_LOST 0x78

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with own SLA+W; data has been received and an ACK is
 * returned.
 */
#define I2C_SR_SLAW_DATA_ACK 0x80

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with own SLA+W; data has been received and a NACK is
 * returned.
 */
#define I2C_SR_SLAW_DATA_NACK 0x88

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with a GC; data has been received and an ACK is
 * returned.
 */
#define I2C_SR_GC_DATA_ACK 0x90

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with a GC; data has been received and a NACK is
 * returned.
 */
#define I2C_SR_GC_DATA_NACK 0x98
#define I2C_SR_STOP 0xA0 //!< Stop or repeated start condition received.

//  ********************************
//  * Slave Transmitter            *
//  ********************************

#define I2C_ST_SLAR_ACK 0xA8 //!< Own SLA+R received, ack returned.
#define I2C_ST_ARB_LOST 0xB0 //!< Arbitration lost as master, own SLA+R received.
#define I2C_ST_DATA_ACK 0xB8 //!< Data sent successfuly, ACK received.
#define I2C_ST_DATA_NACK 0xC0 //!< Data has been sent, NACK received.
#define I2C_ST_LAST_DATA_ACK 0xC8 //!< The last data byte is transmitted, ACK received.

#define I2C_BUS_ERROR 0x0 //!< Generic bus error.

/**
 * \def TWBR
 * \brief TW bit rate register.
 * \see BermudaTwiCalcTWBR
 * 
 * Set the bit rate. Usually calculated using BermudaTwiCalcTWBR.
 */
#define TWBR  MEM_IO8(0xB8)

/**
 * \def TWCR
 * \brief TW control register.
 * \note Most operations on TWCR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Controls the Two Wire Interface.
 */
#define TWCR  MEM_IO8(0xBC)

/**
 * \def TWSR
 * \brief TW status register.
 * \note Most operations on TWSR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 */
#define TWSR  MEM_IO8(0xB9)

/**
 * \def TWDR
 * \brief TW data register.
 * \note All operations on TWDR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Shift register to receive and transmit data.
 */
#define TWDR  MEM_IO8(0xBB)

/**
 * \def TWAR
 * \brief TW SLA register.
 * \note Most operations on TWDR are done using BermudaTwIoctl.
 * \see BermudaTwIoctl
 * 
 * Shift register to receive and transmit data.
 */
#define TWAR  MEM_IO8(0xBA)

/**
 * \def TWAMR
 * \brief TW SLA mask register
 * 
 * Masks bits in the TWAR.
 */
#define TWAMR MEM_IO8(0xBD)


/* ---------------------- */

/**
 * \brief The I2C megaAVR private structure.
 * 
 * This structure contains private data, such as I/O registers, which should be
 * hidden from higher layers. It contains an I/O file for communcation purposes.
 */
struct i2c_atmega_priv
{
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +----------------------------------------------------------+\n
	 * | TWINT | TWEA | TWSTA | TWSTO | TWWC |&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;| 
	 * TWEN | TWIE |\n
	 * +----------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twcr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +---------------------------------------------------------+\n
	 * | TWS7 | TWS6 | TWS5 | TWS4 | TWS3 |&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	 * &nbsp;| TWPS1 | TWPS0 |\n
	 * +---------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twsr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +-------------------------------------------------------+\n
	 * | TWD7 | TWD6 | TWD5 | TWD4 | TWD3 | TWD2 | TWD1 | TWD0 |\n
	 * +-------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twdr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +-------------------------------------------------------+\n
	 * | TWB7 | TWB6 | TWB5 | TWB4 | TWB3 | TWB2 | TWB1 | TWB0 |\n
	 * +-------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twbr;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +--------------------------------------------------------+\n
	 * | TWA6 | TWA5 | TWA4 | TWA3 | TWA2 | TWA1 | TWA0 | TWGCE |\n
	 * +--------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twar;
	
	/**
	 * \brief TWCR is the ATmega I2C control register.
	 * \note See also the ATmega datasheet about this register.
	 * 
	 * Bit layout:\n<DFN>
	 * +-------------------------------------------------------------+\n
	 * | TWAM6 | TWAM5 | TWAM4 | TWAM3 | TWAM2 | TWAM1 | TWAM0 |&nbsp;&nbsp;
	 * &nbsp;&nbsp;&nbsp;&nbsp;|\n
	 * +-------------------------------------------------------------+\n
	 * </DFN>
	 */
	reg8_t twamr;
	
	/**
	 * \brief I/O file.
	 * 
	 * File which is responsible for communcation within the bus.
	 */
	FILE *file;
} __attribute__((packed));

#endif /* __I2C_ATMEGA_PRIV_H_ */