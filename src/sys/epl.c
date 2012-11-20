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
	return BermudaEventWait((volatile THREAD**)&list->mutex, EVENT_WAIT_INFINITE);
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
 * @}
 * @}
 * @}
 */