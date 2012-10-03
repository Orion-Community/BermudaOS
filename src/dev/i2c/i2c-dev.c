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

PUBLIC int i2c_init_adapter(struct i2c_adapter *adapter, char *fname)
{
	int rc = -1;
	adapter->dev = BermudaHeapAlloc(sizeof(*(adapter->dev)));
	
	if(adapter->dev == NULL) {
		return rc;
	} else {
		rc = 0;
	}
	
	adapter->dev->name = fname;
	BermudaDeviceRegister(adapter->dev, adapter);
	
	adapter->flags = 0;
	return rc;
}

PUBLIC int i2c_dev_write(FILE *file, const void *buff, size_t size)
{
	struct device *dev = (struct device *)file->data;
	struct i2c_adapter *adapter = (struct i2c_adapter*)dev->ioctl;
	static uint8_t index = 0;
	int rc;
	
#ifdef __THREADS__
	if((rc = dev->alloc(dev, I2C_MASTER_TMO)) == -1) {
		goto end;
	}
#endif

#ifdef __THREADS__
	rc = dev->release(dev);
	end:
#endif
	return rc;
}

PUBLIC int i2c_dev_read(FILE *file, void *buff, size_t size)
{
	return -1;
}
