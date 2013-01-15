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
#include <lib/list/linkedlist.h>

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
