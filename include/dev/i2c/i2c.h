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

//! \file include/dev/i2c/i2c.h I2C header
/**
 * \addtogroup i2c
 * @{
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdlib.h>

#include <lib/binary.h>
#include <dev/dev.h>

struct i2c_adapter; // forward declaration

typedef uint8_t i2c_features_t;

/*
 * i2c message features
 */
#define I2C_SENT_STOP_FLAG B1
#define I2C_SENT_REP_START_FLAG B10
#define I2C_MASTER_MSG_FLAG B100
#define I2C_TRANSMIT_MSG_FLAG B1000

/*
 * i2c shared info features
 */
#define I2C_CALL_BACK_FLAG B1
#define I2C_CLIENT_HAS_LOCK_FLAG B10

#define I2C_QUEUE_ACTION_MASK B11 //!< Masks the bits 2-3 in i2c_shared_info::features.
#define I2C_QUEUE_ACTION_SHIFT 2  //!< Shift to reach the queue action bits.
#define I2C_QUEUE_ACTION_NEW B00  //!< Flag to create a new queue entry.
#define I2C_QUEUE_ACTION_INSERT B01 //!< Flag to insert a queue entry at the start
#define I2C_QUEUE_ACTION_FLUSH B10 //!< Flag to flush the queue to the adapter.

#define I2C_NEW_QUEUE_ENTRY B00
#define I2C_INSERT_QUEUE_ENTRY B01
#define I2C_FLUSH_QUEUE B10

/**
 * \brief Structure which defines the data to be transfered in the next transaction.
 */
struct i2c_message
{
	uint8_t *buff; //!< Message buffer.
	size_t length; //!< Message length.
	uint16_t addr; //!< Slave address.
	
	i2c_features_t features;
} __attribute__((packed));

/**
 * \brief Structure shared by all from a certain client.
 */
struct i2c_shared_info
{
	struct rcu_list *list; //!< RCU list of messages.
	size_t rcu_list_size; //!< Amount of messages in the list.
	
	/**
	 * \brief Call back function which can be called after a buffer has been sent by the driver.
	 * \param msg Message used in the operation.
	 * \param client I2C client which ordered the transmission.
	 * 
	 * This function pointer will be called when a transmission is done. The
	 * application can then insert another message in the queue if that is nessecary.
	 */
	int (*shared_callback)(struct i2c_client *client, struct i2c_message *msg);
	
	i2c_features_t features; //!< Feature option flags.
};

/**
 * \brief The i2c_client represents the sender or receiver.
 * 
 * The receiver or sender is responsible for knowing where the post should be
 * delivered (ie. what is the slave address).
 */
struct i2c_client {
	struct i2c_adapter *adapter; //!< The adapter where it belongs to.
	uint16_t sla; //!< Slave address.
	uint32_t freq; //!< Operating frequency in master mode.
	
	struct i2c_shared_info *sh_info; //!< 
	
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
	bool busy; //!< Defines wether the interface is busy or not.
	uint8_t error;
	
	struct rcu_list *msgs;

#ifdef __THREADS__
	/* mutex is provided by the device */
	volatile void **master_queue; //!< Master waiting queue.
	volatile void **slave_queue; //!< Slave waiting queue.
#else
	mutex_t mutex; //!< Bus mutex.
	mutex_t master_queue; //!< Master mutex.
	mutex_t slave_queue;  //!< Slave mutex.
#endif
	
	struct i2c_message **cleanup_list;
	void *data; //!< Private data pointer.
	
	/**
	 * \brief Function pointer to respond after a slave receive.
	 * \param stream I/O file.
	 */
	int (*slave_respond)(FILE *stream);
} __attribute__((packed));

__DECL
/* file I/O */
extern int i2cdev_write(FILE *file, const void *buff, size_t size);
extern int i2cdev_read(FILE *file, void *buff, size_t size);
extern int i2cdev_flush(FILE *stream);
extern int i2cdev_close(FILE *stream);
extern int i2cdev_socket(struct i2c_client *client, uint16_t flags);
extern int i2cdev_listen(int fd, void *buff, size_t size);
extern void i2cdev_error(int fd);

/* init routines */
extern int i2c_init_adapter(struct i2c_adapter *adap, char *name);

/* core functions */
extern int i2c_setup_msg(FILE *stream, struct i2c_message *msg, uint8_t flags);
extern int i2c_call_client(struct i2c_client *client, FILE *stream);
extern void i2c_cleanup_msg(FILE *stream, struct i2c_adapter *adap, uint8_t msg);
extern void i2c_do_clean_msgs(struct i2c_adapter *adap);
extern void i2c_cleanup_master_msgs(FILE *stream, struct i2c_adapter *adap);
extern void i2c_cleanup_slave_msgs(FILE *stream, struct i2c_adapter *adap);
__DECL_END


#endif /* I2C_H_ */
/**
 * @}
 */