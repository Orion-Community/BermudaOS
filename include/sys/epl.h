/*
 *  BermudaOS - EPL header
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
 * \file include/sys/epl.h
 * \brief EPL header file.
 * \addtogroup eplAPI
 * @{
 */

#ifndef __EPL_H
#define __EPL_H

#include <stdlib.h>

#include <sys/events/event.h>

#define DEF_EPL(__mutex) struct epl_list = { SIGNALED, 0, &(__mutex) };

#define for_each_epl_node(__list, __car) for((__car) = (__list)->nodes; (__car) != NULL && \
										 (__car)->next != (__car); (__car) = (__car)->next)

/**
 * \brief List node data structure for the EPL list.
 */
struct epl_list_node
{
	struct epl_list_node *next; //!< Next pointer.
	volatile void *data; //!< Pointer to the data of this list node.
};

/**
 * \brief Definition of the EPL data structure.
 */
struct epl_list
{
	struct epl_list_node *nodes; //!< Pointer to the head node.
	size_t list_entries; //!< Amount of entries of this list.
	
	volatile void *mutex; //!< The mutex which is protecting this list.
};

/**
 * \brief Defines what should be done when epl_add_node is called.
 */
enum epl_list_action
{
	EPL_APPEND, //!< Append the new node to the end.
	EPL_IN_FRONT, //!< Add the new node in front of the head.
};

#ifdef __DOXYGEN__
#else
__DECL
#endif /* __DOXYGEN__ */

/**
 * \brief Return the amount of entries in the list.
 * \param list The list to determine the amount of entries of.
 * \return The amount of entries in the given <i>list</i>.
 */
static inline size_t epl_entries(struct epl_list *list)
{
	return list->list_entries;
}

extern void epl_deref(struct epl_list *list, struct epl_list **ref);
extern struct epl_list *epl_alloc();
extern int epl_unlock(struct epl_list *list);
extern int epl_lock(struct epl_list *list);
extern int epl_test_lock(struct epl_list *list);
extern int epl_add_node(struct epl_list *list, struct epl_list_node *node, enum epl_list_action a);
extern int epl_delete_node(struct epl_list *list, struct epl_list_node *node);
extern struct epl_list_node *epl_node_at(struct epl_list *list, size_t index);

#ifdef __DOXYGEN__
#else
__DECL_END
#endif /* __DOXYGEN__ */
#endif /* __EPL_H */

//@}