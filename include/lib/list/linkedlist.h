/*
 *  BermudaOS - C++ Linked List
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

#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H

#include <stdlib.h>
#include <lib/list/list.h>

/**
 * \brief Type defintion of the insert locations.
 */
typedef enum
{
	LINKEDLIST_HEAD, //!< Defines insertion at the start.
	LINKEDLIST_TAIL, //!< Defines insertion at the end.
} linkedlist_location_t;

#ifdef __cplusplus
template <class T>
class LinkedListNode
{
public:
        LinkedListNode(T *data);
        LinkedListNode();
        
        LinkedListNode<T> *GetNext();
        
private:
        T *data;
        LinkedListNode<T> *next;
};

template <class T> 
class LinkedList
{
public:
        LinkedList();
        void add(T *value);
        
private:
        LinkedListNode<T> *list;
};
#else
extern struct linkedlist *linkedlist_alloc();
extern void linkedlist_init(struct linkedlist *list, void *data);
extern int linkedlist_set_data(struct linkedlist *list, void *data);
extern int linkedlist_set_data_at(struct linkedlist *head, void *data, size_t index);
extern int linkedlist_add_node(struct linkedlist *head, void *data, int location);
extern int linkedlist_delete_node_at(struct linkedlist *head, size_t index);
extern int linkedlist_delete_node(struct linkedlist *head, struct linkedlist *node);
#endif

#endif