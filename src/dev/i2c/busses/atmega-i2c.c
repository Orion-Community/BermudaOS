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

#include <fs/vfile.h>

#include <arch/io.h>
#include <arch/irq.h>

#include "atmega_priv.h"

/* static functions */
static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data);
static void atmega_i2c_setup_file(FILE *stream, struct i2c_message *msg);
static int atmega_i2c_init_transfer(FILE *stream);

static FDEV_SETUP_STREAM(i2c_c0_io, NULL, NULL, 
						 NULL /*put*/, NULL /*get*/, &atmega_i2c_init_transfer, 
						 I2C_FNAME, 0 /* flags */, NULL /* data */);

static struct atmega_i2c_priv i2c_c0 = {
	.twcr = &TWCR,
	.twdr = &TWDR,
	.twsr = &TWSR,
	.twar = &TWSR,
	.twbr = &TWBR,
	.twamr = &TWAMR,
	.isr = atmega_i2c_isr_handle,
};

#ifdef __THREADS__
static volatile void *bus_c0_mutex = SIGNALED;
static volatile void *bus_c0_master_queue = SIGNALED;
static volatile void *bus_c0_slave_queue = SIGNALED;
#endif

/**
 * \brief Array of all available I2C busses
 */
static struct i2c_adapter *atmega_i2c_busses[ATMEGA_BUSSES];

static struct i2c_message i2c_c0_msgs[I2C_MSG_NUM];

/**
 * \brief Intialize an I2C bus.
 * \param adapter Bus adapter.
 * 
 * This function will initialize bus 0 on port c.
 */
PUBLIC void i2c_c0_hw_init(struct i2c_adapter *adapter)
{
	int rc = i2c_init_adapter(adapter, i2c_c0_io.name);
	if(rc < 0) {
		return;
	}

	atmega_i2c_busses[0] = adapter;
	adapter->dev->mutex = &bus_c0_mutex;
	adapter->dev->ctrl = &atmega_i2c_ioctl;
	adapter->dev->io = &i2c_c0_io;
	
	adapter->dev->io->buff = (void*)i2c_c0_msgs;
	
	adapter->master_queue = &bus_c0_master_queue;
	adapter->slave_queue = &bus_c0_slave_queue;
	adapter->data = (void*)&i2c_c0;

	vfs_add(&i2c_c0_io);
}

/**
 * \brief Start a master transfer.
 * \param stream The device I/O file.
 * 
 * This function is called by the user using the <b>flush(fd : int)</b> funtion.
 */
static int atmega_i2c_init_transfer(FILE *stream)
{
	return -1;
}

static void atmega_i2c_ioctl(struct device *dev, int cfg, void *data)
{
	int i = 1;
	
	do {
		switch(cfg & i) {
			default:
				break;
		}
		i <<= 1;
	} while(i <= ((sizeof(i)*2UL) - 1UL));
	
	return;
}

ISR(atmega_i2c_isr_handle, adapter, struct i2c_adapter *)
{
	struct atmega_i2c_priv *priv = (struct atmega_i2c_priv*)adapter->data;
	int fd, status = atmega_i2c_get_status(priv);
	
	if(adapter->dev->io->fd < 0) {
		fd = adapter->dev->io->fd;
	} else {
		return;
	}
	
	switch(status) {
		default:
			break;
	}
}

SIGNAL(TWI_STC_vect)
{
	atmega_i2c_isr_handle(atmega_i2c_busses[0]);
}
