/*
 *  BermudaOS - C++ Support
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

#ifndef __I2C_MSG_H
#define __I2C_MSG_H

#include <dev/i2c.h>
#include <dev/i2c-core.h>

/**
 * \brief Iterate through an I2C vector.
 * \param __vec The vector to iterate through.
 * \param __it name of the iterator.
 */
#define i2c_vector_foreach(__vec, __it) \
size_t __it; \
for(__it = 0; __it < (__vec)->length; (__it)++)

/**
 * \brief Iterate through an I2C vector reversed.
 * \param __vec The vector (pointer) to iterate through.
 * \param __it The symbol name of the iterator.
 * 
 * Iterate through the list using the symbel name given by \p __it. It starts at 
 * i2c_msg_vector::length - 1 (the last entry) and decreses by one every iteration until 0 has been
 * reached.
 */
#define i2c_vector_foreach_reverse(__vec, __it) \
size_t __it; \
for(__it = (__vec)->length - 1; __it >= 0; (__it)--)

__DECL
/*
 * I2C-MSG functions
 */
extern int i2c_create_msg_vector(struct i2c_adapter *adapter);
extern int i2c_vector_add(struct i2c_adapter *adapter, struct i2c_message *msg);
extern struct i2c_message *i2c_vector_get(struct i2c_adapter *adapter, size_t index);
extern struct i2c_message *i2c_vector_delete_at(struct i2c_adapter *adapter, size_t size);
extern struct i2c_message *i2c_vector_delete_msg(struct i2c_adapter *adapter, 
													 struct i2c_message *msg);
extern int i2c_vector_error(struct i2c_adapter *adapter, int error);
extern int i2c_vector_erase(struct i2c_adapter *adapter);
extern int i2c_vector_insert_at(struct i2c_adapter *adapter, struct i2c_message *msg, size_t index);
extern size_t i2c_vector_locate(struct i2c_adapter *adapter, struct i2c_message *id);

/**
 * \brief Retrieve the length of the vector on an I2C adapter.
 * \param adapter The I2C adapter to retrieve the vector length from.
 * \return The vector length.
 */
static inline size_t i2c_vector_length(struct i2c_adapter *adapter)
{
	return adapter->msg_vector.length;
}

static inline bool i2c_msg_is_master(struct i2c_message *msg)
{
	return ((neg(i2c_msg_features(msg)) & I2C_MSG_MASTER_MSG_MASK) != 0);
}
__DECL_END

#endif /* __I2C_MSG_H */