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

/**
 * \brief Time-out for I2C transfers if threads are enabled.
 */
#define I2C_TMO 500

/**
 * \brief Number of I2C messages in one array.
 * \see I2C_MASTER_TRANSMIT_MSG
 * \see I2C_MASTER_RECEIVE_MSG
 * \see I2C_SLAVE_RECEIVE_MSG
 * \see I2C_SLAVE_TRANSMIT_MSG
 */
#define I2C_MSG_NUM 4

/**
 * \brief Master transmit message array index.
 */
#define I2C_MASTER_TRANSMIT_MSG 0
/**
 * \brief Master receive message array index.
 */
#define I2C_MASTER_RECEIVE_MSG  1
/**
 * \brief Slave receive message array index.
 */
#define I2C_SLAVE_RECEIVE_MSG   2
/**
 * \brief Slave transmit message array index.
 */
#define I2C_SLAVE_TRANSMIT_MSG  3

#define I2C_MASTER 0xAB
#define I2C_SLAVE  0xBA

struct i2c_adapter; // forward declaration

/**
 * \brief Structure which defines the data to be transfered in the next transaction.
 */
struct i2c_message
{
	uint8_t *buff; //!< Message buffer.
	size_t length; //!< Message length.
	uint16_t addr; //!< Slave address.
	uint32_t freq; //!< Frequency.
} __attribute__((packed));

/**
 * \brief The i2c_client represents the sender or receiver.
 * 
 * The receiver or sender is responsible for knowing where the post should be
 * delivered (ie. what is the slave address).
 */
struct i2c_client {
	struct i2c_adapter *adapter; //!< The adapter where it belongs to.
	uint8_t sla;
	uint32_t freq;
	
	/**
	 * \brief Call back used in slave operation.
	 * \param msg Message used in the operation.
	 * 
	 * This function pointer will be called when a slave receive is done. The
	 * application can then check the received values and apply the correct
	 * transmission values.
	 */
	void (*callback)(struct i2c_message *msg);
} __attribute__((packed));

struct i2c_adapter {
	struct device *dev; //!< Adapter device.
	
	uint8_t flags; //!< Bus flags.
#define I2C_MASTER_ENABLE 		BIT(0) //!< Master enable bit in the flags member of i2c_client.
#define I2C_TRANSMITTER 		BIT(1)
#define I2C_RECEIVER			BIT(2)

#ifdef __THREADS__
	/* mutex is provided by the device */
	volatile void *master_queue; //!< Master waiting queue.
	volatile void *slave_queue; //!< Slave waiting queue.
#endif
	
	void *data; //!< Private data pointer.
} __attribute__((packed));

__DECL
extern int i2c_init_adapter(struct i2c_adapter *adap, char *name);
extern struct i2c_client *i2c_alloc_client(struct i2c_adapter *adap);
extern int i2c_free_client(struct i2c_client *client);

extern int i2cdev_write(FILE *file, const void *buff, size_t size);
extern int i2cdev_read(FILE *file, void *buff, size_t size);

extern int i2c_setup_master_transfer(FILE *stream, struct i2c_message *msg);
__DECL_END


#endif /* I2C_H_ */
