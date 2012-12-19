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
 * \todo Rewrite the I2C core module.
 * 
 * The I2C core module is a device/peripheral agnostic layer. It is responsible for editting
 * all queue data, creating and deleting new messages, handling of call backs, initializing data
 * transfers at the device/peripheral driver, etc..
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
#include <lib/list/list.h>

#include <sys/thread.h>
#include <sys/epl.h>

#include "i2c-core-priv.h"

#ifndef __THREADS__
#error I2C needs threads to function properly
#endif

/*
 * Static functions.
 */
static inline i2c_action_t i2c_eval_action(struct i2c_client *client);
static int i2c_queue_processor(struct i2c_client *client, const void *data, size_t size, 
							   i2c_features_t flags);
static inline bool i2c_client_has_action(i2c_features_t features);
static int i2c_init_transfer(struct i2c_client *client);
static void __i2c_init_client(struct i2c_client *client, uint16_t sla, uint32_t hz);
static inline int i2c_cleanup_adapter_msgs(struct i2c_client *client);

/* concurrency functions */
static int i2c_lock_adapter(struct i2c_adapter *adapter, struct i2c_shared_info *info);
static int i2c_release_adapter(struct i2c_adapter *adapter, struct i2c_shared_info *info);


/**
 * \brief Prepare the driver for an I2C transfer.
 * \param stream Device file.
 * \param msg Message to add to the buffer.
 * \return 0 will be returned on success, -1 otherwise.
 */
PUBLIC int i2c_setup_msg(FILE *stream, struct i2c_message *msg,
									 uint8_t flags)
{
	return fwrite(stream, msg, flags);
}

/**
 * \brief Clean up master buffers.
 * \param stream I/O file.
 */
PUBLIC void i2c_cleanup_master_msgs(FILE *stream, struct i2c_adapter *adapter)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	
	adapter->cleanup_list[I2C_MASTER_TRANSMIT_MSG] = (void*)msgs[I2C_MASTER_TRANSMIT_MSG];
	adapter->cleanup_list[I2C_MASTER_RECEIVE_MSG]  = (void*)msgs[I2C_MASTER_RECEIVE_MSG];
	msgs[I2C_MASTER_TRANSMIT_MSG] = NULL;
	msgs[I2C_MASTER_RECEIVE_MSG]  = NULL;
}

/**
 * \brief Clean up slave buffers.
 * \param stream I/O file.
 */
PUBLIC void i2c_cleanup_slave_msgs(FILE *stream, struct i2c_adapter *adapter)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	
	adapter->cleanup_list[I2C_SLAVE_RECEIVE_MSG] = (void*)msgs[I2C_SLAVE_RECEIVE_MSG];
	adapter->cleanup_list[I2C_SLAVE_TRANSMIT_MSG] = (void*)msgs[I2C_SLAVE_TRANSMIT_MSG];
	msgs[I2C_SLAVE_RECEIVE_MSG] = NULL;
	msgs[I2C_SLAVE_TRANSMIT_MSG]  = NULL;
}

/**
 * \brief Clean up all marked I2C messages.
 * 
 * All allocated memory will be free'd.
 */
PUBLIC void i2c_do_clean_msgs(struct i2c_adapter *adapter)
{
	uint8_t i = 0;
	
	for(; i < I2C_MSG_NUM; i++) {
		if(adapter->cleanup_list[i] != NULL) {
			BermudaHeapFree(adapter->cleanup_list[i]);
			adapter->cleanup_list[i] = NULL;
		}
	}
}

/**
 * \brief Mark a message to be cleaned up.
 * \param stream I/O file. The message array should be in the buffer of this I/O file.
 * \param msg Index of the message to clean up.
 */
PUBLIC void i2c_cleanup_msg(FILE *stream, struct i2c_adapter *adapter, uint8_t msg)
{
	volatile struct i2c_message **msgs = (volatile struct i2c_message**)stream->buff;
	adapter->cleanup_list[msg] = (void*)msgs[msg];
	msgs[msg] = NULL;
}

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
	
	adapter->flags = 0;
	adapter->busy = false;
	return rc;
}

/**
 * \brief Sets a new action for the given I2C client.
 * \param client Client to set the action for.
 * \param action Action to set.
 * \param force If set to <b><i>TRUE</i></b>, this function will override any pending action.
 * \return 0 on success, -1 if there is an action pending and <i>force</i> is set to 
 *         <b><i>FALSE</i></b>.
 */
PUBLIC int i2c_set_action(struct i2c_client *client, i2c_action_t action, bool force)
{
	i2c_features_t features = i2c_client_features(client);
	
	if((features & I2C_ACTION_PENDING) == 0 || force) {
		action <<= I2C_QUEUE_ACTION_SHIFT;
		features &= ~I2C_QUEUE_ACTION_MASK;
		features |= (action | I2C_ACTION_PENDING);
		i2c_shinfo(client)->features = features;
		return 0;
	} else {
		return -1;
	}
}

/**
 * \brief Checks if the client has an action pending.
 * \param features Features of the given client.
 * \see I2C_ACTION_PENDING i2c_set_action
 * \return Wether there is an action set or not.
 * \retval 1 An action is pending.
 * \retval 0 No action is pending.
 */
static inline bool i2c_client_has_action(i2c_features_t features)
{
	return ((features & I2C_ACTION_PENDING) != 0);
}

/**
 * \brief Check the current action.
 * \param client Client which should be checked for actions.
 * \warning This function does not check for pending actions.
 * \return The current configured action.
 */
static inline i2c_action_t i2c_eval_action(struct i2c_client *client)
{
	i2c_features_t features = i2c_client_features(client);
	
	return (i2c_action_t)((features & I2C_QUEUE_ACTION_MASK) >> I2C_QUEUE_ACTION_SHIFT);
}

/**
 * \brief Arrange the call back to the client.
 * \param client Client to call.
 * \param stream Bus I/O file.
 */
PUBLIC int i2c_call_client(struct i2c_client *client, FILE *stream)
{
	struct i2c_message msg;
	
	client->callback(&msg);
	fwrite(stream, &msg, I2C_SLAVE_TRANSMIT_MSG);
	
	return client->adapter->slave_respond(stream);
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
		return adapter->dev->alloc(adapter->dev, I2C_TMO);
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
	i2c_set_action(client, I2C_NEW_QUEUE_ENTRY, FALSE);
	return i2c_queue_processor(client, data, size, flags);
}

/**
 * \brief Flush the I2C client and start the transfer.
 * \param client Client to flush.
 * \see i2c_queue_processor i2c_set_action
 * \note This function will initiate the transfer (and acquire the needed locks).
 * 
 * All messages which \p client has are flushed to its adapter and a transfer is initialized.
 */
PUBLIC int i2c_flush_client(struct i2c_client *client)
{
	i2c_set_action(client, I2C_FLUSH_QUEUE_ENTRIES, FALSE);
	return i2c_queue_processor(client, SIGNALED, 0, 0);
}

/**
 * \brief Remove all messages from the adapter.
 * \param client I2C client which adapters messages have to be deleted.
 * \warning The adapter must be locked before it is safe to use this function.
 */
static inline int i2c_cleanup_adapter_msgs(struct i2c_client *client)
{
	i2c_vector_foreach_reverse(&client->adapter->msg_vector, i) {
		i2c_set_action(client, I2C_DELETE_QUEUE_ENTRY, TRUE);
		i2c_queue_processor(client, NULL, i, 0);
		if(i == 0) {
			break;
		}
	}
	return i2c_vector_erase(client->adapter);
}

#ifdef I2C_MSG_LIST
/**
 * \brief Edit the queues of the given I2C client.
 * \param client Client to edit the queues of.
 * \param data Data to append to the queue.
 * \param size Length of <i>data</i>.
 * \param flags <i>flags</i> gives information about the data passed to <i><b>i2c_queue_processor</b></i>.
 * \note The given I2C client must have allocated (i.e. locked) its bus adapter.
 * \see list_last_entry I2C_MSG_CALL_BACK_FLAG I2C_MSG_SENT_STOP_FLAG I2C_MSG_SENT_REP_START_FLAG
 * \see I2C_MSG_MASTER_MSG_MASK I2C_MSG_TRANSMIT_MSG_FLAG i2c_set_action
 * \note Messages are not guarranteed to be transmitted, they will be checked if they are compatible
 *       with the adapter.
 *
 * Append the given \p data to the client queue. 
 * 
 * \section flush I2C_FLUSH_QUEUE_ENTRIES
 * When a flush signal is given the queue will be moved to the appropriate I2C adapter. 
 * When this is done, all messages will be checked against 
 *  \f$ f(x) = (((\neg y_{m} \land m_{f}) \gg m_{s}) \land (y_{b} \land s_{m})) \oplus ((y_{m} \gg 
 * m_{s}) \land ((y_{b} \land s_{s}) \gg m_{s})) \f$ \n
 * Where \n
 * * \f$ y_{m} \f$ defines the message; \n
 * * \f$ m_{f} = 2 \f$ defines the message mask; \n
 * * \f$ m_{s} = 1 \f$ defines the message shift; \n
 * * \f$ y_{b} \f$ defines the bus; \n
 * * \f$ s_{x} \f$ defines the bus mask (if x = m it is the master mask and if x = s it is the slave
 * mask); \n
 * 
 * If this function is <i>1</i>, the adapter supports the message, if <i>0</i> the
 * adapter is unable to send the message (and the message willl therefore be discarded).
 * 
 * \section act Actions
 * The following actions can be passed to \p i2c_queue_processor:
 * \verbatim
   I2C_NEW_QUEUE_ENTRY		Create a new message and add it to the client.
   I2C_INSERT_QUEUE_ENTRY	Insert a message in front of the EPL of the client's adapter.
   I2C_FLUSH_QUEUE_ENTRIES	Flush all queue entries in the client's list to the adapter's list.
   I2C_DELETE_QUEUE_ENTRY	Delete the first entry of the adapter queue. Passed by setting data to
  							NULL.
   \endverbatim
 * 
 * \section conc Concurrency
 * It is very important that the application has locked the \p client. If the client (and thus
 * its adapter) are not locked, the function will always return an error (\p -1.).
 *
 * The complexity of this function when appending data is \f$ O(n) \f$, since the new
 * data is appended at the end of the queue. See list_last_entry for more information
 * about the editting of queues.
 */
static int __link i2c_queue_processor(client, data, size, flags)
struct i2c_client *client;
const void *data;
size_t size;
i2c_features_t flags;
{
	auto bool i2c_check_msg(register i2c_features_t msg, register i2c_features_t bus);
	struct i2c_shared_info *sh_info;
	struct epl_list *clist;
	struct epl_list_node *node, *n_node;
	struct i2c_message *msg;
	struct i2c_adapter *adpt;
	i2c_features_t features, b_features; /* client and bus features */
	i2c_action_t action;
	int rc = -1;
	
	if(data == NULL) {
		action = I2C_DELETE_QUEUE_ENTRY;
	} else {
		action = i2c_eval_action(client);
	}

	sh_info = i2c_shinfo(client);
	features = i2c_client_features(client);
	adpt = client->adapter;
	
	if((features & I2C_CLIENT_HAS_LOCK_FLAG) == 0 && !i2c_client_has_action(features)) {
		return rc;
	}
	
	switch(action) {
		case I2C_NEW_QUEUE_ENTRY:
			if(epl_lock(sh_info->list) == 0) {
				epl_deref(sh_info->list, &clist);
				node = malloc(sizeof(*node));
				if(node) {
					msg = malloc(sizeof(*msg));
					if(msg) {
						msg->buff = (void*)data;
						msg->length = size;
						msg->addr = client->sla;
						
						if(flags) {
							features = (flags & I2C_MSG_SENT_STOP_FLAG) ? I2C_MSG_SENT_STOP_FLAG :
										I2C_MSG_SENT_REP_START_FLAG;
							/* Mask out invalid bits */
							features |= flags & (I2C_MSG_FEATURES_MASK ^ (I2C_MSG_SENT_STOP_FLAG | 
										I2C_MSG_SENT_REP_START_FLAG));
						} else {
							features = 0;
						}
						i2c_msg_set_features(msg, features);
					}
					
					node->data = msg;
					node->next = NULL;
					rc = epl_add_node(clist, node, EPL_APPEND);
				}
				epl_unlock(sh_info->list);
			}
			break;
		
		/*
		 * Insert an entry at the start of the list of the adapter. This case is usually used
		 * after a call back to the application to insert a new I2C message.
		 */
		case I2C_INSERT_QUEUE_ENTRY:
				msg = (struct i2c_message*)data;
				if(msg) {
					b_features = i2c_adapter_features(adpt) & (I2C_MASTER_SUPPORT | 
																I2C_SLAVE_SUPPORT);
					features = i2c_msg_features(msg);
					if(i2c_check_msg(features, b_features)) {
						rc = i2c_vector_insert_at(adpt, msg, size);
						if(rc) {
							if(i2c_vector_error(adpt, rc) == 0) {
								rc = i2c_vector_insert_at(adpt, msg, size);
							}
						}
					} else {
						logmsg_P(I2C_CORE_LOG, PSTR("Message (0x%p) not compliant with adapter."
													"\n"), msg);
						free(msg);
					}
				}
				break;

		case I2C_FLUSH_QUEUE_ENTRIES:
			if(epl_lock(sh_info->list) == 0) {
				epl_deref(sh_info->list, &clist);
				
				if(i2c_lock_adapter(adpt, sh_info) == 0) {
					b_features = i2c_adapter_features(adpt) & (I2C_MASTER_SUPPORT | 
																I2C_SLAVE_SUPPORT);
					for_each_epl_node_safe(clist, node, n_node) {
						features = i2c_msg_features((void*)node->data);
						if((features & I2C_MSG_CALL_BACK_FLAG) != 0 && 
							sh_info->shared_callback == NULL) {
							features &= (features & ~I2C_MSG_CALL_BACK_FLAG);
						}
						msg = (struct i2c_message*) node->data;
						msg->features = features;
						features &= I2C_MSG_MASTER_MSG_MASK | I2C_MSG_SLAVE_MSG_FLAG;

						if(i2c_check_msg(features, b_features)) {
							epl_delete_node(clist, node);
							free(node);
							rc = i2c_vector_add(adpt, msg);
							if(rc) {
								if(i2c_vector_error(adpt, rc) == 0) {
									rc = i2c_vector_add(adpt, msg);
									continue;
								}
								logmsg_P(I2C_CORE_LOG, PSTR("Error occurred while trying to add a "
															"msg (0x%p) to the adapter\n"), msg);
								i2c_set_error(client);
								free(msg);
								rc = -DEV_INTERNAL;
								break;
							}
						} else {
							logmsg_P(I2C_CORE_LOG, PSTR("Message (0x%p) not compliant with adapter."
														"\n"), msg);
							i2c_set_error(client);
							epl_delete_node(clist, node);
							free(msg);
							free(node);
							rc = -DEV_INTERNAL;
						}
					}
					epl_unlock(sh_info->list);
					if(!rc) {
						rc = i2c_init_transfer(client);
					}
					i2c_release_adapter(adpt, sh_info);
				}
			}
			break;
			
		case I2C_DELETE_QUEUE_ENTRY:
			if(size >= 0) {
				if((msg = i2c_vector_delete_at(adpt, size)) != NULL) {
					free(msg);
					rc = DEV_OK;
				}
			}
			break;
			
		default:
			break;
	}
	
	features = i2c_client_features(client); /* features could be compromised */
	features &= ~(I2C_ACTION_PENDING | I2C_QUEUE_ACTION_MASK);
	i2c_shinfo(client)->features = features;
	
	return rc;
	
	auto bool __maxoptimize i2c_check_msg(register i2c_features_t msg, 
										  register i2c_features_t bus)
	{
		return (I2C_MSG_CHECK(msg, bus) != 0);
	}
}
#endif

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
 * \brief Initializes the transfer to the adapter.
 * \param adapter Adapter containing new messages.
 * \note This function also tries to get the locks needed to complete the transfer.
 */
static int i2c_init_transfer(struct i2c_client *client)
{
	struct i2c_adapter *adapter = client->adapter;
	struct i2c_message *msg, *newmsg;
	size_t index;
	int rc = -1;
	
	if((msg = adapter->start_transfer(adapter, client->freq)) != NULL) {
		index = i2c_vector_locate(adapter, msg);
		rc = 0;
		if(i2c_msg_features(msg) & I2C_MSG_CALL_BACK_FLAG) {
			newmsg = malloc(sizeof(*newmsg));
			newmsg->addr = msg->addr;
			if(!i2c_shinfo(client)->shared_callback(client, newmsg)) {
				i2c_set_action(client, I2C_INSERT_QUEUE_ENTRY, TRUE);
				i2c_queue_processor(client, newmsg, index, 0);
			}
			rc = adapter->resume(adapter);
		}
	}
	printf("Entries: %u\n", adapter->msg_vector.length);
	i2c_cleanup_adapter_msgs(client);
	return rc;
}

/**
 * \brief Delete all messages in the client message list.
 * \param client Client to cleanup.
 */
PUBLIC void i2c_cleanup_client_msgs(struct i2c_client *client)
{
	struct i2c_shared_info *shinfo = i2c_shinfo(client);
	struct epl_list_node *node, *n_node;
	struct epl_list *clist;
	
	if(epl_lock(shinfo->list) == 0) {
		epl_deref(shinfo->list, &clist);
		for_each_epl_node_safe(clist, node, n_node) {
			epl_delete_node(clist, node);
			free((void*)node->data);
			free(node);
		}
		
		epl_unlock(shinfo->list);
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
	
	if(!client && !shinfo) {
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
	
	shinfo->list = epl_alloc();
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


/**
 * @}
 * @}
 */