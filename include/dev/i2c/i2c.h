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

#include <stdlib.h>

#include <dev/dev.h>

struct i2c_adapter; // forward declaration

/**
 * \brief Structure which defines the data to be transfered in the next transaction.
 */
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
	
	uint8_t *buff; //!< Message buffer.
	size_t length; //!< Message length.
	size_t index;  //!< Buffer index.
} __attribute__((packed));

/**
 * \brief The i2c_client represents the sender or receiver.
 * 
 * The receiver or sender is responsible for knowing where the post should be
 * delivered (ie. what is the slave address).
 */
struct i2c_client {
	struct device *dev;      //!< COM device.
	struct i2c_adapter *adapter; //!< The adapter where it belongs to.
	uint8_t sla;
} __attribute__((packed));

struct i2c_adapter {
	struct device *dev; //!< Adapter device.
	
	uint8_t flags; //!< Bus flags.
#define I2C_MASTER_ENABLE 		BIT(0) //!< Master enable bit in the flags member of i2c_client.
#define I2C_TRANSMITTER 		BIT(1)
#define I2C_RECEIVER			BIT(2)

#ifdef __THREADS__
	volatile void *mutex;
	volatile void *master_queue;
	volatile void *slave_queue;
	struct thread *allocator;
#endif
	
	struct i2c_message *master_msg[]; //!< TWI master message.
	struct i2c_message *slave_msg[];  //!< TWI slave message.
	size_t master_index; //!< TWI master buffer index.
	size_t slave_index;   //!< TWI slave buffer index.
} __attribute__((packed));

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
