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

//! \file arch/avr/328/dev/twireg.h

#ifndef __TWIREG_328_H
#define __TWIREG_328_H

#include <bermuda.h>
#include <arch/avr/io.h>

// Master transmitter status bytes

/**
 * \brief Start has been sent.
 */
#define TWI_MASTER_START 0x8

/**
 * \brief Repeated start has been sent.
 */
#define TWI_MASTER_REP_START 0x10

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
#define TWI_MASTER_ARB_LOST 0x38

//  ********************************
//  * Master receiver              *
//  ********************************

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

/*****************************/
/* Slave receiver            */
/*****************************/

#define TWI_SR_SLAW_ACK 0x60 //!< Own slave address has been received and ACKed.

/**
 * \brief Lost arbitration as master.
 * 
 * Arbitration lost in SLA+R/W as Master; own SLA+W has been 
 * received; ACK has been returned.
 */
#define TWI_SR_SLAW_ARB_LOST 0x68
#define TWI_SR_GC_ACK 0x70 //!< General call received, ACK returned.

/**
 * \brief Lost arbitration as master.
 * 
 * Arbitration lost in SLA+R/W as Master; General call address has 
 * been received; ACK has been returned.
 */
#define TWI_SR_GC_ARB_LOST 0x78

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with own SLA+W; data has been received and an ACK is
 * returned.
 */
#define TWI_SR_SLAW_DATA_ACK 0x80

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with own SLA+W; data has been received and a NACK is
 * returned.
 */
#define TWI_SR_SLAW_DATA_NACK 0x88

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with a GC; data has been received and an ACK is
 * returned.
 */
#define TWI_SR_GC_DATA_ACK 0x90

/**
 * \brief Data received ACK returned.
 * 
 * Previously addressed with a GC; data has been received and a NACK is
 * returned.
 */
#define TWI_SR_GC_DATA_NACK 0x98
#define TWI_SR_STOP 0xA0 //!< Stop or repeated start condition received.

//  ********************************
//  * Slave Transmitter            *
//  ********************************

#define TWI_ST_SLAR_ACK 0xA8 //!< Own SLA+R received, ack returned.
#define TWI_ST_ARB_LOST 0xB0 //!< Arbitration lost as master, own SLA+R received.
#define TWI_ST_DATA_ACK 0xB8 //!< Data sent successfuly, ACK received.
#define TWI_ST_DATA_NACK 0xC0 //!< Data has been sent, NACK received.
#define TWI_ST_LAST_DATA_ACK 0xC8 //!< The last data byte is transmitted, ACK received.

#define TWI_BUS_ERROR 0x0 //!< Generic bus error.

#ifdef TWCR
#undef TWCR
#undef TWBR
#undef TWSR
#undef TWDR
#undef TWAR
#undef TWAMR
#endif

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

#endif /* __TWIREG_328_H */