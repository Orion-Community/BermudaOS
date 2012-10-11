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
		rc = i2c_setup_master_transfer(client->adapter->dev->io, &msg, 
									   I2C_MASTER_TRANSMIT_MSG);
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
		rc = i2c_setup_master_transfer(client->adapter->dev->io, 
									   &msg, I2C_MASTER_RECEIVE_MSG);
	}

	
	return rc;
}

PUBLIC int i2cdev_socket(struct i2c_client *client, uint16_t flags)
{
	struct i2c_adapter *adap;
	struct device *dev;
	FILE *socket;
	int rc = -1;
	
	if(client == NULL) {
		return -1;
	}
	
	adap = client->adapter;
	dev = adap->dev;

#ifdef __THREADS__
	if((flags & I2C_MASTER) != 0) {
		if((rc = dev->alloc(dev, I2C_TMO)) == -1) {
			rc = -1;
			goto out;
		}
	}
#endif
	
	socket = BermudaHeapAlloc(sizeof(*socket));
	rc = iob_add(socket);
	if(rc < 0) {
		BermudaHeapFree(socket);
		dev->release(dev);
		goto out;
	}
	
	socket->data = client;
	socket->name = "I2C";
	socket->write = &i2cdev_write;
	socket->read = &i2cdev_read;
	socket->flush = &i2cdev_flush;
	socket->close = &i2cdev_close;
	socket->flags = flags;
	
	out:
	return rc;
}

PUBLIC int i2cdev_flush(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adap = client->adapter;
	int rc = adap->dev->io->fd;
	
	adap->dev->io->data = stream->data;
	adap->dev->io->flags &= 0xFF;
	adap->dev->io->flags |= stream->flags & 0xFF00;
	
	rc = flush(rc);
	return rc;
}

PUBLIC int i2cdev_listen(int fd, void *buff, size_t size)
{
	FILE *stream = fdopen(fd);
	struct i2c_message msg;
	struct i2c_client *client;
	int rc, dev;
	
	if(stream == NULL) {
		rc = -1;
		goto out;
	}
	
	client = stream->data;
	dev = client->adapter->dev->io->fd;

	msg.buff = buff;
	msg.length = size;
	msg.addr = client->sla;
	msg.freq = client->freq;
	i2c_setup_master_transfer(fdopen(dev), &msg, 
							  I2C_SLAVE_RECEIVE_MSG);
	fdopen(dev)->data = client;
	rc = client->adapter->slave_listen(fdopen(dev));
	
	if(!rc) {
		if(client->callback) {
			fdopen(dev)->data = client;
			rc = i2c_call_client(client, fdopen(dev));
		} else {
			_exit(); /* fatal error, restart */
		}
	}
	
	out:
	return rc;
}

PUBLIC int i2cdev_close(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adap = client->adapter;
	int rc = -1;
	
	if(stream != NULL) {
		if(stream->buff != NULL) {
			BermudaHeapFree((void*)stream->buff);
		}
		BermudaHeapFree(stream);
		rc = 0;
	}
	
#ifdef __THREADS__
	if(stream->flags & I2C_MASTER) {
		adap->dev->release(adap->dev);
	}
#endif
	return rc;
}
