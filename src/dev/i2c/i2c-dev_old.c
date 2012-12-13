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

/**
 * \file src/dev/i2c/i2c-dev_old.c I2C device driver.
 */

/**
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
\code{.c}
struct i2c_client client;
atmega_i2c_init_client(&client, ATMEGA_I2C_C0);
client.sla = 0x54;      // slave address
client.freq = 100000UL; // frequency in hertz
int rc, fd;

fd = i2cdev_socket(&client, _FDEV_SETUP_RW | I2C_MASTER);
if(fd < 0) {
    error();
}

rc = write(fd, txbuff, txbuff_length);
rc += read(fd, rxbuff, rxbuff_length);

if(rc == 0) {
    rc = flush(fd);
} else {
    i2cdev_error(fd);
}
  
close(fd);
\endcode
 * 
 * If you want to do a transmit or receive only you should set the correct buffer to <i>NULL</i>. The
 * transmit buffer will be set using <i>write</i> the receive buffer is set using <i>read</i>.
 * 
 * <b>Slave recieve/transmit</b>
\code{.c}
struct i2c_client slave_client;

atmega_i2c_init_client(&slave_client, ATMEGA_I2C_C0);
slave_client.callback = &slave_responder;
 
fd = i2cdev_socket(&slave_client, _FDEV_SETUP_RW | I2C_SLAVE);
if(fd < 0) {
    goto _usart;
}
i2cdev_listen(fd, &rx, 1);
close(fd);
\endcode
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
#include <dev/i2c/i2c-core.h>
#include <dev/i2c/reg.h>

#include <sys/thread.h>
#include <sys/events/event.h>

/**
 * \brief Handle an I2C file I/O error.
 * \param flags Set to I2C_MASTER when it occured in a master driver.
 * 
 * It will free the correspondending buffers. If I2C_MASTER is given all master buffers will be
 * free'd if I2C_SLAVE is passed, all slave buffers will be free'd.
 */
PUBLIC void i2cdev_error(int fd)
{
	FILE *stream = fdopen(fd);
	struct i2c_client *client = stream->data;
	
	if((stream->flags & I2C_MASTER) != 0) {
		i2c_cleanup_master_msgs(client->adapter->dev->io, client->adapter);
	} else {
		i2c_cleanup_slave_msgs(client->adapter->dev->io, client->adapter);
	}
}


/**
 * @}
 */
