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

#ifndef __TWIF_H
#define __TWIF_H

#include <bermuda.h>

#include <sys/thread.h>
#include <fs/vfile.h>
#include <dev/twif.h>

#include <sys/events/event.h>

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

#endif __TWIF_H
