/*
 *  BermudaOS - EPL :: Event Protected List
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
 * \file src/sys/epl.c
 * \brief EPL :: Event Protected List
 * \addtogroup tmAPI
 * @{
 * \addtogroup synchAPI Synchronization principle's API
 * @{
 * \addtogroup eplAPI Event Protected List API
 * @{
 * 
 * EPL is a synchronization API to protect lists from multiple updates at the same time, by different
 * threads. This principle does not give any form of protection from ISR's. So, when an interrupt
 * could occur during when the list is editted <b>and</b> that list is important to the ISR - interrupts
 * have to be disabled.
 */

#include <stdlib.h>

#include <sys/thread.h>
#include <sys/sched.h>
#include <sys/epl.h>
#include <sys/events/event.h>

#define EPL_LOCK_WAIT 500

/**
 * \brief Test weather the list is locked or not.
 * \param list List which should be tested.
 * \return If not locked 0, 1 otherwise.
 */
PUBLIC int epl_test_lock(struct epl_list *list)
{
	if(list->mutex == SIGNALED) {
		return 0;
	} else {
		return 1;
	}
}

/**
 * \brief Lock an EPL.
 * \param list EPL to lock.
 * \return Success of the lock. Zero means the locking was successful, -1 means that an error has
 *         occurred.
 */
PUBLIC int epl_lock(struct epl_list *list)
{
	return BermudaEventWait((volatile THREAD**)&list->mutex, EPL_LOCK_WAIT);
}

/**
 * \brief Unlock an EPL.
 * \param list EPL to unlock.
 * \return 0 on success, -1 on an error.
 */
PUBLIC int epl_unlock(struct epl_list *list)
{
	return BermudaEventSignal((volatile THREAD**)&list->mutex);
}

/**
 * \brief Use the heap to allocate a new EPL.
 * \return The address to the newly created EPL. An error has occurred if <i>NULL</i> is returned.
 */
PUBLIC struct epl_list *epl_alloc()
{
	struct epl_list *list;
	
	list = malloc(sizeof(*list));
	list->mutex = SIGNALED;
	list->nodes = NULL;
	list->list_entries = 0;
	
	return list;
}

/**
 * \brief Set a reference to an EPL.
 * \param list List reference.
 * \param ref Reference to set.
 * 
 * When this function returns <b><i>ref</i></b> will point to the the EPL pointer <i><b>list</b></i>.
 */
PUBLIC void epl_deref(struct epl_list *list, struct epl_list **ref)
{
	enter_crit();
	*ref = list;
	exit_crit();
}

/**
 * \brief Add a new node to the list.
 * \param list The EPL pointer which holds the head of nodes.
 * \param node Node which should be added.
 * \param a Defines where the node should be added.
 * \return 0 on success, -1 otherwise.
 */
PUBLIC int epl_add_node(struct epl_list *list, struct epl_list_node *node, enum epl_list_action a)
{
	struct epl_list_node *head = list->nodes, *car;
	int rc = 0;
	
	list->list_entries++;
	/*
	 * If the head is not set, set it.
	 */
	if(head == NULL) {
		list->nodes = node;
		node->next = NULL;
		goto out;
	}
	
	switch(a) {
		case EPL_APPEND:
			for_each_epl_node(list, car) {
				if(car->next == NULL) {
					car->next = node;
					node->next = NULL;
					break;
				}
			}
			break;
			
		case EPL_IN_FRONT:
			node->next = head;
			list->nodes = node;
			break;
			
		default:
			rc = -1;
			break;
	}
	
	out:
	return rc;
}

PUBLIC int epl_delete_node(struct epl_list *list, struct epl_list_node *node)
{
	struct epl_list_node *head = list->nodes, *carriage, *prev;
	int rc = -1;
	
	if(head) {
		if(head == node) {
			head = node->next;
			list->list_entries--;
			list->nodes = head;
			rc = 0;
		} else {
			prev = head;
			for_each_epl_node(list, carriage) {
				if(carriage == node) {
					prev->next = node->next;
					node->next = NULL;
					list->list_entries--;
					rc = 0;
					break;
				}
				prev = carriage;
			}
		}
	} 
	
	return rc;
}

PUBLIC int epl_delete_node_at(struct epl_list *list, size_t num)
{
	struct epl_list_node *node = epl_node_at(list, num);
	if(node) {
		return epl_delete_node(list, node);
	}
	return -1;
}

PUBLIC struct epl_list_node *epl_node_at(struct epl_list *list, size_t index)
{
	struct epl_list_node *carriage = list->nodes;
	int i;
	
	if(index == 0) {
		return carriage;
	}
	
	for(i = 0; i < index; i++) {
		if(carriage->next == NULL) {
			return NULL;
		}
		carriage = carriage->next;
	}
	
	return carriage;
}

/**
 * @}
 * @}
 * @}
 */