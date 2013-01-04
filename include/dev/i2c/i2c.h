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
#include <sys/epl.h>

struct i2c_adapter; // forward declaration
struct i2c_client;

/**
 * \addtogroup i2c-core
 * @{
 */

/**
 * \brief Definition of the I2C features type.
 */
typedef uint8_t i2c_features_t;

/**
 * \brief Definition of the I2C action type.
 */
typedef uint8_t i2c_action_t;

/*
 * i2c shared info features
 */
/**
 * \brief Defines that messages have to be called back.
 * \deprecated Each message has its own bit defining call back functionallity.
 */
#define I2C_CALL_BACK_FLAG B1
#define I2C_CLIENT_HAS_LOCK_FLAG B10 //!< Defines that the client has locked the adapter.

/**
 * \brief Masks all queue action bits.
 * \deprecated Not used.
 */
#define I2C_QUEUE_ACTION_MASK (I2C_QUEUE_ACTION_NEW | I2C_QUEUE_ACTION_INSERT | \
							  I2C_QUEUE_ACTION_FLUSH)
#define I2C_QUEUE_ACTION_SHIFT 2  //!< Shift to reach the queue action bits.
#define I2C_QUEUE_ACTION_NEW B100  //!< Flag to create a new queue entry.
#define I2C_QUEUE_ACTION_INSERT B1000 //!< Flag to insert a queue entry at the start
#define I2C_QUEUE_ACTION_FLUSH B10000 //!< Flag to flush the queue to the adapter.
#define I2C_ACTION_PENDING B100000 //!< If set, the currently set action is not executed yet.
#define I2C_QUEUE_ERROR B1000000 //!< An error has occured while queuing a message.

#define I2C_DELETE_QUEUE_ENTRY B000 //!< Defines the deletion of a queue entry.
#define I2C_NEW_QUEUE_ENTRY B1 //!< Defines the creation of a queue entry.
#define I2C_INSERT_QUEUE_ENTRY B10 //!< Defines the insertion of a queue entry.
#define I2C_FLUSH_QUEUE_ENTRIES B100 //!< Definition of the flush action.

/*
 * I2C adapter features
 */
#define I2C_MASTER_SUPPORT_SHIFT 0 //!< I2C_MASTER_SUPPORT shift.
#define I2C_SLAVE_SUPPORT_SHIFT 1 //!< I2C_SLAVE_SUPPORT shift.

/**
 * \brief When set, the adapter supports master transfers.
 * \see I2C_MSG_CHECK
 */
#define I2C_MASTER_SUPPORT BIT(I2C_MASTER_SUPPORT_SHIFT)
/**
 * \brief When set, the adapter supports slave transfers.
 * \see I2C_MSG_CHECK
 */
#define I2C_SLAVE_SUPPORT BIT(I2C_SLAVE_SUPPORT_SHIFT)

/**
 * \brief Structure which defines the data to be transfered in the next transaction.
 */
struct i2c_message
{
	uint8_t *buff; //!< Message buffer.
	size_t length; //!< Message length.
	uint16_t addr; //!< Slave address.
	
	i2c_features_t features; //!< I2C message features.
} __attribute__((packed));

/**
 * \brief Structure shared by all from a certain client.
 */
struct i2c_shared_info
{
	struct epl_list *list; //!< EPL list of messages.
	
	struct i2c_adapter *adapter; //!< The I2C adapter.
	FILE *socket; //!< I/O socket.
	char *transmission_layout; //!< I2C transmission layout.
	
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
	
	/**
	 * \brief Core layer mutex.
	 * This mutex must be locked before the core layer may edit the queue's of the client.
	 */
	volatile void *mutex;
};

//@}

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
	
	struct i2c_shared_info *sh_info; //!< Shared info.
} __attribute__((packed));

/**
 * \brief Message vector structure.
 */
struct i2c_msg_vector;

struct i2c_adapter {
	struct device *dev; //!< Adapter device.
	
	i2c_features_t features; //!< Adapter features.
	bool busy; //!< Defines wether the interface is busy or not.
	/**
	 * \brief Adapter error field.
	 */
	uint8_t error;
	
	/**
	 * \var msg_vector
	 * \brief Message vector (dynamic array), to list all messages for transmission.
	 * The messages are sorted by master/slave (master messages first, then slave messages).
	 */
	struct i2c_msg_vector
	{
		size_t length, //!< Length of the vector.
		       limit;  //!< Maximum value \p length may reach.
		struct i2c_message *volatile*msgs; //!< The data array.
	} msg_vector;

#ifdef __THREADS__
	/* mutex is provided by the device */
	volatile void **master_queue; //!< Master waiting queue.
	volatile void **slave_queue; //!< Slave waiting queue.
#endif
	
	void *data; //!< Private data pointer.
	
	/**
	 * \brief Transfer hook implemented by the bus driver.
	 * \param adapter The bus adapter.
	 * \param freq The frequency to operate on.
	 * \return The current message index.
	 * 
	 * A hook which <b>must</b> be implemented by the adapter to initialise a transfer.
	 */
	int (*xfer)(struct i2c_adapter *adapter, uint32_t freq, bool master, size_t *index);
	
	/**
	 * \brief Resume transmission after a call back.
	 * \param adapter Adapter to resume.
	 * \note The message which has been set by the call back must be inserted IN THE FRONT of the
	 *       message vector.
	 * \return The current message index.
	 */
	int (*resume)(struct i2c_adapter *adapter, size_t *index);
	
	/**
	 * \brief Update the driver message index.
	 * \param diff The index diff.
	 * \note The driver is required to implement this function pointer.
	 */
	void (*update)(struct i2c_adapter *adapter, long diff);
};

#include <dev/i2c-core.h>

/* file I/O */
extern int i2cdev_write(FILE *file, const void *buff, size_t size);
extern int i2cdev_read(FILE *file, void *buff, size_t size);
extern int i2cdev_flush(FILE *stream);
extern int i2cdev_close(FILE *stream);
extern int i2cdev_socket(struct i2c_client *client, uint16_t flags);
extern int i2cdev_listen(int fd, void *buff, size_t size);
extern void i2cdev_error(int fd);
extern struct i2c_client *i2c_alloc_client(struct i2c_adapter *adapter, uint16_t sla, uint32_t hz);
extern void i2c_init_client(struct i2c_client *client, struct i2c_adapter *adapter, 
							uint16_t sla, uint32_t hz);

/**
 * \brief Set a call back function.
 * \param client i2c_client to set the call back for.
 * \param cb The call back function.
 */
static inline void i2c_set_callback(struct i2c_client *client, int (*cb)(struct i2c_client *,
																		 struct i2c_message *))
{
	i2c_shinfo(client)->shared_callback = cb;
}

//@}
__DECL_END


#endif /* I2C_H_ */
/**
 * @}
 */