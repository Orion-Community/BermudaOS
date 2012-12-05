/*
 *  BermudaOS - I2C device driver
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
#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>
#include <dev/i2c/i2c-core.h>

#include <lib/binary.h>
#include <lib/list/list.h>
#include <lib/list/rcu.h>

/*
 * Static functions.
 */
static inline i2c_action_t i2c_eval_action(struct i2c_client *client);
static int i2c_queue_processor(struct i2c_client *client, const void *data, size_t size, 
							   uint8_t flags);
static inline bool i2c_client_has_action(i2c_features_t features);
static void i2c_init_transfer(struct i2c_adapter *adapter);

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
 * \param Peripheral I/O file.
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
 * \param Peripheral I/O file.
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
		features |= action;
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
 * \todo Debug this function.
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
uint8_t flags;
{
	auto i2c_features_t i2c_check_msg(i2c_features_t msg, i2c_features_t bus);
	struct i2c_shared_info *sh_info;
	struct epl_list *clist, *alist;
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
	
	if((features & I2C_CLIENT_HAS_LOCK_FLAG) == 0 && i2c_client_has_action(features)) {
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
			if(epl_lock(sh_info->list) == 0) {
				epl_deref(sh_info->list, &clist);
				
				msg = (struct i2c_message*)data;
				if(msg->length == size) {
					node = malloc(sizeof(*node));
					if(node) {
						node->next = NULL;
						node->data = (void*)msg;
						rc = epl_add_node(clist, node, EPL_IN_FRONT);
					}
				}
				epl_unlock(sh_info->list);
			}
			break;
		
		case I2C_FLUSH_QUEUE_ENTRIES:
			if(epl_lock(sh_info->list) == 0) {

				epl_deref(sh_info->list, &clist);
				epl_deref(adpt->msgs, &alist);
				
				if(epl_lock(adpt->msgs) == 0) {
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
							rc = epl_add_node(alist, node, EPL_APPEND);
							if(rc) {
								i2c_set_error(client);
								rc = -1;
								break;
							}
							epl_delete_node(clist, node);
						} else {
							i2c_set_error(client);
							epl_delete_node(clist, node);
							free(msg);
							free(node);
						}
					}
					epl_unlock(adpt->msgs);
					
				}
				epl_unlock(sh_info->list);
				i2c_init_transfer(adpt);
			}
			break;
			
		case I2C_DELETE_QUEUE_ENTRY:
			if(epl_lock(adpt->msgs) == 0) {
				epl_deref(adpt->msgs, &alist);
				
				node = epl_node_at(alist, size);
				if(node) {
					epl_delete_node(alist, node);
					free((void*)node->data);
					free(node);
					rc = 0;
				}
				epl_unlock(adpt->msgs);
			}
			break;
			
		default:
			break;
	}
	
	features = i2c_client_features(client); /* features could be compromised */
	features &= ~(I2C_ACTION_PENDING | I2C_QUEUE_ACTION_MASK);
	i2c_shinfo(client)->features = features;
	
	return rc;
	
	auto __link __maxoptimize i2c_features_t i2c_check_msg(i2c_features_t msg, i2c_features_t bus)
	{
#define I2C_MSG_CHECK(__msg, __bus) \
( \
(((neg(__msg) & I2C_MSG_MASTER_MSG_MASK) >> I2C_MSG_MASTER_MSG_FLAG_SHIFT) & (__bus & I2C_MASTER_SUPPORT)) ^ \
((__msg >> I2C_MSG_SLAVE_MSG_FLAG_SHIFT) & ((__bus & I2C_SLAVE_SUPPORT) >> I2C_SLAVE_SUPPORT_SHIFT))  \
)
		return I2C_MSG_CHECK(msg, bus);
#undef I2C_MSG_CHECK
	}
}
#endif

static void i2c_init_transfer(struct i2c_adapter *adapter)
{
	
}

/**
 * \brief Clean all messages in the adapter queue of the given client.
 * \param client Client which adapter's messages should be deleted.
 */
PUBLIC void i2c_cleanup_adapter_msgs(struct i2c_client *client)
{
	int rc = 0;
	i2c_set_action(client, I2C_DELETE_QUEUE_ENTRY, TRUE);
	
	do {
		rc = i2c_queue_processor(client, NULL, 0, 0);
		if(rc == 0) {
			i2c_set_action(client, I2C_DELETE_QUEUE_ENTRY, FALSE);
		}
	} while(rc == 0);
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
		client->sh_info = shinfo;
		client->adapter = adapter;
		client->sla = sla;
		client->freq = hz;
		
		shinfo->list = epl_alloc();
		shinfo->features = 0;
		return client;
	} else {
		return NULL;
	}
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

#define TEST_DATA0_FLAGS I2C_MSG_MASTER_MSG_FLAG | I2C_MSG_TRANSMIT_MSG_FLAG | I2C_MSG_SENT_STOP_FLAG
#define TEST_DATA1_FLAGS I2C_MSG_SLAVE_MSG_FLAG

/**
 * \brief Test the functionality of i2c_queue_processor.
 * \param client The (test) client to use for the tests.
 * \note Altough this is a debugging function, \p client should be fully initialized.
 */
PUBLIC int i2cdbg_test_queue_processor(struct i2c_client *client)
{
	i2c_set_action(client, I2C_NEW_QUEUE_ENTRY, TRUE);
	i2c_queue_processor(client, &test_data0[0], TEST_DATA0_LEN, TEST_DATA0_FLAGS);
	i2c_set_action(client, I2C_NEW_QUEUE_ENTRY, TRUE);
	i2c_queue_processor(client, &test_data1, TEST_DATA1_LEN, TEST_DATA1_FLAGS);
	
	if(i2c_shinfo(client)->list->list_entries >= 10) {
		i2c_set_action(client, I2C_FLUSH_QUEUE_ENTRIES, TRUE);
		i2c_queue_processor(client, SIGNALED, 0, 0);
	}
	return 0;
}
#endif


/**
 * @}
 * @}
 */