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

/**
 * \brief General I2C controller.
 * \param file Device I/O file.
 * \param buff Contains I2C client which requested the call.
 * \param size In case of a read write operation, <i>size</i> determines the amount
 *             of bytes to read/write into/from the buffers.
 * \see i2c_control_action
 * 
 * Not all I2C busses are compatible with this controller, in that case they 
 * have their own controller specified in their bus specific file.
 */
static __link int i2c_bus_controller(FILE *file, const void *buff, size_t size)
{
	struct i2c_client *client = (struct i2c_client*)buff;
	struct i2c_message *msg;
	
	int fd = open(client->dev->io->name, _FDEV_SETUP_RWB);
	enum i2c_control_action action = (enum i2c_control_action)fgetc(fdopen(fd,
		_FDEV_SETUP_RWB
	));
	close(fd);
	
	if((client->flags & I2C_MASTER_ENABLE) != 0) {
		/*
		 * The bus is in master mode. This means that all actions will be done
		 * on master buffers only.
		 */
		msg = client->master_msg;
		
		if(msg->rx_length) {
			client->flags |= I2C_RECEIVER;
					
		} if(msg->tx_length) {
			client->flags |= I2C_TRANSMITTER;
			client->flags &= ~I2C_RECEIVER;
		}
		
		switch(action) {
			/*
			 * The I2C start command is successfully sent.
			 */
			case I2C_START_SENT:
			case I2C_REP_START_SENT:
				client->master_index = 0;
				if((client->flags & I2C_TRANSMITTER) == 0) {
					msg->sla |= I2C_SLA_READ_BIT;
				} else {
					msg->sla &= I2C_SLA_WRITE_MASK;
				}
				
				/* Now we can sent the SLA */
				client->dev->ctrl(client->dev, I2C_WRITE_SLA | I2C_ACK, 
								  (void*)&(msg->sla));
				break;
				
			/*
			 * I2C_SLA_ACK: Slave address is sent and received by slave. 
			 *              ACK returned.
			 * I2C_DATA_WRITE_ACK: Data is sent and received. ACK returned.
			 */
			case I2C_SLA_ACK:
			case I2C_DATA_WRITE_ACK:
				if(client->master_index + size <= msg->tx_length) {
					do {
						client->dev->ctrl(client->dev, I2C_WRITE_DATA | I2C_ACK,
										  (void*)&(msg->tx_buff[client->master_index]));
						client->master_index++;
					} while(client->master_index < client->master_index + size);
					break;
				} else if(msg->rx_length) {
					/* initiate a read */
					msg->tx_length = 0;
					client->dev->ctrl(client->dev, I2C_START | I2C_ACK, NULL);
					break;
				} else {
					/* end of transfer */
					msg->tx_length = 0;
					client->dev->ctrl(client->dev, I2C_STOP, NULL);
					//TODO: Finish off.
				}
		}
	}
	
	return -1;
}

static __link int i2c_io_controller(struct device *dev, int flags, void *buff)
{
	return -1;
}
