/*
 *  BermudaOS - Architecture independent I2C header.
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

//!< \file include/dev/i2c/i2c.h I2C header

#ifndef I2C_H_
#define I2C_H_

#include <bermuda.h>

#include <dev/dev.h>
#include <dev/twidev.h>

struct i2c_client {
	struct i2c_client *next;
	struct device *dev;

	uint8_t id;
	bool allocated;
	void *mutex;
	void *queue;

#ifdef __THREADS__
	struct thread *allocator;
#endif
} __attribute__((packed));

struct i2c_adapter {
	struct i2c_client *clients; //!< List of maintained I2C clients.

} __attribute__((packed));

struct i2c_message {

};

__DECL
extern i2c_client *i2c_get_client_by_id(struct i2c_adapter *adap, uint8_t id);
extern i2c_client *i2c_alloc_client(struct i2c_adapter *adap);
extern int i2c_free_client(struct i2c_client *client);
extern int i2c_move_client(struct i2c_adapter *adap1, struct i2c_adapter *adap2,
		uint8_t id);
extern bool i2c_client_is_allocated(struct i2c_client *client);
extern bool i2c_client_is_allocated_by_caller(struct i2c_client *client);

extern int i2c_client_write(struct i2c_client *client, struct i2c_message *msg);
__DECL_END


#endif /* I2C_H_ */
