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

/**
 * \file include/dev/i2c/busses/atmega.h ATmega bus header.
 */

#ifndef __ATMEGA_I2C_H
#define __ATMEGA_I2C_H

#include <stdlib.h>

/**
 * \brief The maximum I2C busses an ATmega MCU can have.
 */
#define ATMEGA_BUSSES 1

extern struct i2c_adapter *atmega_i2c_busses[ATMEGA_BUSSES];

/**
 * \brief Slave address.
 */
#define ATMEGA_I2C_C0_SLA 0x56

/**
 * \def ATMEGA_I2C_C0
 * \brief I2C bus 0 on port C.
 * \see atmega_i2c_init_client
 */
#define ATMEGA_I2C_C0 0

#define ATMEGA_I2C_C0_ADAPTER (atmega_i2c_busses[ATMEGA_I2C_C0])

__DECL
extern void atmega_i2c_init_client(struct i2c_client *client, uint8_t ifac);
extern void atmega_i2c_c0_hw_init(uint8_t sla, struct i2c_adapter *adapter);
__DECL_END

#endif /* __ATMEGA_I2C_H */