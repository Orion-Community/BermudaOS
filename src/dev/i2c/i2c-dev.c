/*
 *  BermudaOS - I2C device driver
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
 * \file src/dev/i2c/i2c-dev.c I2C device driver
 *
 * \addtogroup i2c I2C module
 * \brief Universal I2C driver.
 * 
 * \section Unix Unix
 * This driver uses a file based unix interface. This means that all communication is done using
 * file I/O. The only differency is the opening of files. Normally the (f)open function is used for
 * this purpose, but in this case i2cdev_socket is used.
 * 
 * \section io I/O and error handling
 * <b>Master read/write</b>
 * When you want to do a master transfer, sent a repeated start and receive data from the slave
 * your I/O block will look like the following.
 * 
 * \code{i2cmaster.c}
 * struct i2c_client client;
 * atmega_i2c_init_client(&client, ATMEGA_I2C_C0);
 * client.sla = 0x54;      // slave address
 * client.freq = 100000UL; // frequency in hertz
 * int rc, fd;
 * 
 * fd = i2cdev_socket(&client, _FDEV_SETUP_RW | I2C_MASTER);
 * if(fd < 0) {
 *     error();
 *     return;
 * }
 * 
 * rc = write(fd, txbuff, txbuff_length);
 * rc += read(fd, rxbuff, rxbuff_length);
 * 
 * if(rc == 0) {
 *     rc = flush(fd);
 * } else {
 *     i2cdev_error(fd);
 * }
 * 
 * close(fd);
 * \endcode
 * 
 * If you want to do a transmit or receive only you should set the correct buffer to <i>NULL</i>. The
 * transmit buffer will be set using <i>write</i> the receive buffer is set using <i>read</i>.
 * 
 * <b>Slave recieve/transmit</b>
 * \code{i2cslave.c}
 * struct i2c_client slave_client;
 * 
 * atmega_i2c_init_client(&slave_client, ATMEGA_I2C_C0);
 * slave_client.callback = &slave_responder;
 * 
 * fd = i2cdev_socket(&slave_client, _FDEV_SETUP_RW | I2C_SLAVE);
 * if(fd < 0) {
 *    error();
 *    return;
 * }
 * i2cdev_listen(fd, &rx, 1);
 * close(fd);
 * \endcode
 * 
 * The slave will first wait for a master receive request. When it is finished, it will check for a
 * user callback function. That function can set the transmit buffer. Set the buffer to <i>NULL</i>
 * to not transmit any data as a slave.
 * @{
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>
#include <dev/i2c/i2c-core.h>

#include <sys/thread.h>
#include <sys/epl.h>
#include <sys/events/event.h>

#if 0
PUBLIC int i2cdev_open(char *data, uint8_t flags)
{
	return -1;
}

/**
 * \brief Request an I2C I/O file.
 * \param client I2C driver client.
 * \param flags File flags.
 * \return The file descriptor associated with this client.
 */
PUBLIC int i2cdev_socket(struct i2c_client *client, uint16_t flags)
{
	FILE *socket;
	i2c_features_t features;
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	int rc = -1;
	
	if(client == NULL) {
		return -1;
	}
	
	if(BermudaEventWait(event(&(shinfo->mutex)), I2C_TMO) != 0) {
		goto out;
	}
	
	socket = BermudaHeapAlloc(sizeof(*socket));
	if(!socket) {
		rc = -1;
		goto out;
	}
	
	rc = iob_add(socket);
	if(rc < 0) {
		BermudaHeapFree(socket);
		BermudaEventSignal(event(&(shinfo->mutex)));
		goto out;
	}
	
	features = i2c_client_features(client);
	features |= I2C_CLIENT_HAS_LOCK_FLAG;
	i2c_client_set_features(client, features);
	
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

/**
 * \brief Flush the I/O file.
 * \param stream File to flush.
 * 
 * This will start the actual transfer.
 */
PUBLIC int i2cdev_flush(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adap = client->adapter;
	int rc;
	
	if(adap->dev->alloc(adap->dev, I2C_TMO) != 0) {
		return -1;
	}
	
	adap->dev->io->data = stream->data;
	adap->dev->io->flags &= 0xFF;
	adap->dev->io->flags |= stream->flags & 0xFF00;
	
	rc = i2c_flush_client(client);
	
	adap->dev->release(adap->dev);
	i2c_do_clean_msgs(adap);
	return rc;
}

/**
 * \brief Close the I2C socket.
 * \param stream File to close.
 */
PUBLIC int i2cdev_close(FILE *stream)
{
	struct i2c_client *client = stream->data;
	struct i2c_adapter *adap = client->adapter;
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	i2c_features_t features;
	int rc = -1;
	
	i2c_do_clean_msgs(adap);
	if(stream != NULL) {
		if(stream->buff != NULL) {
			BermudaHeapFree((void*)stream->buff);
		}
		BermudaHeapFree(stream);
		rc = 0;
	}
	
	BermudaEventSignal(event(&(shinfo->mutex)));
	
	features = i2c_client_features(client);
	features &= ~I2C_CLIENT_HAS_LOCK_FLAG;
	i2c_client_set_features(client, features);
	
	return rc;
}
#endif
