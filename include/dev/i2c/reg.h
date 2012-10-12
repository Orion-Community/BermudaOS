/*
 *  BermudaOS - I2C registers
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

//! \file include/dev/i2c/reg.h I2C definitions

#ifndef __I2C_REG_H
#define __I2C_REG_H

/**
 * \brief Time-out for I2C transfers if threads are enabled.
 */
#define I2C_TMO 500

/**
 * \brief Number of I2C messages in one array.
 * \see I2C_MASTER_TRANSMIT_MSG
 * \see I2C_MASTER_RECEIVE_MSG
 * \see I2C_SLAVE_RECEIVE_MSG
 * \see I2C_SLAVE_TRANSMIT_MSG
 */
#define I2C_MSG_NUM 4

/**
 * \brief Master transmit message array index.
 */
#define I2C_MASTER_TRANSMIT_MSG 0
/**
 * \brief Master receive message array index.
 */
#define I2C_MASTER_RECEIVE_MSG  1
/**
 * \brief Slave receive message array index.
 */
#define I2C_SLAVE_RECEIVE_MSG   2
/**
 * \brief Slave transmit message array index.
 */
#define I2C_SLAVE_TRANSMIT_MSG  3

/* file flags */
#define I2C_MASTER 0x100 //!< I2C master flag
#define I2C_SLAVE  0x200 //!< I2C slave flag

/* adapter flags */
#define I2C_MASTER_ENABLE 		BIT(0) //!< Master enable bit in the flags member of i2c_client.
#define I2C_TRANSMITTER 		BIT(1)
#define I2C_RECEIVER			BIT(2)
#define I2C_ERROR				BIT(3)
#define I2C_CALL_BACK			BIT(4)

#define I2C_SLAVE_MASK			(~BIT(0))
#define I2C_SLAVE_ENABLE		0

#endif
