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
 * 
 * I2C messages which are attatched to an adapter are managed using a vector. This file is a simple
 * implementation of a vector (i.e. dynamic array). The reason an adapter uses a vector is that
 * \verbatim vector_get(int index) \endverbatim is verry fast. This function is used allot by several
 * bus implementations.
 * 
 * \addtogroup i2c-core
 * @{
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/error.h>
#include <dev/i2c.h>
#include <dev/i2c-core.h>
#include <dev/i2c-msg.h>

#include <sys/thread.h>
#include <sys/events/event.h>

/**
 * \def DEFAULT_MSG_LIMIT
 * \brief Default amount of available entries in i2c_adapter::msg_vector.
 */
#define DEFAULT_MSG_LIMIT 10

/**
 * \def ENTRY_SIZE
 * \brief Size of one entry in i2c_adapter::msg_vector.
 */
#define ENTRY_SIZE (sizeof(void*))

/*
 * static funcs
 */
static int i2c_vector_shift_left(struct i2c_msg_vector *vector, size_t index);
static int i2c_vector_shift_right(struct i2c_msg_vector *vector, size_t index, size_t num);

/**
 * \brief Allocate a new vector for adapter messages.
 * \param adapter Adapter to allocate a vector for.
 * \return Error code.
 * \retval -1 An error has occurred.
 * \retval 0 Allocation was successful.
 */
PUBLIC int i2c_create_msg_vector(struct i2c_adapter *adapter)
{
	int rc = -DEV_NULL;
	
	adapter->msg_vector.msgs = malloc(sizeof(*(adapter->msg_vector.msgs))*DEFAULT_MSG_LIMIT);
	
	if(adapter->msg_vector.msgs) {
		adapter->msg_vector.limit = DEFAULT_MSG_LIMIT;
		adapter->msg_vector.length = 0;
		rc = -DEV_OK;
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
PUBLIC struct i2c_message *i2c_vector_delete_at(struct i2c_adapter *adapter, size_t index)
{
	struct i2c_message *tmp;
	
	if(!adapter->msg_vector.msgs) {
		return PTR_ERROR(-DEV_NOINIT);
	}
	
	if(index < adapter->msg_vector.length) {
		tmp = adapter->msg_vector.msgs[index];
		adapter->msg_vector.msgs[index] = NULL;
		if(i2c_vector_shift_left(&adapter->msg_vector, index+1)) {
			adapter->msg_vector.length -= 1;
		}
		
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
PUBLIC struct i2c_message *i2c_vector_get(struct i2c_adapter *adapter, size_t index)
{
	if(!adapter->msg_vector.msgs) {
		return PTR_ERROR(-DEV_NOINIT);
	}
	if(index < adapter->msg_vector.length) {
		return adapter->msg_vector.msgs[index];
	} else {
		return NULL;
	}
}

/**
 * \brief Locate a message in an I2C vector.
 * \param adapter I2C adapter to locate the message in.
 * \param id Message to locate.
 * \return The message index. If the message is not found the return value will be 
 *         i2c_msg_vector::length + 1.
 */
PUBLIC size_t i2c_vector_locate(struct i2c_adapter *adapter, struct i2c_message *id)
{
	struct i2c_msg_vector *vector = &adapter->msg_vector;
	size_t ret;
	
	for(ret = 0; ret < vector->length; ret++) {
		if(vector->msgs[ret] == id) {
			return ret;
		} else {
			continue;
		}
	}
	
	return vector->length + 1; /* return an impossible value */
}

/**
 * \brief Add a new message to the adapter.
 * \param adapter The I2C adapter to add the message to.
 * \param msg I2C message to add.
 * \param master Indicates whether \p msg is a master message or not.
 * \return Error code.
 * \retval 0 when no error has occurred.
 * \retval -1 when an error has occurred.
 */
PUBLIC int i2c_vector_add(struct i2c_adapter *adapter, struct i2c_message *msg, bool master)
{
	void *buff = (void*)adapter->msg_vector.msgs;
	
	if(!buff) {
		return -DEV_NOINIT;
	} else if(adapter->msg_vector.length > adapter->msg_vector.limit) {
		return -DEV_OUTOFBOUNDS;
	}
	
	if(adapter->msg_vector.length == adapter->msg_vector.limit) {
		buff = realloc(buff, (adapter->msg_vector.limit + DEFAULT_MSG_LIMIT)*ENTRY_SIZE);
		if(buff) {
			adapter->msg_vector.msgs = buff;
			adapter->msg_vector.limit += DEFAULT_MSG_LIMIT;
		} else {
			return -DEV_NULL;
		}
	}

	if(i2c_vector_length(adapter) == 0) {
		adapter->msg_vector.msgs[0] = msg;
		adapter->msg_vector.length += 1;
		return 0;
	}

	if(master) {
		size_t i = 0;
		struct i2c_message *tmp;
		for(; i < adapter->msg_vector.length; i++) {
			tmp = adapter->msg_vector.msgs[i];
			if((i+1) == i2c_vector_length(adapter)) {
				adapter->msg_vector.msgs[i+1] = msg;
				adapter->msg_vector.length += 1;
				break;
			} else if(i2c_msg_is_master(tmp)) {
				i2c_vector_shift_right(&adapter->msg_vector, i, 1);
				adapter->msg_vector.msgs[i] = msg;
				break;
			} else {
				continue;
			}
		}
	} else {
		adapter->msg_vector.msgs[adapter->msg_vector.length] = msg;
		adapter->msg_vector.length += 1;
	}
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
PUBLIC struct i2c_message *i2c_vector_delete_msg(struct i2c_adapter *adapter, 
													 struct i2c_message *msg)
{
	size_t i;
	
	if(!adapter->msg_vector.msgs) {
		return PTR_ERROR(-DEV_NOINIT);
	}
	
	for(i = 0; i < adapter->msg_vector.length; i++) {
		if(adapter->msg_vector.msgs[i] == msg) {
			adapter->msg_vector.msgs[i] = NULL;
			if(i2c_vector_shift_left(&adapter->msg_vector, i+1)) {
				adapter->msg_vector.length -= 1;
			}
			return msg;
		}
	}
	
	return NULL;
}

/**
 * \brief Erease the entire vector.
 * \param adapter Vector to erease.
 * \see i2c_create_msg_vector
 * 
 * The vector will actually be reset to its initial state, an empty vector with a limit of
 * 10 elements.
 */
PUBLIC int i2c_vector_erase(struct i2c_adapter *adapter)
{
	free((void*)adapter->msg_vector.msgs);
	return i2c_create_msg_vector(adapter);
}

/**
 * \brief An error has occurred, try to fix.
 * \param adapter The I2C adapter.
 * \param error The error code.
 * \return The new error code.
 * \retval 0 when fixed.
 * \retval error if no fix was made.
 * \see dev_error
 * 
 * Errors this function can fix:
\verbatim
		+********************+*********************************************************************+
		| Error              | Solution                                                            |
		+********************+*********************************************************************+
		| DEV_NOINIT         | Create a new vector by calling i2c_create_msg_vector.               |
		+********************+*********************************************************************+
		| DEV_OUTOFBOUNDS    | Fix the boundries of the vector by exapanding i2c_msg_vector::limit.|
		+********************+*********************************************************************+
\endverbatim
 */
PUBLIC int i2c_vector_error(struct i2c_adapter *adapter, int error)
{
	struct i2c_msg_vector *vector = &adapter->msg_vector;
	
	if(error == -DEV_NULL) {
		return error;
	} else {
		error *= -1;
		switch(error) {
			case DEV_NOINIT:
				error = i2c_create_msg_vector(adapter);
				break;
				
			case DEV_OUTOFBOUNDS:
				vector->msgs = realloc((void*)vector->msgs, (vector->length+1) * ENTRY_SIZE);
				if(vector->msgs) {
					vector->limit = vector->length+1;
					error = -DEV_OK;
				} else {
					error = -DEV_NULL;
				}
				break;
				
			default:
				error = -DEV_ERROR;
				break;
		}
		return error;
	}
}

/**
 * \brief Insert one entry in the i2c_adapter.
 * \param adapter I2C adapter to insert into.
 * \param msg Message to insert.
 * \param index Index to insert at.
 * \return An error code.
 * \retval -DEV_OK on success.
 * \retval !-DEV_OK on error.
 * 
 * This function will insert \p msg in \p adapter at location \p index.
 */
PUBLIC int i2c_vector_insert_at(struct i2c_adapter *adapter, struct i2c_message *msg, size_t index)
{
	int rc;
	
	if(!adapter->msg_vector.msgs) {
		return -DEV_NOINIT;
	}
	
	if((rc = i2c_vector_shift_right(&adapter->msg_vector, index, 1)) == -DEV_OK) {
		adapter->msg_vector.msgs[index] = msg;
		return -DEV_OK;
	} else {
		return rc;
	}
}

/**
 * \brief Shift all elements to the right.
 * \param vector The vector to shift.
 * \param index Index to start shifting at.
 * \param num Amount of shifts.
 * \return Error code.
 * \retval 0 on success.
 * \retval !0 on error.
 * \note i2c_msg_vector::length is increased by one when successful.
 * \todo Add expansion.
 */
static int i2c_vector_shift_right(struct i2c_msg_vector *vector, size_t index, size_t num)
{
	size_t last;
	if((vector->length + num) >= vector->limit) {
		vector->msgs = realloc((void*)vector->msgs, (vector->limit+num)*ENTRY_SIZE);
		if(!vector->msgs) {
			return -DEV_NULL;
		} else {
			vector->limit += num;
		}
	}

	last = vector->length - 1;
	for(; last >= index; last--) {
		vector->msgs[last+num] = vector->msgs[last];
		if(last == 0) {
			/*
			 * This check is necessary due to the unsignedness of last.
			 */
			break;
		}
	}
	vector->length += num;
	return -DEV_OK;
}

/**
 * \brief Shift array indexes to the left.
 * \param vector The vector to shift.
 * \param index Index to start shifting.
 * \note To start at the first element set \p index to 0 (i.e. \p index is zero-counting).
 * \note i2c_msg_vector::length is decreased by one.
 * All values starting from \p index will be shifted 1 space to the left. So consider the following
 * array:
 * \verbatim
*+++++++++++*++++++++++++*+++++++++++*++++++++++++*+++++++++++*
|     5      |     4     |      7     |     3     |     9     |
*+++++++++++*++++++++++++*+++++++++++*++++++++++++*+++++++++++*
\endverbatim
 * If i2c_vector_shift_left is called with:
 * \code{.c}
i2c_vector_shift_left(vector, 2);
\endcode
 * then the array will look like:
\verbatim
*+++++++++++*++++++++++++*+++++++++++*++++++++++++*
|     5      |     7     |      3     |     9     |
*+++++++++++*++++++++++++*+++++++++++*++++++++++++*
\endverbatim
 */
static int i2c_vector_shift_left(struct i2c_msg_vector *vector, size_t index)
{
	if(index >= vector->length) {
		return 1;
	} else {
		if(index == 0) {
			vector->msgs[0] = vector->msgs[1];
			index++;
		}
		if(index < vector->length) {
			for(; index < vector->length; index++) {
				vector->msgs[index-1] = vector->msgs[index];
			}
		}
		vector->length -= 1;
		return 0;
	}
}

//@}
