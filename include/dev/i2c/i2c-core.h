/*
 *  BermudaOS - I2C core layer
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

#ifndef __I2C_CORE_H
#define __I2C_CORE_H

#include <lib/binary.h>

#include <dev/i2c.h>

/**
 * \addtogroup i2c-core
 * @{
 */

/*
 * i2c message features
 */

#define I2C_MSG_CALL_BACK_FLAG_SHIFT 0 //!< Shift value of I2C_MSG_CALL_BACK_FLAG
#define I2C_MSG_SLAVE_MSG_FLAG_SHIFT 1 //!< Shift value of I2C_MSG_SLAVE_MSG_FLAG
/**
 * \brief Master message flag.
 * \see I2C_MSG_SLAVE_MSG_FLAG_SHIFT I2C_MSG_MASTER_MSG_MASK
 */
#define I2C_MSG_MASTER_MSG_FLAG_SHIFT I2C_MSG_SLAVE_MSG_FLAG_SHIFT //!< I2C master message shift.
#define I2C_MSG_TRANSMIT_MSG_FLAG_SHIFT 2 //!< Shift value of I2C_MSG_TRANSMIT_MSG_FLAG
#define I2C_MSG_SENT_STOP_FLAG_SHIFT 3 //!< Shift value of I2C_MSG_SENT_STOP_FLAG
#define I2C_MSG_SENT_REP_START_FLAG_SHIFT 4 //!< Shift value of I2C_MSG_SENT_REP_START_FLAG
#define I2C_MSG_DONE_FLAG_SHIFT 5 //!< Shift value of I2C_MSG_DONE_FLAG.

/**
 * \brief Defines that the application should be called after transmission.
 */
#define I2C_MSG_CALL_BACK_FLAG BIT(I2C_MSG_CALL_BACK_FLAG_SHIFT)

/**
 * \brief Defines that this message is sent from slave perspective.
 */
#define I2C_MSG_SLAVE_MSG_FLAG BIT(I2C_MSG_SLAVE_MSG_FLAG_SHIFT)
/**
 * \brief Master message flag.
 * \see I2C_MSG_SLAVE_MSG_FLAG
 */
#define I2C_MSG_MASTER_MSG_FLAG 0

/**
 * \brief Defines the message holds a transmit buffer, not a receive buffer.
 */
#define I2C_MSG_TRANSMIT_MSG_FLAG BIT(I2C_MSG_TRANSMIT_MSG_FLAG_SHIFT)

/**
 * \brief Defines that a stop bit should be sent after transmission.
 */
#define I2C_MSG_SENT_STOP_FLAG BIT(I2C_MSG_SENT_STOP_FLAG_SHIFT)

/**
 * \brief Defines that a repeated start should be sent after transmission.
 * \warning This bit does NOT exist in the flags argument to i2c_queue_processor.
 * 
 * Either this bit has to be set or I2C_MSG_SENT_STOP_FLAG has to be set.
 */
#define I2C_MSG_SENT_REP_START_FLAG BIT(I2C_MSG_SENT_REP_START_FLAG_SHIFT)

/**
 * \brief When set the message is handled by the adapter and it can be deleted safely.
 */
#define I2C_MSG_DONE_FLAG BIT(I2C_MSG_DONE_FLAG_SHIFT)

/*
 * Message feature masks
 */

/**
 * \brief Defines that the application should be called after transmission.
 */
#define I2C_MSG_CALL_BACK_MASK BIT(I2C_MSG_CALL_BACK_FLAG_SHIFT)

/**
 * \brief Defines that this message is sent from slave perspective.
 */
#define I2C_MSG_SLAVE_MSG_MASK BIT(I2C_MSG_SLAVE_MSG_FLAG_SHIFT)

/**
 * \brief Master message mask.
 * \see I2C_MSG_MASTER_MSG_FLAG
 * When this bit is set to one the message is a <b>slave</b> message.
 */
#define I2C_MSG_MASTER_MSG_MASK BIT(I2C_MSG_MASTER_MSG_FLAG_SHIFT)

/**
 * \brief Defines the message holds a transmit buffer, not a receive buffer.
 */
#define I2C_MSG_TRANSMIT_MSG_MASK BIT(I2C_MSG_TRANSMIT_MSG_FLAG_SHIFT)

/**
 * \brief Defines that a stop bit should be sent after transmission.
 */
#define I2C_MSG_SENT_STOP_MASK BIT(I2C_MSG_SENT_STOP_FLAG_SHIFT)

/**
 * \brief Defines that a repeated start should be sent after transmission.
 * \warning This bit does NOT exist in the flags argument to i2c_queue_processor.
 * 
 * Either this bit has to be set or I2C_MSG_SENT_STOP_FLAG has to be set.
 */
#define I2C_MSG_SENT_REP_START_MASK BIT(I2C_MSG_SENT_REP_START_FLAG_SHIFT)

/**
 * \brief When set the message is handled by the adapter and it can be deleted safely.
 */
#define I2C_MSG_DONE_MASK BIT(I2C_MSG_DONE_FLAG_SHIFT)

/**
 * \brief I2C message features mask.
 * 
 * Mask which masks all bits in the i2c_message::features field.
 */
#define I2C_MSG_FEATURES_MASK (I2C_MSG_CALL_BACK_FLAG | I2C_MSG_MASTER_MSG_MASK |   \
                              I2C_MSG_SLAVE_MSG_FLAG | I2C_MSG_TRANSMIT_MSG_FLAG |  \
                              I2C_MSG_SENT_STOP_FLAG | I2C_MSG_SENT_REP_START_FLAG)
                              

/**
 * \brief Read message bit.
 * \see I2C_MSG_TRANSMIT_MSG_FLAG
 * 
 * When this bit is set, the message holds a receive buffer.
 */
#define I2C_MSG_READ BIT(0)
//@}

__DECL
/**
 * \addtogroup i2c-core
 * @{
 */
/*
 * I2C-CORE functions
 */
extern int i2c_init_adapter(struct i2c_adapter *adapter, char *fname);
extern void i2c_cleanup_client_msgs(struct i2c_client *client);
extern int i2c_flush_client(struct i2c_client *client);
extern int i2c_write_client(struct i2c_client *client, const void *data, size_t size, 
							i2c_features_t flags);

#ifdef I2C_DBG
extern int i2cdbg_test_queue_processor(struct i2c_client *client);
#endif

/*
 * inline functions
 */

/**
 * \brief Get the shared info of the given I2C client.
 * \param client Client whose shared info you want.
 * \return The shared info of the given client.
 */
static inline struct i2c_shared_info *i2c_shinfo(struct i2c_client *client)
{
	return client->sh_info;
}

/**
 * \brief Get the I2C features of the given client.
 * \param client Client whose features you want.
 * \return The features of the given client.
 */
static inline i2c_features_t i2c_client_features(struct i2c_client *client)
{
	return i2c_shinfo(client)->features;
}

/**
 * \brief Get the I2C features of the given adapter.
 * \param adapter The adapter whose features are wanted.
 * \return The features of the given adapter.
 */
static inline i2c_features_t i2c_adapter_features(struct i2c_adapter *adapter)
{
	return adapter->features;
}

/**
 * \brief Mask the client for queue a queue error.
 * \param client Client to mask.
 */
static inline void i2c_set_error(struct i2c_client *client)
{
	i2c_features_t features = i2c_client_features(client);
	
	features |= I2C_QUEUE_ERROR;
	i2c_shinfo(client)->features = features;
}

/**
 * \brief Set the features of an I2C message.
 * \param msg Message to set the features for.
 * \param features Features to set.
 * 
 * Sets the i2c_message::features field.
 */
static inline void i2c_msg_set_features(struct i2c_message *msg, i2c_features_t features)
{
	msg->features = features;
}

/**
 * \brief Get the features of a msg.
 * \param msg The message which features are needed.
 * \return The features of the given I2C message.
 */
static inline i2c_features_t i2c_msg_features(struct i2c_message *msg)
{
	return msg->features;
}

/**
 * \brief Set the features of a given client.
 * \param client Client whose features are being set.
 * \param features Features to set.
 * \see i2c_shinfo
 */
static inline void i2c_client_set_features(struct i2c_client *client, i2c_features_t features)
{
	i2c_shinfo(client)->features = features;
}

/**
 * \brief Set the layout of the upcomming transmission.
 * \param client Client which is hosting the transmission.
 * \param layout The transmission layout to set.
 */
static inline void i2c_set_transmission_layout(struct i2c_client *client, char *layout)
{
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	shinfo->transmission_layout = layout;
}

/**
 * \brief Get the transmission layout.
 * \param client Client to get the transmission layout from.
 * \note Transmission layouts are only used in master transfers.
 */
static inline char *i2c_transmission_layout(struct i2c_client *client)
{
	return i2c_shinfo(client)->transmission_layout;
}

//@}
__DECL_END

#endif /* __I2C_CORE_H */