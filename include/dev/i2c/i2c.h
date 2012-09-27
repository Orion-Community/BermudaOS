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
	struct i2c_client *next; //!< Next client in the list.
	struct device *dev;      //!< COM device.
	uint8_t id; //!< Client ID.
	uint8_t flags; //!< Bus flags.
	
#ifdef __THREADS__
	struct thread *allocator;
#endif
} __attribute__((packed));

struct i2c_adapter {
	struct i2c_client *clients; //!< List of maintained I2C clients.

	struct device *dev; //!< Adapter device.
	struct thread *handle; //!< I2C command handler.

} __attribute__((packed));

struct i2c_message
{
	/**
	 * \brief Call back used in slave operation.
	 * \param msg Message used in the operation.
	 * 
	 * This function pointer will be called when a slave receive is done. The
	 * application can then check the received values and apply the correct
	 * transmission values.
	 */
	void (*call_back)(struct i2c_message *msg);
	const unsigned char *tx_buff; //!< Transmit buffer.
	size_t tx_length; //!< Transmit buffer length.
	
	unsigned char *rx_buff; //!< Receive buffer.
	size_t rx_length; //!< Receive buffer length.
	
	uint32_t scl_freq; //!< TWI operation frequency in master mode.
	unsigned int tmo; //!< Maximum transfer waiting time-out. Used in master and slave.
	unsigned char sla; //!< Slave address to address in master mode.
};

__DECL
extern struct i2c_client *i2c_get_client_by_id(struct i2c_adapter *adap, uint8_t id);
extern struct i2c_client *i2c_alloc_client(struct i2c_adapter *adap);
extern int i2c_free_client(struct i2c_client *client);
extern int i2c_move_client(struct i2c_adapter *adap1, struct i2c_adapter *adap2,
		uint8_t id);
extern bool i2c_client_is_allocated(struct i2c_client *client);
extern bool i2c_client_is_allocated_by_caller(struct i2c_client *client);

extern int i2c_client_write(struct i2c_client *client, struct i2c_message *msg);
__DECL_END


#endif /* I2C_H_ */
