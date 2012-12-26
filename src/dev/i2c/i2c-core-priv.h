/*
 *  BermudaOS - I2C core (private header)
 *  Copyright (C) 2012   Michel Megens <dev@michelmegens.net>
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
 * \file src/dev/i2c/i2c-core-priv.h I2C private header.
 * \brief Private header for the I2C Core.
 * 
 * \addtogroup i2c-core
 * @{
 */

#ifndef __I2C_CORE_PRIV_H
#define __I2C_CORE_PRIV_H

#include <lib/binary.h>

/**
 * \brief Check a message against the bus.
 * \param __msg I2C message features.
 * \param __bus I2C bus features.
 * \retval 1 if the message is a master message ánd the bus supports master messages.
 * \retval 0 in any other case.
 * 
 * Checks \p __msg against \p __bus to see if it is a master message, and if so, if the bus supports
 * master transfers.
 */
#define I2C_MSG_MASTER_CHECK(__msg, __bus) \
( \
(((neg(__msg) & I2C_MSG_MASTER_MSG_MASK) >> I2C_MSG_MASTER_MSG_FLAG_SHIFT) & \
(__bus & I2C_MASTER_SUPPORT)) \
)

/**
 * \brief Check a message against the bus.
 * \param __msg I2C message features.
 * \param __bus I2C bus features.
 * \retval 1 if the message is a slave message ánd the bus supports slave messages.
 * \retval 0 in any other case.
 * 
 * Checks \p __msg against \p __bus to see if it is a slave message, and if so, if the bus supports
 * slave transfers.
 */
#define I2C_MSG_SLAVE_CHECK(__msg, __bus) \
( \
((__msg >> I2C_MSG_SLAVE_MSG_FLAG_SHIFT) & ((__bus & I2C_SLAVE_SUPPORT) >> I2C_SLAVE_SUPPORT_SHIFT)) \
)

/**
 * \brief Check a message against its bus to see if it can be sent.
 * \param __msg I2C message features.
 * \param __bus I2C bus features.
 */
#define I2C_MSG_CHECK(__msg, __bus) \
( \
	xor(I2C_MSG_MASTER_CHECK(__msg, __bus), I2C_MSG_SLAVE_CHECK(__msg, __bus)) \
)

#endif /* __I2C_CORE_PRIV_H */

//@}
