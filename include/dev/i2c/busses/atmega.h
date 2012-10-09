/*
 *  BermudaOS - SPI bus driver
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

#ifndef __ATMEGA_I2C_H
#define __ATMEGA_I2C_H

#include <stdlib.h>

/**
 * \def ATMEGA_I2C_C0
 * \brief I2C bus 0 on port C.
 */
#define ATMEGA_I2C_C0 0

__DECL
extern void atmega_i2c_init_client(struct i2c_client *client, uint8_t ifac);
extern void atmega_i2c_c0_hw_init(struct i2c_adapter *adapter);
__DECL_END

#endif /* __ATMEGA_I2C_H */