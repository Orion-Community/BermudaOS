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

//! \file arch/avr/328/dev/devreg.h

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
#define TWI_SR_SLAW_DATA_ACK 0x80 //!< Own SLA+W received, ACK returned.
#define TWI_SR_SLAW_DATA_NACK 0x88 //!< Own SLA+W received, NACK returned.
#define TWI_SR_GC_DATA_ACK 0x90 //!< General call received, ACK returned.
#define TWI_SR_GC_DATA_NACK 0x98 //!< General call received, NACK returned.
#define TWI_SR_STOP 0xA0 //!< Stop or repeated start condition received.

#ifdef TWCR
#undef TWCR
#undef TWBR
#undef TWSR
#undef TWDR
#undef TWAR
#undef TWAMR
#endif

#define TWBR  MEM_IO8(0xB8)
#define TWCR  MEM_IO8(0xBC)
#define TWSR  MEM_IO8(0xB9)
#define TWDR  MEM_IO8(0xBB)
#define TWAR  MEM_IO8(0xBA)
#define TWAMR MEM_IO8(0xBD)

#endif /* __TWIREG_328_H */