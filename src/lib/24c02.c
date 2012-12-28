/*
 *  BermudaOS - Serial EEPROM library
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

//!< \file lib/24c02.c Serial EEPROM library

#include <stdlib.h>
#include <stdio.h>

#include <fs/vfile.h>

#include <dev/dev.h>
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>
#include <dev/i2c/i2c-core.h>

#include <lib/24c02.h>

static struct i2c_client *client;

/**
 * \brief Initialize the 24C02 driver.
 * \param iicc The I2C client
 * \note It is very important that the bus passed to this routine
 *       is initialized.
 * 
 * When this routine is called, the driver is functional and ready to use.
 */
PUBLIC void Bermuda24c02Init(struct i2c_client *iicc)
{
	client = iicc;
}

PUBLIC int Bermuda24c02WriteByte(unsigned char addr, unsigned char data)
{
	int rc = -1, fd;
	unsigned char tx[] = { addr, data };
	
	fd = i2cdev_socket(client, _FDEV_SETUP_RW | I2C_MASTER);
	if(fd < 0) {
		return rc;
	}
	
	rc = write(fd, tx, 2);
	rc += read(fd, NULL, 0);
	if(rc == 0) {
		rc = flush(fd);
	} else {
		i2cdev_error(fd);
	}
	close(fd);

	return rc;
}

PUBLIC unsigned char Bermuda24c02ReadByte(unsigned char addr)
{
	unsigned char tx = addr;
	unsigned char rx = 0;
	int fd, rc;
	
	fd = i2cdev_socket(client, _FDEV_SETUP_RW | I2C_MASTER);
	if(fd < 0) {
		return 0xFF;
	}
	
	rc = write(fd, &tx, 1);
	rc += read(fd, &rx, 1);
	flush(fd);
	close(fd);

	return rx;
}
