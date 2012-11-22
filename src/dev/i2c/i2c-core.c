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
 */

#include <stdlib.h>
#include <stdio.h>

#include <dev/i2c/i2c.h>
#include <dev/i2c/reg.h>

#include <lib/list/list.h>
#include <lib/list/rcu.h>

/*
 * Static functions.
 */
static inline i2c_action_t i2c_eval_action(struct i2c_client *client);
static int i2c_edit_queue(struct i2c_client *client, const void *data, size_t size, uint8_t flags);
static inline bool i2c_has_action(i2c_features_t features);

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

PUBLIC int i2c_set_action(struct i2c_client *client, i2c_action_t action, bool force)
{
	i2c_features_t features = i2c_client_features(client);
	
	if((features & I2C_ACTION_PENDING) == 0 || force) {
		action <<= I2C_QUEUE_ACTION_SHIFT;
		features |= action;
		return 0;
	} else {
		return -1;
	}
}

static inline bool i2c_has_action(i2c_features_t features)
{
	return ((features & I2C_ACTION_PENDING) != 0);
}

/**
 * \brief Edit the queues of the given I2C client.
 * \param client Client to edit the queues of.
 * \param data Data to append to the queue.
 * \param size Length of <i>data</i>.
 * \param flags <i>flags</i> gives information about the data passed to <i><b>i2c_edit_queue</b></i>.
 * \note The given I2C client must have allocated its bus adapter.
 * \see list_last_entry I2C_MSG_CALL_BACK_FLAG I2C_MSG_SENT_STOP_FLAG I2C_MSG_SENT_REP_START_FLAG
 * \see I2C_MSG_MASTER_MSG_FLAG I2C_MSG_TRANSMIT_MSG_FLAG i2c_set_action
 *
 * Append the given <i>data</i> to the client queue. When a flush signal is given
 * the queue will be moved to the appropriate I2C adapter. If the client has not
 * allocated (i.e. locked) its bus adapter, current I2C transfer may get corrupted.
 *
 * The complexity of this function when appending data is \f$ O(n) \f$, since the new
 * data is appended at the end of the queue. See list_last_entry for more information
 * about the editting of queues.
 */
static int i2c_edit_queue(struct i2c_client *client, const void *data, size_t size, uint8_t flags)
{
	struct i2c_shared_info *sh_info;
	struct epl_list *clist, *alist;
	struct epl_list_node *node, *n_node;
	struct i2c_message *msg;
	struct i2c_adapter *adpt;
	i2c_features_t features;
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
	
	if((features & I2C_CLIENT_HAS_LOCK_FLAG) == 0 && i2c_has_action(features)) {
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
							features |= flags & (I2C_MSG_FEATURES_MASK ^ (I2C_MSG_SENT_STOP_FLAG | 
										I2C_MSG_SENT_REP_START_FLAG)); /* Do not OR stop 
																		and rep start */
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
					for_each_epl_node_safe(clist, node, n_node) {
						rc = epl_add_node(alist, node, EPL_APPEND);
						if(rc) {
							break;
						}
						
						epl_delete_node(clist, node);
						
						if(!node) {
							break;
						}
					}
					epl_unlock(adpt->msgs);
				}
				
				epl_unlock(sh_info->list);
			}
			break;
			
		case I2C_DELETE_QUEUE_ENTRY:
			if(epl_lock(adpt->msgs) == 0) {
				epl_deref(adpt->msgs, &alist);
				
				node = epl_node_at(alist, size);
				epl_delete_node(alist, node);
				free((void*)node->data);
				free(node);
				epl_unlock(adpt->msgs);
				rc = 0;
			}
			break;
			
		default:
			break;
	}
	
	features = i2c_client_features(client); /* features could be compromised */
	features &= ~I2C_ACTION_PENDING;
	
	return rc;
}

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
 * @}
 */