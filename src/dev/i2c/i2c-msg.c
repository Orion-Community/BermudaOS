/*
 *  BermudaOS - I2C-MSG manager
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
 * \file src/dev/i2c/i2c-msg.c I2C-MSG manager
 * \brief Manage I2C adapter messages.
 * \addtogroup i2c-core
 * @{
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/i2c.h>
#include <dev/i2c-core.h>

#include <sys/thread.h>
#include <sys/events/event.h>

#define DEFAULT_MSG_LIMIT 10
#define ENTRY_SIZE (sizeof(void*))

/**
 * \brief Allocate a new vector for adapter messages.
 * \param adapter Adapter to allocate a vector for.
 * \return Error code.
 * \retval -1 An error has occurred.
 * \retval 0 Allocation was successful.
 */
PUBLIC int i2c_create_msg_vector(struct i2c_adapter *adapter)
{
	int rc = -1;
	
	adapter->msg_vector.msgs = malloc(sizeof(*(adapter->msg_vector.msgs))*DEFAULT_MSG_LIMIT);
	
	if(adapter->msg_vector.msgs) {
		adapter->msg_vector.limit = DEFAULT_MSG_LIMIT;
		adapter->msg_vector.length = 0;
		rc = 0;
	}
	return rc;
}

/**
 * \brief Delete a message from the adapter.
 * \param adapter I2C adapter to delete from.
 * \param index Index to delete at.
 * \return The deleted entry, if it exists.
 * \retval NULL When index is out of bounds
 * \retval i2c_message The message which was at index.
 * 
 * Delete the entry at \p index from \p adapter.
 */
PUBLIC struct i2c_message *i2c_msg_vector_delete_at(struct i2c_adapter *adapter, size_t index)
{
	struct i2c_message *tmp;
	
	if(index < adapter->msg_vector.length) {
		tmp = adapter->msg_vector.msgs[index];
		adapter->msg_vector.msgs[index] = NULL;
		return tmp;
	} else {
		return NULL;
	}
}

/**
 * \brief Return the message at <i>index</i>.
 * \param adapter The I2C adapter.
 * \param index Index to return.
 * \return The i2c_message at \p index. If \p index is out of bounds <i>NULL</i> is returned.
 * \retval NULL when an error has occurred.
 * \retval i2c_message when no error has occurred.
 */
PUBLIC struct i2c_message *i2c_msg_vector_get(struct i2c_adapter *adapter, size_t index)
{
	if(index < adapter->msg_vector.length) {
		return adapter->msg_vector.msgs[index];
	} else {
		return NULL;
	}
}

/**
 * \brief Add a new message to the adapter.
 * \param adapter The I2C adapter to add the message to.
 * \param msg I2C message to add.
 * \return Error code.
 * \retval 0 when no error has occurred.
 * \retval -1 when an error has occurred.
 */
PUBLIC int i2c_msg_vector_add(struct i2c_adapter *adapter, struct i2c_message *msg)
{
	void *buff = adapter->msg_vector.msgs;
	
	if(adapter->msg_vector.length == adapter->msg_vector.limit) {
		buff = realloc(buff, (adapter->msg_vector.limit + DEFAULT_MSG_LIMIT)*ENTRY_SIZE);
		if(buff) {
			adapter->msg_vector.msgs = buff;
		} else {
			return -1;
		}
	}
	
	adapter->msg_vector.msgs[adapter->msg_vector.length] = msg;
	adapter->msg_vector.length += 1;
	return 0;
}

/**
 * \brief Delete the given message from the adapter.
 * \param adapter I2C adapter to delete from.
 * \param msg Message object to delete.
 * \return The deleted message.
 * \retval msg if the message is found.
 * \retval NULL if the message is not found.
 */
PUBLIC struct i2c_message *i2c_msg_vector_delete_msg(struct i2c_adapter *adapter, 
													 struct i2c_message *msg)
{
	size_t i;
	
	for(i = 0; i < adapter->msg_vector.length; i++) {
		if(adapter->msg_vector.msgs[i] == msg) {
			adapter->msg_vector.msgs[i] = NULL;
			return msg;
		}
	}
	
	return NULL;
}

//@}
