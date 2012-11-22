/*
 *  BermudaOS - List library
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

/** \file list.h */
#ifndef __LIST_H
#define __LIST_H

/**
 * \def foreach
 * \brief Loop trough a linked list.
 * \warning This loop construction is not protected against node removal.
 * \see foreach_safe
 * 
 * If the list construction has to be removal safe, use foreach_safe.
 */
#define foreach(list, c) \
                                for(c = (list); c; c = (c)->next)

/**
 * \brief Linked list prototype.
 */
struct list
{
	struct list *next; //!< Next entry in the list.
};

/**
 * \def foreach_safe
 * \brief Linked list loop construction.
 * 
 * Walk trough the linked list in a safe way. The extra temporary variable
 * protects against the removal of items.
 */
#define foreach_safe(list, c, tmp) \
                                for(c = (list), tmp = (list)->next; c != NULL && \
                                c != tmp; c = (tmp), tmp = (c)->next)
                                
#define ForEachpp(lpp, lp) \
                        for(; lp != NULL; lp = (lp)->next, lpp = &((lp)->next))

#ifdef __DOXYGEN__
#else
__DECL
#endif

extern struct list *list_last_entry(void *vp);

#ifdef __DOXYGEN__
#else
__DECL_END
#endif                        

#endif