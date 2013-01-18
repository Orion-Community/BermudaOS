/*
 *  BermudaOS - I2C core driver
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
 * \file src/dev/i2c/i2c-core.c I2C core functions.
 * \addtogroup i2c
 * @{
 * \addtogroup i2c-core I2C Core
 * \brief I2C core module.
 * 
 * The I2C core module is a device/peripheral agnostic layer. It is responsible for editting
 * all queue data, creating and deleting new messages, handling of call backs, initializing data
 * transfers at the device/peripheral driver, etc..
 * 
 * \section busf Bus interface
 * The core module provides an interface for device drivers through some function calls in the
 * i2c_adapter structure (i2c_adapter::xfer i2c_adapter::resume and i2c_adapter::update). These
 * functions control the core layer using their return values and the index set. The core layer
 * will stop processing when either the return value is non zero or when the index has reached its
 * maximum length. As long as both conditions are met, i2c_adapter::resume will be called.
 * 
 * Call-back messages should NEVER be masked with I2C_MSG_DONE_FLAG too soon, since they may be
 * deleted by some other thread. The core layer will mask these messages.
 * @{
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/dev.h>
#include <dev/error.h>
#include <dev/i2c.h>
#include <dev/i2c-reg.h>
#include <dev/i2c-core.h>
#include <dev/i2c-msg.h>

#include <lib/binary.h>
#include <lib/vector.h>
#include <lib/linkedlist.h>

#include <sys/thread.h>
#include <sys/epl.h>

#include "i2c-core-priv.h"

#ifndef __THREADS__
#error I2C needs threads to function properly
#endif

/*
 * Static functions.
 */
static void __i2c_init_client(struct i2c_client *client, uint16_t sla, uint32_t hz);
static inline int i2c_cleanup_adapter_msgs(struct i2c_client *client, bool master);
static int i2c_add_entry(struct i2c_client *client, struct i2c_message *msg);

/* transmission funcs */
static int i2c_start_xfer(struct i2c_client *client);
static inline int __i2c_start_xfer(struct i2c_client *client);
static void i2c_update(struct i2c_client *client, bool master);
static inline void i2c_master_tmo(struct i2c_client *client);
static inline void i2c_slave_tmo(struct i2c_client *client);

/* concurrency functions */
static int i2c_lock_adapter(struct i2c_adapter *adapter, struct i2c_shared_info *info);
static int i2c_release_adapter(struct i2c_adapter *adapter, struct i2c_shared_info *info);

/**
 * \brief Initializes the given adapter.
 * \param adapter Adapter to initialize.
 * \param fname Name of the device.
 * \note Called by the bus driver.
 */
PUBLIC int i2c_init_adapter(struct i2c_adapter *adapter, char *fname)
{
	int rc = -1;
	adapter->dev = BermudaHeapAlloc(sizeof(*(adapter->dev)));
	
	if(adapter->dev == NULL) {
		return rc;
	} else {
		rc = 0;
	}
	
	adapter->dev->name = fname;
	BermudaDeviceRegister(adapter->dev, adapter);
	
	adapter->error = 0;
	adapter->features = 0;
	adapter->busy = false;
	return rc;
}

/**
 * \brief Lock the adapter.
 * \param adapter Adapter to lock.
 * \param info Shared info from the client which is commanding the flush.
 * \return Error code.
 * \retval 0 on success.
 * \retval -1 on error.
 * \see i2c_release_adapter
 */
static int i2c_lock_adapter(struct i2c_adapter *adapter, struct i2c_shared_info *info)
{
	FILE *stream = info->socket;
	
	if((stream->flags & I2C_MASTER) != 0) {
		return adapter->dev->alloc(adapter->dev, 0);
	} else {
		return 0;
	}
}

/**
 * \brief Unlock the adapter.
 * \param adapter Adapter to unlock.
 * \param info Shared info from the client.
 * \return Error code.
 * \retval 0 on success.
 * \retval -1 on error.
 * \see i2c_lock_adapter
 */
static int i2c_release_adapter(struct i2c_adapter *adapter, struct i2c_shared_info *info)
{
	struct device *dev = adapter->dev;
	FILE *stream = info->socket;
	
	if((stream->flags & I2C_MASTER) != 0) {
		return dev->release(dev);
	} else {
		return 0;
	}
}

/**
 * \brief Write a new buffer to the client.
 * \param client I2C client to write to.
 * \param data Buffer to write.
 * \param size Size of \p buffer.
 * \param flags Flags describing \p buffer.
 * \see i2c_queue_processor i2c_set_action
 */
PUBLIC int i2c_write_client(struct i2c_client *client, const void *data, size_t size, 
							i2c_features_t flags)
{
	struct i2c_message *msg = malloc(sizeof(*msg));
	
	if(msg) {
		msg->buff = (void*)data;
		msg->length = size;
		msg->features = flags;
		msg->addr = client->sla;
		return i2c_add_entry(client, msg);
	}
	return -1;
}

/**
 * \brief Flush the I2C client and start the transfer.
 * \param client Client to flush.
 * \see i2c_start_xfer __i2c_start_xfer
 * \note This function will initiate the transfer (and acquire the needed locks).
 * 
 * All messages which \p client has are flushed to its adapter and a transfer is initialized.
 */
PUBLIC int i2c_flush_client(struct i2c_client *client)
{
	return i2c_start_xfer(client);
}

/**
 * \brief Remove all messages from the adapter.
 * \param client I2C client which adapters messages have to be deleted.
 * \param master If set to true, not only slave messages will be deleted, but master messages too.
 * \warning The adapter must be locked before it is safe to use this function.
 */
static inline int i2c_cleanup_adapter_msgs(struct i2c_client *client, bool master)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_message *msg;
	
	size_t i = i2c_vector_length(adapter) -1;
	for(; i >= 0; i--) {
		msg = i2c_vector_get(adapter, i);
		if((i2c_msg_is_master(msg) && master) || (!i2c_msg_is_master(msg) && !master)) {
			if((i2c_msg_features(msg) & I2C_MSG_DONE_MASK) != 0) {
				i2c_vector_delete_at(adapter, i);
				free(msg);
			}
		}
		if(i == 0) {
			break;
		}
	}
	i2c_vector_reshape(adapter);
	return 0;
}

/**
 * \brief Slave message time-out.
 * \param client Slave client.
 * 
 * The slave received a time-out. This function deletes all slave buffers.
 */
static inline void i2c_slave_tmo(struct i2c_client *client)
{
	size_t index = 0;
	struct i2c_message *msg;
	
	if(i2c_first_slave_msg(client->adapter, &index)) {
		for(; index < i2c_vector_length(client->adapter); index++) {
			msg = i2c_vector_get(client->adapter, index);
			msg->features |= I2C_MSG_DONE_FLAG;
		}
	}
	i2c_cleanup_adapter_msgs(client, FALSE);
}

/**
 * \brief Master message time-out.
 * \param client Master client.
 * 
 * A new master transmission has been initiated. This function is responsible for deleting
 * all old master messages.
 */
static inline void i2c_master_tmo(struct i2c_client *client)
{
	size_t index = 0;
	struct i2c_message *msg;
	size_t diff = i2c_vector_length(client->adapter);
	int32_t s_diff;
	
	if(i2c_first_master_msg(client->adapter, &index)) {
		for(; index < i2c_vector_length(client->adapter); index++) {
			msg = i2c_vector_get(client->adapter, index);
			if((i2c_msg_features(msg) & I2C_MSG_SLAVE_MSG_MASK) != 0) {
				break;
			}
			msg->features |= I2C_MSG_DONE_FLAG;
		}
	}
	i2c_cleanup_adapter_msgs(client, TRUE);
	diff -= i2c_vector_length(client->adapter);
	s_diff = (int32_t)diff;
	s_diff *= -1;
	client->adapter->update(client->adapter, s_diff);
}

/**
 * \brief Initialize an I2C transmission.
 * \param client Client whom requests the transmission.
 * \note This function checks whether the adapter sane, __i2c_start_xfer starts the actual transfer.
 * \see __i2c_start_xfer i2c_slave_tmo
 */
static int i2c_start_xfer(struct i2c_client *client)
{
	if(client) {
		return __i2c_start_xfer(client);
	}
	return -1;
}

#if 0
static void i2c_print_vector(struct i2c_adapter *adapter)
{
	if(adapter->msg_vector.length < 7) {
		return;
	}
	struct i2c_message *msg;
	printf("--\n");
	i2c_vector_foreach(&adapter->msg_vector, i) {
		msg = i2c_vector_get(adapter, i);
		printf("M: %X Tx: %X\n", neg(msg->features) & I2C_MSG_MASTER_MSG_MASK,
			   msg->features & I2C_MSG_TRANSMIT_MSG_MASK);
	}
}
#endif

#if defined(I2C_MSG_LIST) || defined(__DOXYGEN__)
/**
 * \brief Add a new message to the client.
 * \param client i2c_client structure to add the i2c_message to.
 * \param msg i2c_message to add.
 */
static int i2c_add_entry(struct i2c_client *client, struct i2c_message *msg)
{
	struct i2c_shared_info *sh_info = i2c_shinfo(client);
	struct linkedlist *node;
	i2c_features_t features = 0;
	int rc = -1;
	

	node = malloc(sizeof(*node));
	if(node) {
		if(i2c_msg_features(msg)) {
			features = (i2c_msg_features(msg) & I2C_MSG_SENT_STOP_FLAG) ? 
						I2C_MSG_SENT_STOP_FLAG : I2C_MSG_SENT_REP_START_FLAG;
			/* Mask out invalid bits */
			features |= i2c_msg_features(msg) & (I2C_MSG_FEATURES_MASK ^ 
						(I2C_MSG_SENT_STOP_FLAG | I2C_MSG_SENT_REP_START_FLAG));
		} else {
			features = 0;
		}
		i2c_msg_set_features(msg, features);
		
		node->data = msg;
		node->next = NULL;
		rc = linkedlist_add_node(&sh_info->msgs, node, LINKEDLIST_TAIL);
	}
	
	return rc;
}

/**
 * \brief Initialize the transfer of a chain of messages.
 * \param client Client which has initialized the transfer.
 * \return An error code.
 * \retval DEV_OK Transfer was succesfull.
 * \retval -DEV_INTERNAL At least one transmission failed.
 * 
 * A transfer will be initiated using the adapter the client is configured to use. Please note, that
 * is not guarranteed that all messages which are added to the client are sent. They have to meet
 * some requirements set by the adapter.
 * 
 * \section i2c_msg_check i2c_check_msg(msg_features, bus_features)
 * 
 * This function contains a nested function, \p i2c_check_msg, which checks the validity of the 
 * messages. This will be done using the macro I2C_MSG_CHECK. The mathematical formula of this 
 * macro is 
 * \f$ f(x) = (((\neg y_{m} \land m_{f}) \gg m_{s}) \land (y_{b} \land s_{m})) \oplus ((y_{m} \gg 
 * m_{s}) \land ((y_{b} \land s_{s}) \gg m_{s})) \f$. \n
 * Where:
 * - \f$ y_{m} \f$ defines the message; 
 * - \f$ m_{f} \f$ defines the message mask;\n
 * - \f$ m_{s} \f$ defines the message shift;\n 
 * - \f$ y_{b} \f$ defines the bus;\n
 * - \f$ s_{x} \f$ defines the bus mask (if x = m it is the master mask and if x = s it is 
 *   the slave mask);\n
 * 
 * When the output of is one the message will be added to the vector on the adapter, if not (i.e.
 * the output is zero), the message will be disposed.
 * 
 * \see I2C_MSG_CHECK
 */
static inline int __i2c_start_xfer(struct i2c_client *client)
{
	auto bool i2c_check_msg(register i2c_features_t msg, register i2c_features_t bus);
	struct i2c_adapter *adapter;
	struct i2c_message *msg, *newmsg;
	struct linkedlist *node, *n_node;
	struct i2c_shared_info *sh_info;
	i2c_features_t msg_features, bus_features;
	FILE *stream;
	size_t index, length;
	int rc = -DEV_INTERNAL;
	bool master;
	
	sh_info = i2c_shinfo(client);
	adapter = client->adapter;
	stream = i2c_shinfo(client)->socket;
	master = (stream->flags & I2C_MASTER) ? TRUE : FALSE;
		
	if(i2c_lock_adapter(adapter, sh_info) == 0) {
		
		if(master) {
			i2c_master_tmo(client);
		}
		bus_features = i2c_adapter_features(adapter) & (I2C_MASTER_SUPPORT | 
														I2C_SLAVE_SUPPORT);
		index = i2c_vector_length(adapter);
		foreach_safe(sh_info->msgs, node, n_node) {
			msg = (struct i2c_message*)node->data;
			msg_features = i2c_msg_features(msg);
			if(i2c_check_msg(msg_features, bus_features)) {
				linkedlist_delete_node(&sh_info->msgs, node);
				if((msg_features & I2C_MSG_CALL_BACK_FLAG) != 0 && 
					sh_info->shared_callback == NULL) {
					msg_features = (msg_features & ~I2C_MSG_CALL_BACK_FLAG);
				}
				if((msg_features & I2C_MSG_TRANSMIT_MSG_FLAG) == 0) {
					msg->addr |= I2C_READ_BIT;
				}
				i2c_msg_set_features(msg, msg_features);
				rc = i2c_vector_add(adapter, msg, master);
				free(node);
				if(rc) {
					if(i2c_vector_error(adapter, rc) == 0) {
						rc = i2c_vector_add(adapter, msg, master);
						continue;
					}
					i2c_set_error(client);
					free(msg);
					rc = -DEV_INTERNAL;
					break;
				}
			} else {
				logmsg_P(I2C_CORE_LOG, PSTR("Msg (0x%p) not compliant with adapter (0x%p).\n"), 
							msg, adapter);
				i2c_set_error(client);
				linkedlist_delete_node(&sh_info->msgs, node);
				free(msg);
				free(node);
				rc = -DEV_INTERNAL;
			}
		}
		
		if(rc < 0) {
			goto err;
		}
		
		index = (index != 0) ? index-1 : index;
		rc = adapter->xfer(adapter, client->freq, master, &index);
		if(!rc) {
			do {
				bus_features = i2c_adapter_features(adapter);
				msg = i2c_vector_get(adapter, index-1);
				if(i2c_msg_features(msg) & I2C_MSG_CALL_BACK_FLAG) {
					newmsg = malloc(sizeof(*newmsg));
					if(!newmsg) {
						rc = -DEV_NULL;
						break;
					}
					rc = sh_info->shared_callback(client, msg, newmsg);
					msg_features = i2c_msg_features(newmsg);
					bus_features &= I2C_MASTER_SUPPORT | I2C_SLAVE_SUPPORT;
					if(i2c_check_msg(msg_features, bus_features)) {
						newmsg->addr = msg->addr & ~I2C_READ_BIT;
						if((msg_features & I2C_MSG_TRANSMIT_MSG_MASK) == 0) {
							newmsg->addr |= I2C_READ_BIT;
						}
						msg_features &= ~I2C_MSG_DONE_MASK;
						msg_features &= ~(I2C_MSG_MASTER_MSG_MASK | I2C_MSG_SLAVE_MSG_MASK);
						msg_features |= i2c_msg_features(msg) & 
										(I2C_MSG_MASTER_MSG_MASK | I2C_MSG_SLAVE_MSG_MASK);
						if(rc) {
							newmsg->buff = NULL;
							newmsg->length = 0;
						}
						msg->features |= I2C_MSG_DONE_FLAG;
						i2c_msg_set_features(newmsg, msg_features);
						rc = i2c_vector_insert_at(adapter, newmsg, index);
						if(rc) {
							if(i2c_vector_error(adapter, rc) == 0) {
								rc = i2c_vector_insert_at(adapter, newmsg, index);
								goto loop_continue;
							}
							i2c_set_error(client);
							free(newmsg);
							rc = -DEV_INTERNAL;
							break;
						}
					} else {
						logmsg_P(I2C_CORE_LOG, PSTR("Msg (0x%p) not compliant with adapter "
													"(0x%p).\n"), newmsg, adapter);
						i2c_set_error(client);
						free(newmsg);
						rc = -DEV_INTERNAL;
						break;
					}
				}
				
				loop_continue:
				length = i2c_vector_length(adapter);
				if(index < length && rc == 0) {
					rc = adapter->resume(adapter, &index);
				}
			} while(index < length && rc == 0);
		}
		i2c_update(client, master);
		i2c_release_adapter(adapter, sh_info);
	}

	return (rc <= -1) ? -DEV_INTERNAL : -DEV_OK;
	err:
	i2c_release_adapter(adapter, sh_info);
	return rc;
	
	auto bool __maxoptimize i2c_check_msg(register i2c_features_t msg, 
										  register i2c_features_t bus)
	{
		return (I2C_MSG_CHECK(msg, bus) != 0);
	}
}
#endif

/**
 * \brief Update the I2C adapter after a transfer.
 * \param client I2C client, whose adapter needs an update.
 * \see i2c_adapter::update __i2c_start_xfer
 * 
 * The I2C vector will be updated and all redudant messages will be deleted. The update function
 * of the bus will also be called.
 */
static void i2c_update(struct i2c_client *client, bool master)
{
	struct i2c_adapter *adapter = client->adapter;
	size_t diff = i2c_vector_length(adapter);
	int32_t s_diff;
	
	if(master) {
		i2c_cleanup_adapter_msgs(client, TRUE);
		diff -= i2c_vector_length(adapter);
		s_diff = (int32_t)diff;
		s_diff *= -1;
		adapter->update(adapter, s_diff);
	} else {
		i2c_slave_tmo(client);
	}
}

/**
 * \brief Delete all messages in the client message list.
 * \param client Client to cleanup.
 */
PUBLIC void i2c_cleanup_client_msgs(struct i2c_client *client)
{
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	struct linkedlist *node, *n_node;
	
	foreach_safe(shinfo->msgs, node, n_node) {
		linkedlist_delete_node(&shinfo->msgs, node);
		free((void*)node->data);
		free(node);
	}
}

/**
 * \brief Allocate an I2C client.
 * \param adapter The peripheral adapter.
 * \param sla The slave address of this adapter.
 * \param hz The frequency this adapter operates on.
 * \return The allocated client. If NULL is returned either an error occurred or the system ran out
 *         of memory.
 */
PUBLIC struct i2c_client *i2c_alloc_client(struct i2c_adapter *adapter, uint16_t sla, uint32_t hz)
{
	struct i2c_client *client = malloc(sizeof(*client));
	struct i2c_shared_info *shinfo = malloc(sizeof(*shinfo));
	
	if(client != NULL && shinfo != NULL) {
		client->adapter = adapter;
		client->sh_info = shinfo;
		__i2c_init_client(client, sla, hz);
		return client;
	} else {
		return NULL;
	}
}

/**
 * \brief Initializes a client.
 * \param client I2C client to initialize.
 * \param adapter I2C adapter backend for the client.
 * \param sla Slave address of this client.
 * \param hz Frequency it operates on.
 */
PUBLIC void i2c_init_client(struct i2c_client *client, struct i2c_adapter *adapter,
							uint16_t sla, uint32_t hz)
{
	struct i2c_shared_info *shinfo = malloc(sizeof(*shinfo));
	
	if(client && shinfo) {
		client->adapter = adapter;
		client->sh_info = shinfo;
		__i2c_init_client(client, sla, hz);
	}
}

/**
 * \brief Initializes a client.
 * \param client I2C client to initialize.
 * \param sla Slave address of this client.
 * \param hz Frequency it operates on.
 */
static void __i2c_init_client(struct i2c_client *client, uint16_t sla, uint32_t hz)
{
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	
	client->sla = sla;
	client->freq = hz;
	
	shinfo->msgs = NULL;
	shinfo->features = 0;
	shinfo->mutex = SIGNALED;
}

/*
 * Debugging functions
 */
#ifdef I2C_DBG
/**
 * \brief Test data to transfer.
 * 
 * This data is used in I2C tests. It contains 2 'random' bytes.
 */
static uint8_t test_data0[] = {0xAA, 0xCF};
/**
 * \brief Test data to transfer.
 * 
 * One random byte of test data.
 */
static uint8_t test_data1 = 0xAB;

/**
 * \brief Length of test_data0.
 */
#define TEST_DATA0_LEN 2

/**
 * \brief Length of test_data1.
 */
#define TEST_DATA1_LEN 1

/**
 * \brief Message features for <i>test_data0</i>.
 */
#define TEST_DATA0_FLAGS I2C_MSG_MASTER_MSG_FLAG | I2C_MSG_TRANSMIT_MSG_FLAG | I2C_MSG_SENT_STOP_FLAG

/**
 * \brief Message features for <i>test_data1</i>.
 */
#define TEST_DATA1_FLAGS I2C_MSG_SLAVE_MSG_FLAG

/**
 * \brief Defines wether the list is made circular yet or not.
 * \note Reset to \p FALSE every complete run.
 */
static bool done = FALSE;
void *tmp = NULL;

/**
 * \brief Test the functionality of i2c_queue_processor.
 * \param client The (test) client to use for the tests.
 * \note Altough this is a debugging function, \p client should be fully initialized.
 */
PUBLIC int i2cdbg_test_queue_processor(struct i2c_client *client)
{
	struct i2c_message *msg;
	
	/*
	 * add two entries
	 */
	i2c_set_action(client, I2C_NEW_QUEUE_ENTRY, TRUE);
	i2c_queue_processor(client, &test_data0[0], TEST_DATA0_LEN, TEST_DATA0_FLAGS);
	i2c_set_action(client, I2C_NEW_QUEUE_ENTRY, TRUE);
	i2c_queue_processor(client, &test_data1, TEST_DATA1_LEN, TEST_DATA1_FLAGS);
	
	if(i2c_vector_length(client->adapter) == 10 && !done) {
		tmp = (void*)client->adapter->msg_vector.msgs;
		client->adapter->msg_vector.msgs = NULL;
		done = TRUE;
	}
	
	/*
	 * Flush the list
	 */
	if(i2c_shinfo(client)->list->list_entries >= 10) {
		i2c_set_action(client, I2C_FLUSH_QUEUE_ENTRIES, TRUE);
		i2c_queue_processor(client, SIGNALED, 0, 0);
	}
	
	/*
	 * insert an entry
	 */
	if(i2c_vector_length(client->adapter) == 20) {
		msg = malloc(sizeof(*msg));
		if(msg) {
			msg->length = TEST_DATA0_LEN;
			msg->buff = &test_data0[0];
			i2c_msg_set_features(msg, TEST_DATA0_FLAGS);
			i2c_set_action(client, I2C_INSERT_QUEUE_ENTRY, TRUE);
			i2c_queue_processor(client, msg, 15, 0);
		}
	}
	
	/*
	 * delete all entries
	 */
	if(i2c_vector_length(client->adapter) >= 20) {
		i2c_cleanup_adapter_msgs(client);
		free((void*)client->adapter->msg_vector.msgs);
		
		client->adapter->msg_vector.length = 10;
		client->adapter->msg_vector.msgs = tmp;
		i2c_cleanup_adapter_msgs(client);
		done = FALSE;
	}
	return 0;
}
#endif

/*
 * Doxygen dummy functions.
 */
#ifdef __DOXYGEN__
/**
 * \brief Check weather a message is valid or not.
 * \param msg I2C message features.
 * \param bus I2C bus features.
 * \retval TRUE if the message is valid.
 * \retval FALSE in any other case.
 * \note This is a doxygen dummy.
 */
static bool __maxoptimize i2c_check_msg(register i2c_features_t msg, register i2c_features_t bus)
{
	return -1;
}
#endif

/**
 * @}
 * @}
 */