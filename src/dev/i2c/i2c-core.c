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
#include <dev/i2c/reg.h>

static void *cleanup_list[I2C_MSG_NUM] = { NULL, NULL, NULL, NULL, };

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
	struct i2c_message *msg2;
	struct i2c_message **msgs = (struct i2c_message**)stream->buff;
	

	if(msg == NULL) {
		msgs[flags] = NULL;
		return 0;
	}
	
	msg2 = BermudaHeapAlloc(sizeof(*msg2));
	if(!msg2) {
		return -1;
	}
	
	msg2->buff = msg->buff;
	msg2->length = msg->length;
	msg2->freq = msg->freq;
	msg2->addr = msg->addr;
	
	msgs[flags] = msg2;
	return 0;
}

PUBLIC void i2c_do_clean_msgs(FILE *stream)
{
	uint8_t i = 0;
	
	for(; i < I2C_MSG_NUM; i++) {
		if(cleanup_list[i] != NULL) {
			BermudaHeapFree(cleanup_list[i]);
			cleanup_list[i] = NULL;
		}
	}
}

PUBLIC void i2c_cleanup_msg(FILE *stream, uint8_t msg)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	cleanup_list[msg] = (void*)msgs[msg];
	msgs[msg] = NULL;
}

/**
 * \brief Arrange the call back to the client.
 * \param client Client to call.
 * \param stream Bus I/O file.
 */
PUBLIC int i2c_call_client(struct i2c_client *client, FILE *stream)
{
	struct i2c_message **msgs = (struct i2c_message**)stream->buff;
	struct i2c_message *msg = BermudaHeapAlloc(sizeof(*msg));
	
	client->callback(msg);
	msgs[I2C_SLAVE_TRANSMIT_MSG] = msg;
	return client->adapter->slave_respond(stream);
}
