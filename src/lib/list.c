/*
 *  BermudaNet - List support functions
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

#include <stdlib.h>

#include <lib/list/list.h>
#include <lib/linkedlist.h>

/**
 * \brief Initialize a linked list.
 * \param list Linked list to initialize.
 * \param data First data pointer.
 */
PUBLIC void linkedlist_init(struct linkedlist *list, void *data)
{
	if(list) {
		list->data = data;
		list->next = NULL;
	}
}

/**
 * \brief Allocate a new linked list.
 * \return The newly created linked list.
 * \retval NULL if no memory is available.
 * \see linkedlist_init
 */
PUBLIC struct linkedlist *linkedlist_alloc()
{
	struct linkedlist *list = malloc(sizeof(*list));
	
	if(list) {
		linkedlist_init(list, NULL);
	}
	
	return list;
}

/**
 * \brief Set the data of a given node.
 * \param node Node to set the data for.
 * \param data Data to set.
 */
PUBLIC int linkedlist_set_data(struct linkedlist *node, void *data)
{
	if(node) {
		node->data = data;
		return 0;
	} else {
		return -1;
	}
}

/**
 * \brief Attatch data to a node.
 * \param head Head of the linkedlist.
 * \param data Data to attach.
 * \param index Index in \p head to attach \p data to.
 * \retval 0 data is successfully attach to the node at \p index.
 * \retval -1 An error occurred while attaching \p data to the node at \p index.
 * 
 * \p data will be attached to the node located at \p index.
 */
PUBLIC int linkedlist_set_data_at(struct linkedlist *head, void *data, size_t index)
{
	size_t i;
	struct linkedlist *carriage;
	
	iforeach(head, carriage, i) {
		if(i == index) {
			carriage = data;
			return 0;
		}
	}
	
	return -1;
}

/**
 * \brief Add a node the the given linked list.
 * \param headpp Linked list to add to.
 * \param data Data to attatch to the new node.
 * \param location Location to insert the new node.
 * \see linkedlist_location_t
 * \note A new linkedlist will be allocated.
 * \retval 0 node has successfully been added.
 * \retval -1 Error occurred while trying to add the node.
 */
PUBLIC int linkedlist_create_node(struct linkedlist **headpp, void *data, int location)
{
	struct linkedlist *node = malloc(sizeof(*node));
	
	if(!node) {
		return -1;
	}
	
	node->data = data;
	return linkedlist_add_node(headpp, node, location);
}

/**
 * \brief Add a new node to the a linked list.
 * \param node Node to add.
 * \param location Location to add the node.
 * \see linkedlist_location_t
 * \note A new linkedlist will be allocated.
 * \retval 0 node has successfully been added.
 * \retval -1 Error occurred while trying to add the node.
 */
PUBLIC int linkedlist_add_node(struct linkedlist **headpp, struct linkedlist *node, int location)
{
	struct linkedlist *listp;
	int rc;
	listp = *headpp;
	
	switch(location) {
		case LINKEDLIST_HEAD:
			node->next = listp;
			rc = 0;
			break;
			
		case LINKEDLIST_TAIL:
			while(listp) {
				headpp = &listp->next;
				listp = listp->next;
			}
			
			node->next = listp;
			*headpp = node;
			rc = 0;
			break;
			
		default:
			rc = -1;
			break;
	}
	
	return rc;
}

/**
 * \brief Delete a node at a given index.
 * \param headpp List to delete from.
 * \param index Index to delete at.
 * \retval 0 deletion was successful.
 * \retval -1 deletion failed.
 */
PUBLIC int linkedlist_delete_node_at(struct linkedlist **headpp, size_t index)
{
	size_t i = 0;
	int rc = -1;
	struct linkedlist *listp;
	
	listp = *headpp;
	
	foreach(listp, listp) {
		if(index == i) {
			*headpp = listp->next;
			listp->next = NULL;
			rc = 0;
			break;
		}
	}
	
	return rc;
}

PUBLIC int linkedlist_delete_node(struct linkedlist **headpp, struct linkedlist *node)
{
	int rc = 0;
	struct linkedlist *listp;
	
	listp = *headpp;
	
	foreach(listp, listp) {
		if(listp == node) {
			*headpp = node->next;
			node->next = NULL;
			rc = -1;
			break;
		}
	}
	
	return rc;
}

/**
 * \brief Locate the last entry in a linked list.
 * \param vp Pointer to the linked list.
 * \note The list::next member <b>MUST</b> be the first member in the structure.
 */
PUBLIC struct list *list_last_entry(void *vp)
{
	struct list *list;
	
	for(list = vp; list != NULL && list != list->next; list = list->next) {
		if(list->next == NULL) {
			return list;
		}
	}
	
	return NULL;
}
