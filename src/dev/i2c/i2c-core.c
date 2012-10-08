/*
 *  BermudaOS - I2C device driver
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

#include <stdlib.h>
#include <stdio.h>

#include <dev/i2c/i2c.h>

/**
 * \brief Prepare the driver for an I2C transfer.
 * \param stream Device file.
 * \param msg Message to add to the buffer.
 * \return -1 if a fatal error has occurred. The user application should allways
 *         respond with <b>i2cdev_reset</b> in this case. If no problems have occurred
 *         0 will be returned. When there is data overwritten in the stream buffer
 *         1 will be returned. It is up to the caller to react solve (continue or
 *         reset).
 */
PUBLIC int i2c_setup_master_transfer(FILE *stream, struct i2c_message *msg,
									 uint8_t flags)
{	
	struct i2c_message *msgs = (struct i2c_message*)stream->buff;
	
	msgs[flags].buff = msg->buff;
	msgs[flags].length = msg->length;
	msgs[flags].freq = msg->freq;
	msgs[flags].addr = msg->addr;
	
	return 0;
}
