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

/**
 * \brief Initializes the buffers for I2C transfer.
 * \param file I/O file.
 * \param buff The I2C message.
 * \param size Should equal sizeof(struct i2c_message).
 * \return This function returns 0 on success. This means that the bus is
 *         successfully claimed. If this function returns <b>< 0</b>, the bus
 *         is not successfully claimed and NO buffers are initialized.
 */
PUBLIC int i2cdev_write(FILE *file, const void *buff, size_t size)
{
	struct i2c_client *client = file->data;
	struct i2c_message msg;
	int rc = -1;
	
	if(client != NULL) {
		msg.buff = (void*)buff;
		msg.length = size;
		msg.freq = client->freq;
		msg.addr = client->sla;
		rc = i2c_setup_master_transfer(file, &msg, I2C_MASTER_TRANSMIT_MSG);
	}

	
	return rc;
}

PUBLIC int i2cdev_read(FILE *file, void *buff, size_t size)
{
	struct i2c_client *client = file->data;
	struct i2c_message msg;
	int rc = -1;
	
	if(client != NULL) {
		msg.buff = (void*)buff;
		msg.length = size;
		msg.freq = client->freq;
		msg.addr = client->sla;
		rc = i2c_setup_master_transfer(file, &msg, I2C_MASTER_RECEIVE_MSG);
	}

	
	return rc;
}

PUBLIC int i2cdev_socket(struct i2c_client *client, uint8_t flags)
{
	struct i2c_adapter *adap;
	struct device *dev;
	int rc;
	
	if(client == NULL) {
		return -1;
	}
	
	adap = client->adapter;
	dev = adap->device;

#ifdef __THREADS__
	if(flags == I2C_MASTER) {
		if((rc = dev->alloc(dev, I2C_TMO)) == -1) {
			rc = -1;
			goto out;
		}
	}
#endif
	
	dev->io->data = client;
	rc = open(dev->io->name, _FDEV_SETUP_RW);
	
	out:
	return rc;
}

PUBLIC int i2cdev_listen(int fd, void *buff, size_t size)
{
	return -1;
}
