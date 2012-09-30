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

#include <dev/dev.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>

#include <sys/thread.h>
#include <sys/events/event.h>

char *i2c_core = "I2C_CORE";

static __link int i2c_bus_router(FILE *file, const void *buff, size_t size)
{
	struct i2c_adapter *adapter = (struct i2c_adapter*)buff;
	struct i2c_message *msg;
	int fd, status;
	
	fd = open(adapter->dev->io->name, _FDEV_SETUP_RWB);
	if(fd < 0) {
		return -1;
	}
	
	status = adapter->status;
	
	switch(status) {
		default:
			break;
	}
	
	return -1;
}

static __link int i2c_io_controller(struct device *dev, int flags, void *buff)
{
	return -1;
}
