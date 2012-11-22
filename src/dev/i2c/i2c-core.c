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
 * \file src/dev/i2c/i2c-core.c I2C core functions.
 * \addtogroup i2c
 * @{
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>

/**
 * \brief Prepare the driver for an I2C transfer.
 * \param stream Device file.
 * \param msg Message to add to the buffer.
 * \return 0 will be returned on success, -1 otherwise.
 */
PUBLIC int i2c_setup_msg(FILE *stream, struct i2c_message *msg,
									 uint8_t flags)
{
	return fwrite(stream, msg, flags);
}

/**
 * \brief Clean up master buffers.
 * \param Peripheral I/O file.
 */
PUBLIC void i2c_cleanup_master_msgs(FILE *stream, struct i2c_adapter *adapter)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	
	adapter->cleanup_list[I2C_MASTER_TRANSMIT_MSG] = (void*)msgs[I2C_MASTER_TRANSMIT_MSG];
	adapter->cleanup_list[I2C_MASTER_RECEIVE_MSG]  = (void*)msgs[I2C_MASTER_RECEIVE_MSG];
	msgs[I2C_MASTER_TRANSMIT_MSG] = NULL;
	msgs[I2C_MASTER_RECEIVE_MSG]  = NULL;
}

/**
 * \brief Clean up slave buffers.
 * \param Peripheral I/O file.
 */
PUBLIC void i2c_cleanup_slave_msgs(FILE *stream, struct i2c_adapter *adapter)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	
	adapter->cleanup_list[I2C_SLAVE_RECEIVE_MSG] = (void*)msgs[I2C_SLAVE_RECEIVE_MSG];
	adapter->cleanup_list[I2C_SLAVE_TRANSMIT_MSG] = (void*)msgs[I2C_SLAVE_TRANSMIT_MSG];
	msgs[I2C_SLAVE_RECEIVE_MSG] = NULL;
	msgs[I2C_SLAVE_TRANSMIT_MSG]  = NULL;
}

/**
 * \brief Clean up all marked I2C messages.
 * 
 * All allocated memory will be free'd.
 */
PUBLIC void i2c_do_clean_msgs(struct i2c_adapter *adapter)
{
	uint8_t i = 0;
	
	for(; i < I2C_MSG_NUM; i++) {
		if(adapter->cleanup_list[i] != NULL) {
			BermudaHeapFree(adapter->cleanup_list[i]);
			adapter->cleanup_list[i] = NULL;
		}
	}
}

/**
 * \brief Mark a message to be cleaned up.
 * \param stream I/O file. The message array should be in the buffer of this I/O file.
 * \param msg Index of the message to clean up.
 */
PUBLIC void i2c_cleanup_msg(FILE *stream, struct i2c_adapter *adapter, uint8_t msg)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	adapter->cleanup_list[msg] = (void*)msgs[msg];
	msgs[msg] = NULL;
}

/**
 * \brief Edit the queues of the given I2C client.
 * \param client Client to edit the queues of.
 * \param data Data to append to the queue.
 * \param size Length of <i>data</i>.
 * \note The given I2C client must have allocated its bus adapter.
 * \see list_last_entry
 *
 * Append the given <i>data</i> to the client queue. When a flush signal is given
 * the queue will be moved to the appropriate I2C adapter. If the client has not\
 * allocated (i.e. locked) its bus adapter, current I2C transfer may get corrupted.
 *
 * The complexity of this function when appending data is \f$ O(n) \f$, since the new
 * data is appended at the end of the queue. See list_last_entry for more information
 * about the editting of queues.
 */
static int i2c_edit_queue(struct i2c_client *client, const void *data, size_t size)
{
	return -1;
}

/**
 * \brief Arrange the call back to the client.
 * \param client Client to call.
 * \param stream Bus I/O file.
 */
PUBLIC int i2c_call_client(struct i2c_client *client, FILE *stream)
{
	struct i2c_message msg;
	
	client->callback(&msg);
	fwrite(stream, &msg, I2C_SLAVE_TRANSMIT_MSG);
	
	return client->adapter->slave_respond(stream);
}
/**
 * @}
 */