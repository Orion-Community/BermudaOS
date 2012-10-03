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

#ifndef __I2C_REG_H
#define __I2C_REG_H

#include <stdlib.h>

#include <lib/binary.h>



enum i2c_control_action 
{
	I2C_START_SENT,
	I2C_REP_START_SENT,
	I2C_RECV_STOP,
	
	I2C_SLAW_ACK,
	I2C_SLAW_NACK,
	I2C_SLAR_ACK,
	I2C_SLAR_NACK,
	I2C_DATA_WRITE_ACK,
	I2C_DATA_WRITE_NACK,
	I2C_DATA_READ_ACK,
	I2C_DATA_READ_NACK,
	I2C_BUS_ARB,
};

#endif
