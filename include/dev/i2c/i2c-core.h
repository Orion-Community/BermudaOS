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
 * \see I2C_MSG_SLAVE_MSG_FLAG_SHIFT I2C_MSG_MASTER_MSG_FLAG
 */
#define I2C_MSG_MASTER_MSG_FLAG_SHIFT I2C_MSG_SLAVE_MSG_FLAG_SHIFT
#define I2C_MSG_TRANSMIT_MSG_FLAG_SHIFT 2 //!< Shift value of I2C_MSG_TRANSMIT_MSG_FLAG
#define I2C_MSG_SENT_STOP_FLAG_SHIFT 3 //!< Shift value of I2C_MSG_SENT_STOP_FLAG
#define I2C_MSG_SENT_REP_START_FLAG_SHIFT 4 //!< Shift value of I2C_MSG_SENT_REP_START_FLAG

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
 * \see I2C_MSG_MASTER_MSG_FLAG_SHIFT
 */
#define I2C_MSG_MASTER_MSG_FLAG BIT(I2C_MSG_MASTER_MSG_FLAG_SHIFT)

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
 * \brief I2C message features mask.
 * 
 * Mask which masks all bits in the i2c_message::features field.
 */
#define I2C_MSG_FEATURES_MASK (I2C_MSG_CALL_BACK_FLAG | I2C_MSG_MASTER_MSG_FLAG |   \
                              I2C_MSG_SLAVE_MSG_FLAG | I2C_MSG_TRANSMIT_MSG_FLAG |  \
                              I2C_MSG_SENT_STOP_FLAG | I2C_MSG_SENT_REP_START_FLAG)
//@}

__DECL
/**
 * \addtogroup i2c-core
 * @{
 */
extern int i2c_setup_msg(FILE *stream, struct i2c_message *msg, uint8_t flags);
extern int i2c_call_client(struct i2c_client *client, FILE *stream);
extern void i2c_cleanup_msg(FILE *stream, struct i2c_adapter *adap, uint8_t msg);
extern void i2c_do_clean_msgs(struct i2c_adapter *adap);
extern void i2c_cleanup_master_msgs(FILE *stream, struct i2c_adapter *adap);
extern void i2c_cleanup_slave_msgs(FILE *stream, struct i2c_adapter *adap);
extern int i2c_set_action(struct i2c_client *client, i2c_action_t action, bool force);
extern void i2c_cleanup_adapter_msgs(struct i2c_client *client);

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
	return adapter->flags;
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
 * \brief Set the layout of the upcomming transmission.
 * \param client Client which is hosting the transmission.
 * \param layout The transmission layout to set.
 */
static inline void i2c_set_transmission_layout(struct i2c_client *client, char *layout)
{
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	shinfo->transmission_layout = layout;
}
//@}
__DECL_END

#endif /* __I2C_CORE_H */