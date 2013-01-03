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
/**
 * \addtogroup i2c
 * @{
 */

#ifndef __I2C_REG_H
#define __I2C_REG_H

/**
 * \brief The interface is free.
 */
#define TW_IF_IDLE (BIT(SCL) | BIT(SDA))

/**
 * \brief Time-out for I2C transfers if threads are enabled.
 */
#define I2C_TMO 500

/**
 * \def I2C_TIMEOUT
 * \brief Time-out error.
 */
#define I2C_TIMEOUT (-1)

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
/**
 * \brief Call back flag.
 * 
 * All messages added when I2CDEV_CALL_BACK is set will receive a call back. The flag can be removed
 * using the mode(int fd) function.
 * \see mode
 */
#define I2CDEV_CALL_BACK 0x400
#define I2CDEV_CALL_BACK_SHIFT 10 //!< I2CDEV_CALL_BACK shift.
#endif
/**
 * @}
 */